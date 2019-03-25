//
// Created by Mark Plagge on 2019-03-11.
//

#include "../include/mapping.h"
#include "../include/globals.h"
#include "INeuroCoreBase.h"
#include "TrueNorthCore.h"

void setup_neuron_weights(int neuron_weights[], int core_id, int neuron_id){
    for(int i = 0; i < WEIGHTS_PER_TN_NEURON; i ++){
        neuron_weights[i] = i;
    }
}

TrueNorthCore::TrueNorthCore(int coreLocalId, int outputMode)  {
    // load up TrueNorth neuron info.
    //for each neuron in the core:
    //load the TN parameters for that neuron
    //set neuron weights
    // set other neruon params
    this->core_local_id = coreLocalId;
    this->last_active_time = 0;
    this->last_leak_time = 0;
    this->output_mode = outputMode;


}

// Seperate function for init.
void TrueNorthCore::core_init(struct tw_lp *lp) {
    std::cout << "Initializing core # " << get_core_from_gid(lp->gid) << "\n";
    //assuming linear mapping.
    this->core_local_id = get_core_from_gid(lp->gid);
    this->has_self_firing_neuron = false;
    //check to see if this is a self-firing neuron containing core:
    for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
        this->has_self_firing_neuron = this->has_self_firing_neuron | this->is_self_firing_neuron(i);
    }

}

void TrueNorthCore::pre_run(struct tw_lp *lp) {
    // determine if we have self_firing neurons

    for (int i = 0; i < NEURONS_PER_TN_CORE; i ++){
        has_self_firing_neuron = has_self_firing_neuron | is_self_firing_neuron(i);
    }
    if (has_self_firing_neuron){
        // need to send ourselves a heartbeat out of the gate
        send_heartbeat(lp);
    }
#ifdef THREADED_WRITER

    this->spike_output =  (new CoreOutputThread(std::string (SPIKE_OUTPUT_FILENAME)));
#else
    this->spike_output = new CoreOutput(std::string(SPIKE_OUTPUT_FILENAME));
#endif
}

void TrueNorthCore::forward_event(struct tw_bf *bf, nemo_message *m, struct tw_lp *lp) {
    // Messages are core->core
    // if this is a message from a core's output, integrate.
    // if this message is a heartbeat from ourselves, then we will
    // do leaks, get firing info, and do resets.
    // we also need to set the current simulated neuromorphic "tick", and
    // check to see if the message (if it is a spike) has arrived in an invalid order.
    // EG: tick = 1, spike from neuron scheduled for 2, and heartbeat message is out!
    // this should not happen.
    evt_stat = BF_Event_Status ::None;
    unsigned int random_call_count = lp->rng->count;
    //check/update neuro tick
    if (m->intended_neuro_tick > this->current_neuro_tick){
        evt_stat |= BF_Event_Status ::NS_Tick_Update;
        this->current_neuro_tick = get_neurosynaptic_tick(tw_now(lp));
    }

    if(m->message_type == NEURON_SPIKE){
        evt_stat |= BF_Event_Status ::Spike_Rec;
        if ( ! this->heartbeat_sent){
            // send the heartbeat message:
            evt_stat |= BF_Event_Status::Heartbeat_Sent;
            this->send_heartbeat(lp, bf);
            this->heartbeat_sent = true;
        }

        // integrate
        this->integrate(lp, 0);
        if(this->last_active_time < tw_now(lp)){
            this->last_active_time = tw_now(lp);
        }else{
            tw_warning(TW_LOC, "Last active time was %d, current time is: %d. \n", this->last_active_time, tw_now(lp));
        }
    }else if(m->message_type == HEARTBEAT){
        // heartbeat message rcvd. We need to leak, fire, reset.
        //first though, set up the heartbeat state.
        evt_stat |= BF_Event_Status :: Heartbeat_Rec;
        this->heartbeat_sent = false;
        this->leak(lp, bf);
        if(this->last_leak_time < tw_now(lp)){
            this->last_leak_time = tw_now(lp);
        }else{
            tw_warning(TW_LOC, "Last leak time was $d, current time is %d. \n", this->last_leak_time, tw_now(lp));
        }
        //ringing at 0 check
        this->ringing();

        //bool did_fire = this->fire(lp, bf);
        this->fire_reset(lp);
        // Send those fire events:
        for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
            if(this->fire_status[i]){
                evt_stat |= BF_Event_Status :: Spike_Sent;
                // output neurons do not actually send spikes - we will collect them for output layers //
                if( is_output_neuron(i)){
                    /** @todo log output spikes to main output file. */
                }else {

                    for (int j = 0; j < MAX_OUTPUT_PER_TN_NEURON; j++) {

                        uint64_t dest_gid = get_gid_from_core_local(destination_cores[i][j], destination_axons[i][j]);
                        double dest_time = this->delays[i] + JITTER(lp->rng);
                        struct tw_event *evt = tw_event_new(dest_gid, dest_time, lp);
                        nemo_message *msg = (nemo_message *) tw_event_data(evt);
                        msg->message_type = NEURON_SPIKE;
                        msg->intended_neuro_tick = this->current_neuro_tick + 1;
                        msg->dest_axon = destination_axons[i][j];
                        msg->source_core = this->core_local_id;
                        msg->nemo_event_status = as_integer(evt_stat);
                        msg->random_call_count = lp->rng->count - random_call_count;
                        msg->debug_time = tw_now(lp);
                        tw_event_send(evt);
                    }
                }
            }
        }

        this->last_active_time = tw_now(lp);
        if (this->has_self_firing_neuron ){
            this->send_heartbeat(lp, bf); /**! @todo: check to see if we need to send a heartbeat when a neuron fires. */
            evt_stat |= BF_Event_Status ::Heartbeat_Sent;
        }
    }
    else{
        tw_error(TW_LOC,"Error - message not understood.\n");
    }

}

