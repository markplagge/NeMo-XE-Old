//
// Created by Mark Plagge on 2019-02-11.
//

#ifndef NEMO2_INEUROCOREBASE_H
#define NEMO2_INEUROCOREBASE_H
#include <ross.h>
#include "../include/globals.h"
#include "neuron_generic.h"
#include "../include/CoreOutput.h"
#include <vector>

#define RNG_START(lp) auto rng_count = lp->rng->count
#define RNG_END(lp)  msg->random_call_count = (lp->rng->count - rng_count)
/**
 * @defgroup nemo_cores NeMo2 Core Definitions
 * NeMo2 neurosynaptic cores, as well as the ROSS LP wrapper.
 * NeMo2 uses a "fat lp" technique. Neurons are defined in a core.
 * To wrap the LP state from ROSS, we use the CoreLP class, which
 * holds a INeuroCoreBase object.
 *
 * These classes are the base core definitions for NeMo.
 * @{
 */

//


/**
 * crtp helper macro. Helps keep the CRTP polymorphism code to a manageble level.
 * @tparam T
 */
#define IMPL(f,c) public (f)<c>

//
//
//template<typename CORE_BASE>
//struct NeuroCoreBasis : crtp<CORE_BASE>{
//    void core_init_s(tw_lp *lp){
//        this()->underlying()->core_init(lp);
//    }
//private:
//
//};
//
//
//template <typename CORE_BASE>
//struct NeuroIntegrateBasis : crtp<CORE_BASE>{
//    void core_integrate(tw_bf *bf, nemo_message *m, tw_lp *lp){
//        this->underlying()->integrate(bf, m, lp);
//    }
//    void reverse_core_integrate(tw_bf *bf, nemo_message *m, tw_lp *lp){
//        this->underlying()->reverse_integrate(bf, m,lp);
//    }
//    void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp){
//        this->underlying().forward_event_impl(bf,m,lp);
//    }
//
//
//};


/**
 * INeuroCoreBase
 * Base interface for neuromorphic cores in NeMo. Defines the expected functions
 * that will be used by neuromorphic cores.
 *
 * Also implements basic heartbeat code.
 */
struct INeuroCoreBase{

    INeuroCoreBase();

    virtual void core_init(tw_lp *lp) = 0;
    virtual void pre_run(tw_lp *lp) = 0;
    virtual void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void core_finish(tw_lp *lp) = 0;

    /**
     * forward_loop_handler(). Neurosynaptic generally have common functionality - there
     * is a neurosynaptic tick (handled by heartbeat messages), integration, and leak functions.
     * Common code for forward events is here - this function handles basic versions of:
     * - the heartbeat / neurosnaptic tick sync
     * -
     *
     *
     *
     */
    virtual void forward_heartbeat_handler();
    virtual void reverse_heartbeat_handler();
    virtual void send_heartbeat();


    void save_spike(nemo_message *m,long dest_core, long neuron_id);

    void cleanup_output();

    CoreOutput *spike_output;
    /**
     * output_mode - sets the spike output mode of this core.
     * Mode 0 is no output,
     * Mode 1 is output spikes only
     * Mode 2 is all spikes output
     */
    int output_mode = 2;
    /**
 * The last time that this core had activity. This refers to any  message.
 */

    long last_active_time = 0;
    long current_neuro_tick = 0;
    long previous_neuro_tick = -1;
    /**
 * The last time this core computed the leak.
 */
    long last_leak_time= 0 ;
    long leak_needed_count = 0;

    /**
     * A heartbeat check value.
     */
    bool heartbeat_sent;
    /**
     * * the local core id. If linear mapping is enabled, then this will be equal to the GID/PE id
 */

    int core_local_id;
    /**
     * my_lp -> current lp state, holds the lp state given to us from the calling function.
     */
    tw_lp *my_lp;
    tw_bf *my_bf;
    /**
     * Current message holder - @todo: may not need this.
     */
    nemo_message *cur_message;
    /**
     * random number generator counter - used to keep the RNG counter state intact through various calls.
     */
    unsigned long cur_rng_count;

    /**
     * swtich to enable heartbeat timing debug writes.
     */
    bool debug_heartbeat = false;


    /**
 * evt_stat holds the event status for the current event. This is used to compute
 * reverse computation. BF_Event_Stats is used instead of the tw_bf as it allows
 * more explicit naming. The concept is the same, however.
 */
    BF_Event_Status evt_stat;


    std::vector<double> heartbeat_scheduled_send_times;
    std::vector<nemo_message> nemo_message_debug;

};








/** @} */
#endif //NEMO2_INEUROCOREBASE_H
