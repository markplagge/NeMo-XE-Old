//
// Created by Mark Plagge on 2019-03-04.
//

#ifndef NEMO2_CORE_LOGGER_H
#define NEMO2_CORE_LOGGER_H


#include <core/ross.h>
#include "../src/include/globals.h"
#include "../src/neuro/INeuroCoreBase.h"

#include "../src/neuro/TrueNorthCore.h"


//st_model_types test_trace[] =  {
//    {(ev_trace_f) core_ev_trace,
//    sizeof(nemo_message),
//    NULL,
//    NULL,
//    NULL,
//    NULL,
//    NULL},
//    {0},
//
//
//};
//
//struct membrane_pot_change{
//    int neuron_id;
//    long double old_value;
//    long double new_value;
//};
struct membrane_pot_log{
    tw_stime event_trigger_time;
//    std::vector<membrane_pot_change> membrane_pot_changes;
    std::vector<nemo_volt_type> original_membrane_pots;
    std::vector<nemo_volt_type> new_membrane_pots;
    bool changed;
    std::string to_string();

};
struct TrueNorthCoreLogger : TrueNorthCore{
    std::vector<nemo_message> messages;
    std::vector<membrane_pot_log> membrane_log;
    std::vector<nemo_volt_type> membrane_potential_v;


    void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

    TrueNorthCoreLogger(int coreLocalId);
    std::string mpot_to_string(bool only_changed);

};
#endif //NEMO2_CORE_LOGGER_H
