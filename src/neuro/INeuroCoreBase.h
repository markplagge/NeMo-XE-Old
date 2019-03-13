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
/**
 * crtp helper / basis class. Helps keep static polymorphism function boilerplate code managable.
 * @tparam T
 */
template <typename T>
struct crtp
{
    T& underlying() { return static_cast<T&>(*this); }
    T const& underlying() const { return static_cast<T const&>(*this); }
};


template<typename CORE_BASE>
class NeuroCoreBasis : crtp<CORE_BASE>{
    void core_init_s(tw_lp *lp){
        this()->underlying()->core_init(lp);
    }
private:

};


template <typename CORE_BASE>
class NeuroIntegrateBasis : crtp<CORE_BASE>{
    void core_integrate(tw_bf *bf, nemo_message *m, tw_lp *lp){
        this->underlying()->integrate(bf, m, lp);
    }
    void reverse_core_integrate(tw_bf *bf, nemo_message *m, tw_lp *lp){
        this->underlying()->reverse_integrate(bf, m,lp);
    }
};
/**
 * INeuroCoreBase
 * Base interface for neuromorphic cores in NeMo. Defines the expected functions
 * that will be used by neuromorphic cores.
 *
 * Also implements basic heartbeat code.
 */
struct INeuroCoreBase: public NeuroIntegrateBasis<INeuroCoreBase>{

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
     * @todo : these should *not* be virtual, rather use CRTP to prevent excessive virtual function calls?
     *
     */
    virtual void forward_handler();
    virtual void reverse_handler();
    virtual void send_heartbeat();
    virtual void integrate() = 0;
    virtual void reverse_integrate();


    CoreOutput *spike_output;
    /**
     * output_mode - sets the spike output mode of this core.
     * Mode 0 is no output,
     * Mode 1 is output spikes only
     * Mode 2 is all spikes output
     */
    int output_mode = 2;


    double last_active_time;
    unsigned int current_neuro_tick;
    unsigned int previous_neuro_tick;
    int last_leak_time;

    bool heartbeat_sent;
    int coreid;
    tw_lp *my_lp;
    tw_bf *my_bf;
    nemo_message *cur_message;
    unsigned long cur_rng_count;



};








/** @} */
#endif //NEMO2_INEUROCOREBASE_H
