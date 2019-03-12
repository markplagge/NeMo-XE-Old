//
// Created by Mark Plagge on 2019-03-12.
//

#ifndef NEMO2_LIFCORE_H
#define NEMO2_LIFCORE_H
#include "core.h"
#include <array>
#include <string>
#include "../include/CoreOutput.h"
// LIF Core settings:
constexpr int LIF_NEURONS_PER_CORE = 256;
constexpr int LIF_NUM_OUTPUTS = 256;


class LIFCore: public INeuroCoreBase {
public:
    LIFCore(int coreid, int outputMode);

private:
    void core_init(tw_lp *lp);
    void pre_run(tw_lp *lp);
    void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
    void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
    void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp);
    void core_finish(tw_lp *lp);


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

    double last_active_time;
    unsigned int current_big_tick;
    unsigned int previous_big_tick;
    int last_leak_time;
    bool heartbeat_sent;
    int output_mode;
    int coreid;




};


#endif //NEMO2_LIFCORE_H
