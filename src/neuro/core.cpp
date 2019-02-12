//
// Created by Mark Plagge on 2019-02-11.
//

#include "core.h"


void Core::event_trace(nemo_message *m, tw_lp *lp, char *buffer, int *collect_flag) {

}

void Core::pre_run(Core *s, tw_lp *lp) {

}

void Core::forward_event(Core *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {
    tw_printf(TW_LOC,"Message recvd.");
}

void Core::reverse_event(Core *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {

}

void Core::core_commit(Core *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {

}

void Core::core_finish(Core *s, tw_lp *lp) {

}

void Core::core_init(Core *s, tw_lp *lp) {
   tw_event_send(tw_event_new(lp->gid,1,lp));
}
