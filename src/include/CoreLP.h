//
// Created by Mark Plagge on 2019-03-13.
//

#ifndef NEMO2_CORELP_H
#define NEMO2_CORELP_H
#include "../neuro/INeuroCoreBase.h"
#include "../neuro/LIFCore.h"
#include "../neuro/TrueNorthCore.h"
#include <ross.h>
#include "globals.h"
#include "mapping.h"

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
 * Updated - Since we are getting elements passed to us through ROSS C-Style (pointers),
 * We will use function templates to manage this?
 *
 * A CoreLP contains a reference to the INeuroCoreBase class (*core), which is managed
 * here. Basic core logic is handled by this class.
 *
 * Basically, the CoreLP class is a holder for the functions that ROSS expects and the neuron object.
 * Logic that is not related to neurosynaptic core functionality is handled here, then control is handed off
 * to the inner INEuroCoreBase class.
 *
 * Rather than have ROSS pass around INeuroCoreBase objects directly, it will pass around
 */
class CoreLP {
public:


    static void event_trace(nemo_message *m, tw_lp *lp, char *buffer, int *collect_flag);\
    /**
 * ROSS function for initializing this core LP. Creates a new instance of a neurosynaptic core.
 * Calls the instantiated
 * @param s
 * @param lp
 */
    static void core_init(CoreLP *s, tw_lp *lp) {
        //determine the type of core we want through mapping
        s->create_core(lp);
    }

    int core_creation_mode = 0;

    static void pre_run(CoreLP *s, tw_lp *lp) { s->core->pre_run(lp); }

    static void forward_event(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) { s->core->forward_event(bf, m, lp); }

    static void reverse_event(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) { s->core->reverse_event(bf, m, lp); }

    static void core_commit(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) { s->core->core_commit(bf, m, lp); }

    static void core_finish(CoreLP *s, tw_lp *lp) {s->core->core_finish(lp);s->core->cleanup_output();};

    void create_core(tw_lp *lp);

    INeuroCoreBase *getCore() const {
        return core;
    }

    void setCore(INeuroCoreBase *core);

private:
    INeuroCoreBase *core;
    int active;
    core_types core_type;
};
#endif //NEMO2_CORELP_H
