#include <gtest/gtest.h>
#include "../external/ROSS/core/ross.h"
#include "../src/neuro/core.h"
#include "../src/mapping.h"
#include "TrueNorthCoreLogger.h"
#include "../src/neuro/TrueNorthCore.h"

#include <iostream>
#include <fstream>
static std::vector<nemo_message*> message_trace_elements;
static std::vector<std::string> message_trace_string;
char * SPIKE_OUTPUT_FILENAME;
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
        TrueNorthCore *tnc = dynamic_cast<TrueNorthCore *> (core_lp->getCore());
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

/** TN Core init function override for AllSpikeTest */
void core_init_test(CoreLP * core_lp, tw_lp *lp){



    //run the core initialization:
    CoreLP::core_init(core_lp, lp);
    TrueNorthCore *core = (TrueNorthCore *) ((CoreLP*)lp->cur_state)->getCore();
    for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
        for(int j = 0; j < NEURONS_PER_TN_CORE; j ++) {
            core->weights[i][j] = 1;


        }
        for(int j = 0; j < MAX_OUTPUT_PER_TN_NEURON; j ++){
            core->destination_cores[i][j] = 1;
            core->destination_axons[i][j] = 0;
        }
        core->epsilons[i] = false;
        core->c_vals[i] = false;

    }
    //send one spike



}

void core_pre_run_test(CoreLP *s, tw_lp *lp){
    auto cclp = (CoreLP *)lp->cur_state;
    cclp->getCore()->core_init(lp);
    tw_event * evt = tw_event_new(lp->gid, 0.1, lp);
    nemo_message *msg = (nemo_message*) tw_event_data(evt);
    msg->message_type = NEURON_SPIKE;
    msg->source_core = 0;
    msg->dest_axon = 0;
    msg->intended_neuro_tick = 1;
    tw_event_send(evt);

}

tw_lptype mylps[] = {
        {(init_f)core_init_test,

         (pre_run_f) core_pre_run_test,
         (event_f) CoreLP::forward_event,
         (revent_f) CoreLP::reverse_event,
         (commit_f) CoreLP::core_commit,
         (final_f) CoreLP::core_finish,
         (map_f) nemo_map_linear,
         sizeof(INeuroCoreBase)},
        {
                (init_f)    DummyLP::dummy_init,
                (pre_run_f) DummyLP::dummy_pre_run,
                (event_f)   DummyLP::dummy_forward_event,
                (revent_f)  DummyLP::dummy_reverse_event,
                (commit_f)  DummyLP::dummy_commit,
                (final_f)   DummyLP::dummy_final,
                (map_f) nemo_map_linear,
                sizeof(DummyLP)
        },
        {0},
};



// hooks into ROSS for testing:

//tw_petype main_pe={
//        (pe_init_f) nemo_pre_lp_init,
//        (pe_init_f) nemo_post_lp_init,
//        (pe_gvt_f) nemo_pe_gvt_f,
//        0
//};

