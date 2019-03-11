//
// Created by Mark Plagge on 2019-03-04.
//

#include "TrueNorthCoreLogger.h"


//core event trace
void core_ev_trace(void *msg, tw_lp *lp, char *buffer, int *collect_flag){
    //event tracing here - if used.
}

void TrueNorthCoreLogger::forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {
    auto pre_mp_log = membrane_pot_log();
    auto newm = new nemo_message;
    newm = m;
    this->messages.push_back(*newm);
    pre_mp_log.event_trigger_time = tw_now(lp);
    //pre_mp_log.original_membrane_pots = this->membrane_potential_v;
    pre_mp_log.original_membrane_pots.assign(membrane_potential_v.begin(),membrane_potential_v.end());

    TrueNorthCore::forward_event(bf,m,lp);

    for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
        membrane_potential_v[i] = TrueNorthCore::membrane_potentials[i];
    }

    pre_mp_log.new_membrane_pots = this->membrane_potential_v;
    this->membrane_log.push_back(pre_mp_log);
    if(pre_mp_log.original_membrane_pots != pre_mp_log.new_membrane_pots) {
        pre_mp_log.changed= true;
    }else{
        pre_mp_log.changed=false;
    }


}

TrueNorthCoreLogger::TrueNorthCoreLogger(int coreLocalId) : TrueNorthCore(coreLocalId, 0) {
    this->last_active_time = 0;
    this->last_leak_time = 0;
    for(int i = 0; i <NEURONS_PER_TN_CORE; i ++){
        this->membrane_potential_v.push_back(0);
    }

}

std::string TrueNorthCoreLogger::mpot_to_string(bool only_changed) {
    std::string result_str;

    for (auto mlog : membrane_log){
        std::string log = mlog.to_string();
        // I'm tired, this seems convoluted
        if(only_changed){
            if(mlog.changed){
                result_str.append(log);
            }
        }else{
            result_str.append(log);
        }


    }
    return result_str;

}

std::string membrane_pot_log::to_string() {
    std::string result_string ;

    for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
        char bf[32];
        sprintf(bf,"%i,%i,%lf\n", original_membrane_pots[i],new_membrane_pots[i],event_trigger_time);
        result_string.append(bf);

    }
    result_string.append("-,-,-\n");
    return result_string;

}
