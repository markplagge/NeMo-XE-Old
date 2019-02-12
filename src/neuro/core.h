//
// Created by Mark Plagge on 2019-02-11.
//

#ifndef NEMO2_CORE_H
#define NEMO2_CORE_H
#include <ross.h>
#include "../globals.h"

class Core {
public:
    static void event_trace(nemo_message *m, tw_lp *lp, char *buffer, int *collect_flag);
    static void core_init(Core *s, tw_lp *lp);
    static void pre_run(Core *s, tw_lp *lp);
    static void forward_event(Core *s, tw_bf *bf, nemo_message *m, tw_lp *lp);
    static void reverse_event(Core *s, tw_bf *bf, nemo_message *m, tw_lp *lp);
    static void core_commit(Core *s, tw_bf *bf, nemo_message *m, tw_lp *lp);
    static void core_finish(Core *s, tw_lp *lp);


private:

    //void nemo_forward_evt(tw_lp *lp, )

};


#endif //NEMO2_CORE_H
