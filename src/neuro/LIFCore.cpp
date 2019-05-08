//
// Created by Mark Plagge on 2019-03-12.
//

#include "LIFCore.h"
#include "../include/NeMoConfig.h"
#include "nemo_xe.h"

#define NE_MEMBR(el) this->el[neuron_id]

void LIFCore::core_init(tw_lp *lp) {
    for(auto fs : fire_status){
        fs = false;
    }
    last_leak_time = 0;

    current_neuro_tick = 0;
    previous_neuro_tick = 0;
    if(output_mode > 0){
        this->spike_output = (CoreOutput *) new CoreOutputThread(nemo_config.ne_spike_output_filename);
    }
}

void LIFCore::pre_run(tw_lp *lp) {
    spike_output = new CoreOutputThread("lif_spikes");
}



void LIFCore::forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {
    this->my_lp = lp;
    this->cur_rng_count = lp->rng->count;
    this->cur_message = m;
    this->my_bf = bf;
    this->evt_stat = BF_Event_Status::None;
    // call base heartbeat_forward handler
    this->forward_heartbeat_handler();
    //heartbeats and ticks are updated by that func.

    //Integration if a spike
    if (m->message_type == NEURON_SPIKE){
        evt_stat = BF_Event_Status :: Spike_Rec;
        // integreate //
        auto source_axon = m->dest_axon;
#pragma omp parallel for simd
        for(int neuron_id =0; neuron_id< LIF_NEURONS_PER_CORE; neuron_id ++){
            NE_MEMBR(membrane_pots) += NE_MEMBR(weights)[source_axon];
        }
    }else if (m->message_type == HEARTBEAT){ // leak, reset, and fire
        evt_stat = BF_Event_Status ::Heartbeat_Rec;

        if(m->source_core != core_local_id){
            tw_error(TW_LOC, "core %i got a heartbeat from core %i\n", core_local_id, m->source_core);
        }
        if(leak_needed_count > 0){
            evt_stat = add_evt_status(evt_stat,BF_Event_Status::Leak_Update);
        }
#pragma omp parallel for
        for(int lt = leak_needed_count; lt > 0; lt --){
#pragma omp parallel for simd
            for(int neuron_id =0; neuron_id < LIF_NEURONS_PER_CORE; neuron_id ++){
                NE_MEMBR(membrane_pots) += NE_MEMBR(leak_values);
            }
        }
        //fire detect and reset funct
#pragma omp  parallel for
        for(int neuron_id = 0; neuron_id < LIF_NEURONS_PER_CORE; neuron_id ++){
            if(NE_MEMBR(membrane_pots) >= NE_MEMBR(thresholds)) {
                NE_MEMBR(fire_status) = true;
                NE_MEMBR(membrane_pots) = 0;
                bf->c3 = 1;
            }
        }


    }

}

void LIFCore::reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {

}

void LIFCore::core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) {
    RNG_START(lp);
    if(m->message_type == HEARTBEAT){
        for(int neuron_id = 0; neuron_id < LIF_NEURONS_PER_CORE; neuron_id ++){
            if(NE_MEMBR(fire_status)){
                bf->c4 = 1;
                for(int dv = 0; dv < LIF_NUM_OUTPUTS; dv ++){
                    tw_event *e = tw_event_new(get_gid_from_core_local(NE_MEMBR(destination_cores)[dv],NE_MEMBR(destination_axons)[dv]),get_next_neurosynaptic_tick(tw_now(lp)),lp);
                    auto *msg = (nemo_message *) tw_event_data(e);
                    msg->intended_neuro_tick = current_neuro_tick;//get_next_neurosynaptic_tick(tw_now(lp));
                    msg->dest_axon = NE_MEMBR(destination_axons)[dv];
                    msg->source_core = core_local_id;
                    msg->debug_time = tw_now(lp);
                    msg->message_type = NEURON_SPIKE;
                    RNG_END(lp);
                    tw_event_send(e);
                    //save the spike
                    if(output_mode > 0){
                        this->save_spike(msg,NE_MEMBR(destination_cores)[dv],neuron_id);
                    }
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
        NE_MEMBR(weights)[i] = weights[i];
    }
    for(int i = 0; i < LIF_NUM_OUTPUTS; i ++){
        NE_MEMBR(destination_cores)[i] = destination_cores[i];
        NE_MEMBR(destination_axons)[i] = destination_axons[i];
    }
    NE_MEMBR(leak_values) = leak;
    NE_MEMBR(thresholds) = threshold;



}

LIFCore::LIFCore(int coreID, int outputMode)  {
    core_local_id = coreID;
    output_mode = outputMode;
}
