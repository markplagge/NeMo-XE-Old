//
// Created by Mark Plagge on 2019-03-12.
//

#ifndef NEMO2_LIFCORE_H
#define NEMO2_LIFCORE_H

#include "INeuroCoreBase.h"
#include <array>
#include <string>
#include "../include/CoreOutput.h"
#include "../include/NeMoXe.h"


struct LIFCore: public INeuroCoreBase {

    LIFCore(int coreid, int outputMode);


    void core_init(tw_lp *lp) override ;
    void pre_run(tw_lp *lp) override ;
    void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override ;
    void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override ;
    void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp)override ;
    void core_finish(tw_lp *lp)override ;


    void create_lif_neuron(nemo_id_type neuron_id, std::array<int,LIF_NEURONS_PER_CORE> weights, std::array <int,LIF_NUM_OUTPUTS> destination_cores,
                           std::array<int, LIF_NEURONS_PER_CORE> destination_axons, int leak, int threshold);

    void manage_neurosynaptic_tick(tw_bf *bf, nemo_message *m, tw_lp *lp);
    Matrix <int,LIF_NEURONS_PER_CORE,LIF_NEURONS_PER_CORE> weights;
    std::array<int,LIF_NEURONS_PER_CORE> membrane_pots;
    Matrix <int,LIF_NEURONS_PER_CORE,LIF_NUM_OUTPUTS> destination_cores;
    Matrix <int,LIF_NEURONS_PER_CORE,LIF_NUM_OUTPUTS> destination_axons;
    std::array<int,LIF_NEURONS_PER_CORE> leak_values;
    std::array<int,LIF_NEURONS_PER_CORE> thresholds;
    std::array<bool,LIF_NEURONS_PER_CORE> fire_status;



};


#endif //NEMO2_LIFCORE_H
