//
// Created by Mark Plagge on 2019-03-12.
//

#include "LIFCore.h"
#define NE_MEM(el) this->el[neuron_id]

void LIFCore::core_init(tw_lp *lp) {
    for(auto fs : fire_status){
        fs = false;
    }
    last_leak_time = 0;
    last_active_time = 0;
    current_big_tick = 0;
    previous_big_tick = 0;
}

void LIFCore::pre_run(tw_lp *lp) {
    spike_output = new CoreOutputThread("lif_spikes");
}

void LIFCore::manage_neurosynaptic_tick(tw_bf *bf, nemo_message *m, tw_lp *lp) {
    //big tick logic:
    //We are at neurosynaptic tick X.
    //We have been active since last tick.
    //If we have an outstanding heartbeat:
    // AND this is a spike AND this is destined for the next tick,
    // this message was recevied out of order. (should not happen)
    // If we do not have an outstanding heartbeat:
    // AND this message is a heartbeat, error
    // Otherwise, we need to retroactively compute the leak for the previous ticks:
    if(heartbeat_sent && m->intended_neuro_tick > current_big_tick){
        tw_error(TW_LOC, "Got a spike with an out of bounds tick.\n");
    }
    if(!heartbeat_sent && m->message_type == HEARTBEAT){
        tw_error(TW_LOC, "Got a heartbeat when no heartbeat was expected.\n");
    }
    //set heartbeat values:
    if(current_big_tick < m->intended_neuro_tick) {
        previous_big_tick = current_big_tick;
        bf->c0 = 1; // big tick change alert.
        current_big_tick = m->intended_neuro_tick;
    }else if (current_big_tick == m->intended_neuro_tick){
            // no tick change, just integrate;
        bf->c0 = 0;
    }else{
        tw_error(TW_LOC, "Invalid tick times.\n ");
    }
}

void LIFCore::forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {
    auto random_call_count = lp->rng->count;
    manage_neurosynaptic_tick(bf,m,lp);
    if (m->message_type == NEURON_SPIKE){

        if(! heartbeat_sent){
            bf->c1 = 1;
            this->heartbeat_sent = true;
            tw_event *e = tw_event_new(lp->gid,get_next_neurosynaptic_tick(tw_now(lp)),lp);
            nemo_message *hb = (nemo_message *) tw_event_data(e);
            hb->intended_neuro_tick = get_next_neurosynaptic_tick(tw_now(lp));
            hb->message_type = HEARTBEAT;
            hb->source_core = coreid;
            hb->random_call_count = random_call_count;
            hb->debug_time = tw_now(lp);
            tw_event_send(e);
        }
        // integreate //
        auto source_axon = m->dest_axon;

#pragma omp parallel for simd
        for(int neuron_id =0; neuron_id< LIF_NEURONS_PER_CORE; neuron_id ++){
            NE_MEM(membrane_pots) += NE_MEM(weights)[source_axon];
        }

    }else if (m->message_type == HEARTBEAT){
        if(m->source_core != coreid){
            tw_error(TW_LOC, "core %i got a heartbeat from core %i\n", coreid, m->source_core);
        }
        // leak


#pragma omp parallel for
        for(int lt = current_big_tick - last_leak_time; lt > 0; lt --){
#pragma omp parallel for simd
            for(int neuron_id =0; neuron_id < LIF_NEURONS_PER_CORE; neuron_id ++){
                NE_MEM(membrane_pots) += NE_MEM(leak_values);
            }
        }
        last_leak_time = current_big_tick;
        bf->c2 = 1;

        //fire & reset
#pragma omp  parallel for
        for(int neuron_id = 0; neuron_id < LIF_NEURONS_PER_CORE; neuron_id ++){
            if(NE_MEM(membrane_pots) > NE_MEM(thresholds)) {
                NE_MEM(fire_status) = true;
                NE_MEM(membrane_pots) = 0;
                bf->c3 = 1;
            }
        }


    }

}

void LIFCore::reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {

}

void LIFCore::core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) {
    if(m->message_type == HEARTBEAT){
        for(int neuron_id = 0; neuron_id < LIF_NEURONS_PER_CORE; neuron_id ++){
            if(NE_MEM(fire_status)){
                bf->c4 = 1;
                for(int dv = 0; dv < LIF_NUM_OUTPUTS; dv ++){
                    tw_event *e = tw_event_new(get_gid_from_core_local(NE_MEM(destination_cores)[dv],NE_MEM(destination_axons)[dv]),get_next_neurosynaptic_tick(tw_now(lp)),lp);
                    nemo_message *m = (nemo_message *) tw_event_data(e);
                    m->intended_neuro_tick = get_next_neurosynaptic_tick(tw_now(lp));
                    m->dest_axon = NE_MEM(destination_axons)[dv];
                    m->source_core = coreid;
                    m->random_call_count = lp->rng->count;
                    m->debug_time = tw_now(lp);
                    m->message_type = NEURON_SPIKE;
                    tw_event_send(e);
                    //save the spike
                    SpikeData s;
                    s.source_core = coreid;
                    s.dest_axon = m->dest_axon;
                    s.source_neuro_tick = current_big_tick;
                    s.dest_neuro_tick = m->intended_neuro_tick;
                    s.dest_core = NE_MEM(destination_cores)[dv];
                    s.source_neuron = neuron_id;
                    s.tw_source_time = tw_now(lp);

                    this->spike_output->save_spike(s);
                }
            }
        }
    }

}

void LIFCore::core_finish(tw_lp *lp) {

}

void LIFCore::create_lif_neuron(nemo_id_type neuron_id, std::array<int, LIF_NEURONS_PER_CORE> weights,
                                std::array<int, LIF_NUM_OUTPUTS> destination_cores,
                                std::array<int, LIF_NEURONS_PER_CORE> destination_axons, int leak, int threshold) {

    for(int i =0; i < LIF_NEURONS_PER_CORE; i ++){
        NE_MEM(weights)[i] = weights[i];
    }
    for(int i = 0; i < LIF_NUM_OUTPUTS; i ++){
        NE_MEM(destination_cores)[i] = destination_cores[i];
        NE_MEM(destination_axons)[i] = destination_axons[i];
    }
    NE_MEM(leak_values) = leak;
    NE_MEM(thresholds) = threshold;



}

LIFCore::LIFCore(int coreID, int outputMode) : coreid(coreID), output_mode(outputMode) {}
