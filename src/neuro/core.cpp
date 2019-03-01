//
// Created by Mark Plagge on 2019-02-11.
//

#include "core.h"
#include "../include/globals.h"
#include "../mapping.h"

void setup_neuron_weights(int neuron_weights[], int core_id, int neuron_id){
    for(int i = 0; i < WEIGHTS_PER_TN_NEURON; i ++){
        neuron_weights[i] = i;
    }
}

void CoreLP::event_trace(nemo_message *m, tw_lp *lp, char *buffer, int *collect_flag) {

}
void CoreLP::create_core(tw_lp *lp){
    // right now we are going to just create the test TN Cores
    // @TODO: Call the LP->Core mapping function
    // and make this point to whatever core type we are creating.

    // Create a neuro core based on a function return value
    // @TODO: Implement this
    ////

    //Now only TN Cores
    int coreLocalId = lp->id;

    auto ccore = new TrueNorthCore(coreLocalId);
    ccore->core_init(lp);
    this->core = ccore;
    this->active = 1;

}




TrueNorthCore::TrueNorthCore(int coreLocalId) : core_local_id(coreLocalId) {
    // load up TrueNorth neuron info.
    //for each neuron in the core:
    //load the TN parameters for that neuron
    //set neuron weights
    // set other neruon params
    this->last_active_time = 0;
    this->last_leak_time = 0;
}
// Seperate function for init.
void TrueNorthCore::core_init(tw_lp *lp) {
    std::cout << "Initializing core # " << get_core_from_gid(lp->gid) << "\n";
    //assuming linear mapping.
    this->core_local_id = get_core_from_gid(lp->gid);

}




// TrueNorth Core




void TrueNorthCore::pre_run(tw_lp *lp) {
    // determine if we have self_firing neurons

    for (int i = 0; i < NEURONS_PER_TN_CORE; i ++){
        has_self_firing_neuron = has_self_firing_neuron | is_self_firing_neuron(i);
    }
    if (has_self_firing_neuron){
        // need to send ourselves a heartbeat out of the gate
        send_heartbeat(lp);
    }
}

