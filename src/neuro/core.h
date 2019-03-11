//
// Created by Mark Plagge on 2019-02-11.
//

#ifndef NEMO2_CORE_H
#define NEMO2_CORE_H
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
 * @{
 */
/**
 * INeuroCoreBase
 * Base interface for neuromorphic cores in NeMo. Defines the expected functions
 * that will be used by neuromorphic cores.
 */
struct INeuroCoreBase {

    INeuroCoreBase();

    virtual void core_init(tw_lp *lp) = 0;
    virtual void pre_run(tw_lp *lp) = 0;
    virtual void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void core_finish(tw_lp *lp) = 0;


    CoreOutput *spike_output;
    /**
     * output_mode - sets the spike output mode of this core.
     * Mode 0 is no output,
     * Mode 1 is output spikes only
     * Mode 2 is all spikes output
     */
    int output_mode = 2;


};

//////////////////////
/**
 * CoreLP - This class is an adapter for ROSS to the C++ object system in NeMo2.
 * Each LP in ROSS is of type CoreLP - and each CoreLP contains one reference to a
 * neuromorphic core class.
 *
 * Hopefully this abstraction will not hurt performance.
 * This class breaks some OO rules, as it will need to talk to the mapping and configuation
 * parsing systems directly - can't pass it anything but what is expected from ROSS.
 *
 *
 */
class CoreLP{
public:
    static void event_trace(nemo_message *m, tw_lp *lp, char *buffer, int *collect_flag);
    static void core_init(CoreLP *s, tw_lp *lp) {
        //determine the type of core we want through mapping
        s->create_core(lp);
    }
    int core_creation_mode = 0;



    static void pre_run(CoreLP *s, tw_lp *lp) {s->core->pre_run(lp);}

    static void forward_event(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {s->core->forward_event(bf,m,lp);}

    static void reverse_event(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {s->core->reverse_event(bf,m,lp); }

    static void core_commit(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {s->core->core_commit(bf,m,lp);}

    static void core_finish(CoreLP *s, tw_lp *lp) {s->core->core_finish(lp);}

    void create_core(tw_lp *lp);

    INeuroCoreBase *getCore() const {
        return core;
    }

    void setCore(INeuroCoreBase *core);

private:
    INeuroCoreBase  *core;
    int active;





};

/** @} */
#endif //NEMO2_CORE_H
