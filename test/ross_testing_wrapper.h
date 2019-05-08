//
// Created by Mark Plagge on 2019-03-16.
//





#ifndef NEMO2_ROSS_TESTING_WRAPPER_H
#define NEMO2_ROSS_TESTING_WRAPPER_H
#include <core/ross.h>
#include "../src/include/globals.h"
#include "../src/include/CoreLP.h"
#include "nemo_xe.h"

extern std::vector<nemo_message*> message_trace_elements;
extern std::vector<std::string> message_trace_string;


struct DummyLP {


    DummyLP(){
        this->messages = new std::vector<nemo_message*>;
    }
    static void dummy_pre_run(DummyLP *s, tw_lp *lp){
        s->pre_run(lp);

    }
    static void dummy_init(DummyLP *s, tw_lp *lp){
        lp->cur_state = (void *) new DummyLP();
        ((DummyLP *) lp->cur_state)->init(lp);
    }

    static void dummy_forward_event(DummyLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp){
        s->forward_event(bf, m, lp);
    }
    static void dummy_reverse_event(DummyLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp){
        s->reverse_event(bf, m, lp);
    }
    static void dummy_commit(DummyLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp ) {
        s->commit(bf, m, lp);
    }
    static void dummy_final(DummyLP *s, tw_lp *lp){
        s->final(lp);
    }

    static void core_ev_trace(void *msg, tw_lp *lp, char *buffer, int *collect_flag){
        nemo_message *newm = new nemo_message;
        auto m = (nemo_message *) msg;

        newm->intended_neuro_tick =  m->intended_neuro_tick;
        newm->debug_time = m->debug_time;
        newm->random_call_count = m->random_call_count;
        newm->message_type = m->message_type;
        newm->nemo_event_status = m->nemo_event_status;
        newm->source_core = m->source_core;
        newm->dest_axon = m->dest_axon;
        message_trace_elements.push_back(newm);
        auto sti= m->to_string();
        int x = 0;
        for(auto c : sti){
            buffer[x] = c;
            x ++;
        }
        message_trace_string.push_back(sti);




    }


    // dummy lp functions
    virtual void pre_run(tw_lp*lp){
        gid = lp->gid;

    }
    virtual void init(tw_lp*lp){
        std::cout << "Dummy LP Created. in " << lp->id << " -- " << lp->gid << "\n";
    }
    virtual void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp){
        messages->push_back(m);
    }
    virtual void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp){

    }
    virtual void commit(tw_bf *bf, nemo_message *m, tw_lp *lp ) {

    }
    virtual void final(tw_lp *lp){

    }
    virtual void set_core_output_to_me_tn(int core_id){
        tw_lp *lp = tw_getlp(core_id);
        CoreLP *core_lp = (CoreLP *) lp->cur_state;
        TrueNorthCore *tnc = dynamic_cast<TrueNorthCore *> (core_lp->get_core());
        for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
            for(int j = 0; j < WEIGHTS_PER_TN_NEURON; j++) {
                tnc->destination_cores[i][j] = core_id;
                tnc->destination_axons[i][j] = i;
            }
        }

    }
    virtual void send_test_messages() {
        for (auto m : *output_messages) {

            //tw_event *evt = tw_event_new()

        }

    }
    tw_lpid gid;

    std::vector<nemo_message *> *messages;
    std::vector<nemo_message> *output_messages;

};
#endif //NEMO2_ROSS_TESTING_WRAPPER_H