void TrueNorthCore::forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {
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
        //bool did_fire = this->fire(lp, bf);
        this->fire_reset(lp);
        // Send those fire events:
        for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
            if(this->fire_status[i]){
                evt_stat |= BF_Event_Status :: Spike_Sent;

                for(int j = 0; j < MAX_OUTPUT_PER_TN_NEURON; j ++) {
                    tw_lpid dest_gid = get_gid_from_core_local(destination_cores[i][j],destination_axons[i][j]);

                    tw_stime dest_time = this->delays[i] + JITTER(lp->rng,random_call_count);
                    tw_event *evt = tw_event_new(dest_gid,dest_time,lp);
                    nemo_message *msg = (nemo_message *)tw_event_data(evt);
                    msg->message_type=NEURON_SPIKE;
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


void TrueNorthCore::reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {
    if (m->message_type == NEURON_SPIKE){

    }

}

void TrueNorthCore::core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) {
    for(int i = 0; i < NEURONS_PER_TN_CORE; i ++) {
        if (this->fire_status[i])
            this->fire_status[i] = false;
    }
}

void TrueNorthCore::core_finish(tw_lp *lp) {

}



bool TrueNorthCore::is_output_neuron(int neuron_id) {
    bool rv = false;
    for (int i =0; i < MAX_OUTPUT_PER_TN_NEURON; i++)
        rv = rv | (destination_cores[neuron_id][i] < 0 || destination_axons[neuron_id][i] < 0);
    return rv;
}

bool TrueNorthCore::is_self_firing_neuron(int neuron_id) {
    return false;
}

void
TrueNorthCore::create_tn_neuron(nemo_id_type neuron_id, bool *synaptic_connectivity, short *G_i, short *sigma, short *S,
                                bool *b, bool epsilon, short sigma_l, short lambda, bool c, int alpha, int beta,
                                short TM, short VR, short sigmaVR, short gamma, bool kappa, int signal_delay,
                                nemo_id_type dest_core, nemo_id_type dest_axon) {

}



void TrueNorthCore::fire_reset(tw_lp *lp){
    //Handles fire and reset of neurons - post leak.
    // good candidate for omp

    for (bool &fire_statu : fire_status) {
        fire_statu = false;
    }
#pragma omp parallel for
    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id ++){
        // Combines fire and reset functions, since they are interlinked in the TN model
        auto nj = gen_encoded_random(drawn_random_numbers[neuron_id], threshold_PRN_mask[neuron_id]);
        fire_status[neuron_id] = membrane_potentials[neuron_id] >= positive_threshold[neuron_id] + nj;
        // damn if statements!
        if(fire_status[neuron_id]){
            membrane_potentials[neuron_id] = (
                        (dt(reset_modes[neuron_id]) * reset_voltages[neuron_id]) + //δ(γj)Rj
                        (dt(reset_modes[neuron_id] - 1) * (membrane_potentials[neuron_id] - (positive_threshold[neuron_id] + nj))) + //δ(γj − 1)(Vj(t) − (αj + ηj))
                        (dt(reset_modes[neuron_id] - 2) * (membrane_potentials[neuron_id])) //δ(γj −2)Vj(t)
                    );
        }else{ //j(t) < −[βjκj + (βj + ηj)(1 − κj)]
            auto neg_th = membrane_potentials[neuron_id] <  ( 0 -  (negative_threshold[neuron_id] * kappa_vals[neuron_id] +
                                                      (negative_threshold[neuron_id] + nj) *(1-kappa_vals[neuron_id]))
                                                      );
            auto rrv = (((-1*negative_threshold[neuron_id])*kappa_vals[neuron_id]) +
                        (((-1*(dt(reset_modes[neuron_id])))*reset_voltages[neuron_id]) +
                         ((dt((reset_modes[neuron_id] - 1)))*
                          (membrane_potentials[neuron_id]+ (negative_threshold[neuron_id] + drawn_random_numbers[neuron_id]))) +
                         ((dt((reset_modes[neuron_id] - 2)))*membrane_potentials[neuron_id]))*
                        (1 - kappa_vals[neuron_id]));
            membrane_potentials[neuron_id] = ( (neg_th * rrv) + (1 - neg_th) * membrane_potentials[neuron_id]);

        }

    }
}

void TrueNorthCore::ringing(nemo_id_type neuron_id) {

}

void TrueNorthCore::integrate(tw_lp *lp, nemo_id_type input_axon) {
    // This function is a candidate for target offloading.
    // and this possibly could be handled better:
//    static nemo_weight_type stoc_weights[NEURONS_PER_TN_CORE];
//    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id++){
//        stoc_weights[neuron_id] = tw_rand_unif(lp->rng); /** @TODO: fix this to TN Spec @bug */
//    }
//    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id ++){
//        drawn_random_numbers[neuron_id] = tw_rand_integer(lp->rng, 0, 32);
//    }
    generate_prns(drawn_random_numbers,lp);
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





void TrueNorthCore::leak(tw_lp *lp, tw_bf *bf) {


    for(int neuron_id = 0; neuron_id < NEURONS_PER_TN_CORE; neuron_id ++) {
        nemo_weight_type pj = drawn_random_numbers[neuron_id]; //tw_rand_integer(lp->rng,0,32);
        nemo_weight_type epsilon_j = epsilons[neuron_id];
        nemo_weight_type V_j = membrane_potentials[neuron_id];
        short lambda = lambdas[neuron_id];
        bool c_jl = c_vals[neuron_id];


        nemo_weight_type omega = (sigma_ls[neuron_id] * (1 - epsilon_j) ) +(epsilon_j * sgn(V_j));


        nemo_weight_type result = V_j + omega * (
                (1 - c_jl) * lambda +
                c_jl * bincomp(lambda, pj) * sgn(lambda)
                );
        membrane_potentials[neuron_id] += result;
    }

}


void TrueNorthCore::send_heartbeat(tw_lp *lp) {
    tw_event *heartbeat_event = tw_event_new(lp->gid, get_next_neurosynaptic_tick(tw_now(lp)),lp);
    nemo_message *msg = (nemo_message *) tw_event_data(heartbeat_event);
    msg->intended_neuro_tick = this->current_neuro_tick + 1;
    msg->message_type = HEARTBEAT;
    msg->nemo_event_status = as_integer(this->evt_stat);
    msg->source_core = this->core_local_id;
    msg->dest_axon = -1;
    tw_event_send(heartbeat_event);
}

void TrueNorthCore::send_heartbeat(tw_lp *lp, tw_bf *bf) {
    send_heartbeat(lp);
}

unsigned int TrueNorthCore::gen_encoded_random(unsigned int pj, unsigned int p_mask) {
    return pj & p_mask;
}