void TrueNorthCore::reverse_event(struct tw_bf *bf, nemo_message *m, struct tw_lp *lp) {

    auto m_evt_stat = m->nemo_event_status;


    if(in_the(m_evt_stat, BF_Event_Status::Heartbeat_Sent)){
        //undo spike sent logic
    }
    if(in_the(m_evt_stat, BF_Event_Status::Spike_Sent)){
        //undo heartbeat sent logic
    }
    if(in_the(m_evt_stat, BF_Event_Status::Heartbeat_Rec)){
        //undo heartbeat recvd logic.
    }
    if(in_the(m_evt_stat, BF_Event_Status::Spike_Rec)){
        // undo spike recvd logic.
    }





}

void TrueNorthCore::core_commit(struct tw_bf *bf, nemo_message *m, struct tw_lp *lp) {
    if (m->message_type == HEARTBEAT) {

#pragma omp parallel for
        for (int i = 0; i < NEURONS_PER_TN_CORE; i++) {
            if (this->fire_status[i]) {
                if ((this->output_mode == 1 && is_output_neuron(i)) || this->output_mode == 2) {
                    for (int dd = 0; dd < MAX_OUTPUT_PER_TN_NEURON; dd++) {
                        SpikeData d;
                        d.source_core = this->core_local_id;
                        d.source_neuron = i;
                        d.dest_neuro_tick = this->current_neuro_tick + (1 + delays[i]);
                        d.dest_axon = this->destination_axons[i][dd];
                        d.dest_core = this->destination_cores[i][dd];
                        d.tw_source_time = tw_now(lp);
                        d.source_neuro_tick = this->current_neuro_tick;
                        this->spike_output->save_spike(d);
                    }
                }
                this->fire_status[i] = false;
            }

        }
    }
}

void TrueNorthCore::core_finish(struct tw_lp *lp) {

}

bool TrueNorthCore::is_output_neuron(int neuron_id) {
    bool rv = false;
    for (int i =0; i < MAX_OUTPUT_PER_TN_NEURON; i++)
        rv = rv | (destination_cores[neuron_id][i] < 0 || destination_axons[neuron_id][i] < 0);
    return rv;
}

bool TrueNorthCore::is_self_firing_neuron(int neuron_id) {
    return lambdas[neuron_id] > 0 && sigma_ls[neuron_id] > 0;
}