// PE Functionality callback
class CallbackPE{
public:
    virtual void pre_lp_init_test(tw_pe *pe) const;
    virtual void post_lp_init_test(tw_pe *pe) const;
    virtual void pe_gvt_test(tw_pe *pe) const;
protected:
    int ( *callback)(tw_pe *pe);

};
st_model_types test_trace[] =  {
        {(ev_trace_f) DummyLP::core_ev_trace,
                sizeof(nemo_message),
                NULL,
                NULL,
                NULL,
                NULL,
                NULL},
        {0},


};
class CoreTest : public ::testing::Test {
protected:
    void SetUp() override{
        SPIKE_OUTPUT_FILENAME = (char*) calloc(128,sizeof(char));
        sprintf(SPIKE_OUTPUT_FILENAME, "test_case_output");

        test_core = new TrueNorthCore(0, 0); // core 0 tests;
//        lp = (tw_lp *) calloc(1, sizeof(tw_lp));
//        lp->rng = rng_init(1,2);
//        tw_rand_init_streams(lp,1);
//        lp->id=0;
//        lp->gid=0;
    all_connected_weight_one_core = new TrueNorthCore(0, 0);
    //manually set up the weights for this one
    for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
        for(int j = 0; j < NEURONS_PER_TN_CORE; j ++) {
            all_connected_weight_one_core->weights[i][j] = 1;
        }
        all_connected_weight_one_core->positive_threshold[i] = 1;
        all_connected_weight_one_core->reset_voltages[i] = 0;
        all_connected_weight_one_core->reset_modes[i] = 0;
        for(int j = 0; j < MAX_OUTPUT_PER_TN_NEURON; j ++){
        all_connected_weight_one_core->destination_axons[i][j] = i;
        all_connected_weight_one_core->destination_cores[i][j] = i;
        destination1_axons.push_back(i);
        destination1_cores.push_back(i);
        }

    }
    g_tw_nlp = 1;
    g_tw_synchronization_protocol=tw_synch_e::CONSERVATIVE;
    //set_init_ross(1);
    //set_lp(0);
    //tw_run();


    }
    void set_lp(int lpid){
        lp = tw_getlp(lpid);
    }

    /// vars

    std::vector<int> destination1_axons;
    std::vector<int> destination1_cores;
    TrueNorthCore *test_core;
    TrueNorthCore *all_connected_weight_one_core;
    tw_lp *lp;


    void set_init_ross(int nlp){



        g_tw_nlp = nlp;
        g_tw_synchronization_protocol = tw_synch_e :: CONSERVATIVE;
        int *argc = (int *)calloc(sizeof(int),1);
        char **argv = (char **) calloc(sizeof(char*), 3);
        argc[0] = 2;
        argv[0] = (char * ) calloc(sizeof(char), 1);
        argv[1] = (char * ) calloc(sizeof(char), 64);
        argv[2] = "\0";
        sprintf(argv[1],"--event-trace=1");

        g_tw_events_per_pe = 256 * 256;

        tw_init(argc,&argv);
        tw_define_lps(2,sizeof(nemo_message));
        //tw_pe_settype(tw_getpe(0), get_this_test_pes());
        //and manual things
        tw_pe * me;
        int i ;
        for(i = 0; i < g_tw_nlp - 1; i++)
        {
            tw_lp_settype(i, &mylps[0]);
            st_model_settype(i, &test_trace[0]);
        }
        tw_lp_settype(i, &mylps[1]);
    }




};


/**
 * @test
 * Tests a TN Core's basic spike functions.
 * Creates one core, sets the weights to one, reset values to zero, and output "core" destination to the dummy
 * collector.
 *
 */
//TEST_F(CoreTest, AllSpikeTest){
//    // we want to stop the lp
//
//    mylps[0].init   = (init_f) core_init_test;
//    g_tw_ts_end = 30;
//    set_init_ross(2);
//
//    DummyLP *stat_lp = (DummyLP*) tw_getlp(1)->cur_state;
//    //TrueNorthCore *core = (TrueNorthCore *) ((CoreLP*)tw_getlp(0)->cur_state)->getCore();
//    //stat_lp->set_core_output_to_me_tn(0);
//    tw_run();
//    /* check the dummy lp events */
//    //ASSERT_EQ(stat_lp->messages->size(), 256);
//
//    std::cout <<"Times: \n" ;
//    for(auto m : *stat_lp->messages){
//        ASSERT_EQ(m->message_type, 0);
//        ASSERT_GE(m->intended_neuro_tick,2);
//        ASSERT_LT(m->intended_neuro_tick,3);
//        std::cout << m->debug_time << "\t|";
//    }
//
//}


/** @test
 * leak tests
 */

void test_leak_pe_init_one(tw_pe *){
    std::cout<<"Post init PE \n";
    auto *core_lp = (CoreLP *)tw_getlp(0)->cur_state;
    auto logger_core = new TrueNorthCoreLogger(0);
    logger_core->core_init(tw_getlocal_lp(0));
    core_lp->setCore(logger_core);

    auto tn_core = (TrueNorthCoreLogger *) core_lp->getCore();
    //set up leaks based on test pattern:
    /* n0 : negative leak 1
     * n1 : negative leak 2
     * n2 : positive leak 1
     * n3 : positive leak 2
     * n4 : stochastic leak, probability .5
     *
     * initial membrane pots of 5,
     * weights of +1 (from the testing harness)
     *
     */

    tn_core->lambdas[0] = 1;
    tn_core->lambdas[1] = 2;
    tn_core->lambdas[2] = 1;
    tn_core->lambdas[3] = 2;
    tn_core->lambdas[4] = 50;

    tn_core->sigma_ls[0] = -1;
    tn_core->sigma_ls[1] = -1;
    tn_core->sigma_ls[2] = 1;
    tn_core->sigma_ls[3] = 1;
    tn_core->sigma_ls[4] = 1;

    tn_core->c_vals[4] = true;

    for(int i = 0; i < 5; i ++)
        tn_core->membrane_potential_v[i] = 0;

    // now queue up the messages:
    tw_lp *lp = tw_getlp(0);
    auto cclp = (CoreLP *)lp->cur_state;
    cclp->getCore()->core_init(lp);
    tw_event * evt = tw_event_new(lp->gid, 0.1, lp);
    nemo_message *msg = (nemo_message*) tw_event_data(evt);
    msg->message_type = NEURON_SPIKE;
    msg->source_core = 128;
    msg->dest_axon = 0;
    msg->intended_neuro_tick = 1;
    tw_event_send(evt);

}
tw_petype test_pe = {
        NULL, //(pe_init_f) test_leak_pe_init_one,
        (pe_init_f) test_leak_pe_init_one,
        NULL,
        0
};