void TrueNorthCore::compute_weight(nemo_id_type neuron_id, int *synaptic_connectivity, int *G_i, int*sigma, int *b, int *S){
    //G_i is axon types

    for(int ax_id = 0; ax_id < NEURONS_PER_TN_CORE; ax_id ++){
        auto weight_lookup = G_i[ax_id];
        auto syn_con = synaptic_connectivity[ax_id];
        auto n_wt = syn_con * (sigma[weight_lookup] * S[weight_lookup]);
        auto stoc_mode = b[weight_lookup];
        this->weights[neuron_id][ax_id] = n_wt * syn_con;
        this->stochastic_weight_mode[neuron_id][ax_id] = stoc_mode;
    }
}

void
TrueNorthCore::create_tn_neuron(nemo_id_type neuron_id, bool *synaptic_connectivity, short *G_i, short *sigma, short *S,
                                bool *b, bool epsilon, short sigma_l, short lambda, bool c, int alpha, int beta,
                                short TM, short VR, short sigmaVR, short gamma, bool kappa, int signal_delay,
                                const nemo_id_type dest_core[], const nemo_id_type dest_axon[]) {

    //initialize weights
    compute_weight(neuron_id, (int *)synaptic_connectivity,(int *) G_i, (int *) sigma, (int *)b, (int *) S);
    epsilons[neuron_id] = epsilon;
    sigma_ls[neuron_id] = sigma_l;
    lambdas[neuron_id] = lambda;
    c_vals[neuron_id] = c;
    positive_threshold[neuron_id] = alpha;
    negative_threshold[neuron_id] = beta;
    sigma_VRs[neuron_id] = sigmaVR;
    reset_voltages[neuron_id] = VR;
    reset_modes[neuron_id] = gamma;
    kappa_vals[neuron_id] = kappa;
    delays[neuron_id] = signal_delay;

    for(int i = 0; i < MAX_OUTPUT_PER_TN_NEURON; i ++){
        destination_cores[neuron_id][i] = dest_core[i];
        destination_axons[neuron_id][i] = dest_axon[i];
    }
    //threshold bits
    auto Vrst = gen_encoded_random(VR);
    auto Mj = gen_encoded_random(TM);
    this->random_range_leak[neuron_id] = Mj;
    this->random_range_rst[neuron_id] = Vrst;

}

void TrueNorthCore::fire_reset(struct tw_lp *lp){
    //Handles fire and reset of neurons - post leak.
    // good candidate for omp

    for (bool &fire_statu : fire_status) {
        fire_statu = false;
    }
#pragma omp parallel for
    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id ++){
        // Combines fire and reset functions, since they are interlinked in the TN model
        auto nj = (nemo_thresh_type) tw_rand_integer(lp->rng, 0, 255);

        fire_status[neuron_id] = membrane_potentials[neuron_id] >= positive_threshold[neuron_id] + nj;
        // damn if statements!
        if(fire_status[neuron_id]){
            membrane_potentials[neuron_id] = (
                        (dt(reset_modes[neuron_id]) * reset_voltages[neuron_id]) + //δ(γj)Rj
                        (dt(reset_modes[neuron_id] - 1) * (membrane_potentials[neuron_id] - (positive_threshold[neuron_id] + nj))) + //δ(γj − 1)(Vj(t) − (αj + ηj))
                        (dt(reset_modes[neuron_id] - 2) * (membrane_potentials[neuron_id])) //δ(γj −2)Vj(t)
                    );
        }else{ //j(t) < −[βjκj + (βj + ηj)(1 − κj)]
            /** @todo: check this to see if the reverse reset value is being computed properly */
            int neg_th = membrane_potentials[neuron_id] <  (long) ( 0 -  (negative_threshold[neuron_id] * kappa_vals[neuron_id] +
                                                      (negative_threshold[neuron_id] + nj) *(1-kappa_vals[neuron_id]))
                                                      );

            int rrv = (((-1*negative_threshold[neuron_id])*kappa_vals[neuron_id]) +
                        (((-1*(dt(reset_modes[neuron_id])))*reset_voltages[neuron_id]) +
                         ((dt((reset_modes[neuron_id] - 1)))*
                          (membrane_potentials[neuron_id]+ (negative_threshold[neuron_id] + (unsigned int) tw_rand_integer(lp->rng, 0, random_range_rst[neuron_id])))) +
                         ((dt((reset_modes[neuron_id] - 2)))*membrane_potentials[neuron_id]))*
                        (1 - kappa_vals[neuron_id]));

            membrane_potentials[neuron_id] = ( (neg_th * rrv) + (1 - neg_th) * membrane_potentials[neuron_id]);

        }

    }
}

void TrueNorthCore::integrate(struct tw_lp *lp, nemo_id_type input_axon) {
    // This function is a candidate for target offloading.
    // and this possibly could be handled better:
//    static nemo_weight_type stoc_weights[NEURONS_PER_TN_CORE];
//    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id++){
//        stoc_weights[neuron_id] = tw_rand_unif(lp->rng); /** @TODO: fix this to TN Spec @bug */
//    }
//    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id ++){
//        drawn_random_numbers[neuron_id] = tw_rand_integer(lp->rng, 0, 32);
//    }

//Generate random numbers outside of for loop since targeting may happen.
// @todo: Move this to the forward event handler, and call it once per heartbeat.
    nemo_random_type drawn_random_numbers[NEURONS_PER_TN_CORE];
#pragma omp parallel for
    for(int nid = 0; nid < NEURONS_PER_TN_CORE; nid ++){
        drawn_random_numbers[nid] = tw_rand_integer(lp->rng, 0, 255);
    }


#pragma omp parallel for
    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id ++){
        nemo_weight_type prn = drawn_random_numbers[neuron_id];
        nemo_weight_type sj_gi = weights[neuron_id][input_axon];
        bool bj_i = stochastic_weight_mode[neuron_id][input_axon];

        nemo_weight_type  w_result = (
                (1 - bj_i) * sj_gi +
                        (bj_i * bincomp(sj_gi, prn) * sgn(sj_gi))
                );

        membrane_potentials[neuron_id] += w_result;
    }

}

void TrueNorthCore::leak(struct tw_lp *lp, struct tw_bf *bf) {

#pragma omp parallel for
    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id ++) {
        int num_missed_leaks = current_neuro_tick - (int)last_leak_time;
        for(int i = 0; i < num_missed_leaks; i ++) {
            /** @todo: Change the calls to tw_rand_integer to a target friendly function or wrap it into the GPU */
            nemo_weight_type pj = tw_rand_integer(lp->rng, 0, random_range_leak[neuron_id]);
            nemo_weight_type epsilon_j = epsilons[neuron_id];
            nemo_weight_type V_j = membrane_potentials[neuron_id];
            short lambda = lambdas[neuron_id];
            short c_jl = c_vals[neuron_id];


            nemo_weight_type omega = (sigma_ls[neuron_id] * (1 - epsilon_j)) + (epsilon_j * sgn(V_j));


            nemo_weight_type result = V_j + omega * (
                    (1 - c_jl) * lambda +
                    c_jl * bincomp(lambda, pj) * sgn(lambda)
            );
            // Ringing check:
            if (epsilon_j == 1 && (sgn(result) != sgn(membrane_potentials[neuron_id]))) {
                membrane_potentials[neuron_id] = 0;
            } else {
                membrane_potentials[neuron_id] += result;
            }
        }

    }

}

void TrueNorthCore::send_heartbeat(struct tw_lp *lp) {
    struct tw_event *heartbeat_event = tw_event_new(lp->gid, get_next_neurosynaptic_tick(tw_now(lp)), lp);
    nemo_message *msg = (nemo_message *) tw_event_data(heartbeat_event);
    msg->intended_neuro_tick = this->current_neuro_tick + 1;
    msg->message_type = HEARTBEAT;
    msg->nemo_event_status = as_integer(this->evt_stat);
    msg->source_core = this->core_local_id;
    msg->dest_axon = -1;
    msg->debug_time = tw_now(lp);
    tw_event_send(heartbeat_event);
}

void TrueNorthCore::send_heartbeat(struct tw_lp *lp, struct tw_bf *bf) {
    send_heartbeat(lp);
}

unsigned int TrueNorthCore::gen_encoded_random(unsigned int p_mask) {
    return ((unsigned int) 1 )<< p_mask;

}

void TrueNorthCore::ringing() {

}