/**
 * @test
 * Create a TN Core, and test linear leak
 */
 TEST_F(CoreTest, CoreLeakTime1){

    mylps[0].init   = (init_f) core_init_test;
    g_tw_ts_end = 30;


    set_init_ross(2);
    tw_pe_settype(tw_getpe(0), &test_pe);
    auto pe = tw_getpe(0);
    pe->type.post_lp_init = (pe_init_f) test_leak_pe_init_one;
    tw_run();

    std::ofstream messages;
    messages.open("core_leak_test_messages.csv");
    messages<< "message_type_desc,source_core,dest_axon,intended_neuro_tick,nemo_event_status,"
               "random_call_count,debug_time\n";
    for(auto str_dsc : message_trace_string){
        messages << str_dsc;
    }
    messages.close();
    messages.open("core_delta_mpot.csv");
    messages <<"original mpot,new_mpot,time\n";

    auto stat_lp = (DummyLP*) tw_getlp(1)->cur_state;
    auto core = (TrueNorthCoreLogger*)((CoreLP*)tw_getlp(0)->cur_state)->getCore();

    messages <<core->mpot_to_string(true);
    messages.close();

    auto n1_mp = core->membrane_potential_v[0];
    auto n2_mp = core->membrane_potential_v[1];
    auto n3_mp = core->membrane_potential_v[3];
    auto n4_mp = core->membrane_potential_v[4];
    //tw_end();

//    ASSERT_NE(stat_lp->messages->size() , 256);
    //we went to time 30, so:
    /* neuron 0 should be -30
     * neuron 1 should be -60
     * neuron 2 should be 0 (35 but spikes would happen so 0)
     * neuron 3 should be 0 (65 but spikes would happen so 0)
     * neuron 4 */
    ASSERT_EQ(core->membrane_potential_v[0], -30);
    ASSERT_EQ(core->membrane_potential_v[1], -60);



}

TEST_F(CoreTest, CoreInit){
    set_init_ross(1);
    set_lp(0);
    tw_run();
    test_core->core_init(lp);
    ASSERT_EQ(test_core->core_local_id, 0);

}



TEST_F(CoreTest, BF_Heartbeat_Enum){
    BF_Event_Status hb_sent = BF_Event_Status::Heartbeat_Sent | BF_Event_Status::Spike_Sent;
    std::cout << "\n" << as_integer(hb_sent) << "\n" << as_integer(BF_Event_Status::Spike_Rec) << "\n";
    ASSERT_TRUE(is_hb_sent(hb_sent));
    ASSERT_TRUE(is_spike_sent(hb_sent));
    ASSERT_FALSE(is_spike_recv(hb_sent));

    BF_Event_Status bf2 = BF_Event_Status ::None;
    ASSERT_FALSE(is_hb_sent(bf2));
    ASSERT_FALSE(is_spike_sent(bf2));
    ASSERT_FALSE(is_spike_recv(bf2));

    BF_Event_Status bf3 = BF_Event_Status :: Heartbeat_Sent;
    ASSERT_TRUE(is_hb_sent(bf3));
    ASSERT_FALSE(is_spike_sent(bf3));
    ASSERT_FALSE(is_spike_recv(bf3));
    std::cout << "\n" << as_integer(bf3) << "\n" << as_integer(hb_sent) << "\n";


    bf2 = BF_Event_Status ::Spike_Rec;

    ASSERT_FALSE(is_hb_sent(bf2));
    ASSERT_FALSE(is_spike_sent(bf2));
    ASSERT_TRUE(is_spike_recv(bf2));

    bf2 |=  BF_Event_Status ::NS_Tick_Update;
    std::cout << "bf2: " << as_integer(bf2) << " NS UPDATE: "
                                               "" << as_integer(BF_Event_Status ::NS_Tick_Update) << "AMP: " << as_integer(bf2 & BF_Event_Status::NS_Tick_Update) << "\n";
    ASSERT_EQ(as_integer(BF_Event_Status::NS_Tick_Update),as_integer( (bf2 & BF_Event_Status::NS_Tick_Update)));
    //bf1 = as_integer(hb_sent);

}

