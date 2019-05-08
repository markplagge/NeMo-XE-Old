//
// Created by Mark Plagge on 2019-03-16.
//


#include <gtest/gtest.h>
#include <gmock/gmock.h>

//#include "../src/include/globals.h"
#include "../src/include/CoreLP.h"
#include "TrueNorthCoreLogger.h"
#include "ross_testing_wrapper.h"
#include "../src/include/NeMoConfig.h"
#include "nemo_xe.h"

std::vector<core_types> core_type_map;
using namespace std;
std::vector<nemo_message*> message_trace_elements;
std::vector<std::string> message_trace_string;
int OUTPUT_MODE = 2;
void hb_commit_fun(CoreLP *lpd, tw_bf *bf, nemo_message *m, tw_lp *lp);
int num_of_input_spikes = 3;
NeMoConfig &nemo_config = NeMoConfig::get_config();
void lif_pre_run(CoreLP *s, tw_lp *lp){
    auto cclp = (CoreLP * ) lp->cur_state;
    cclp->get_core()->core_init(lp);

    //cclp->get_core()->pre_run(lp);
    for(int i =0; i < num_of_input_spikes; i ++) {
        tw_event *evt = tw_event_new(lp->gid,JITTER(lp->rng),lp);
        nemo_message *msg = (nemo_message *) tw_event_data(evt);
        msg->message_type = NEURON_SPIKE;
        msg->source_core = 0;
        msg->dest_axon = 1;
        msg->intended_neuro_tick = 0;
        tw_event_send(evt);
    }




}

int number_of_active_neurons = 32;
bool output_test = false;
class CoreOutputThreadTester :  public CoreOutputThread {
public:
    explicit CoreOutputThreadTester(const std::string &outputFilename) : CoreOutputThread(outputFilename){};
    ~CoreOutputThreadTester() override {};

    void save_spike(SpikeData spike) override {
        spikes.push_back(spike);
        CoreOutputThread:save_spike(spike);
    }
    vector<SpikeData> spikes;

};




void lif_core_init_test(CoreLP *core_lp, tw_lp *lp){
    CoreLP::core_init(core_lp, lp);
    auto core = (LIFCore * ) ((CoreLP *) lp->cur_state)->get_core();
    core->core_init(lp);
    for(int i = 0; i < LIF_NEURONS_PER_CORE; i ++){
        array<int,LIF_NEURONS_PER_CORE> wts;
        array<int, LIF_NUM_OUTPUTS> dest_cores;
        array<int, LIF_NUM_OUTPUTS> dest_axons;
        int leak_value = -1;
        int threshold = 5;

        for(int j = 0; j < LIF_NEURONS_PER_CORE; j ++){
            if(i < number_of_active_neurons) {
                wts[j] = 1;
            }else{
                wts[j] = 0;
            }
        }
        for(int j = 0; j < LIF_NUM_OUTPUTS; j ++){
            dest_cores[j] = 1;
            dest_axons[j] = i;
        }
        core->create_lif_neuron(i,wts,dest_cores,dest_axons,leak_value,threshold);

    }
    if (output_test){
        delete(core->spike_output);
        core->spike_output = (CoreOutput *) new CoreOutputThreadTester(nemo_config.ne_spike_output_filename);
    }
}
tw_lptype lif_lps[] = {
        {(init_f)lif_core_init_test,

                (pre_run_f) lif_pre_run,
                (event_f) CoreLP::forward_event,
                (revent_f) CoreLP::reverse_event,
                (commit_f) hb_commit_fun,
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
st_model_types lif_test_trace[] =  {
        {(ev_trace_f) DummyLP::core_ev_trace,
                sizeof(nemo_message),
                NULL,
                NULL,
                NULL,
                NULL,
                NULL},
        {0},


};
/* Heartbeat testing helpers */
std::vector<int> heartbeat_schedule;
/* Heartbeat testing data helpers (since gtest won't be able to hook directly into this */
struct test_hb_data {

    double now;
    long prev_neuro_tick;
    long cur_neruo_tick;
    long prev_leak_time;
    long last_active_time;
    std::string desc;
};
std::vector<test_hb_data> heartbeat_data;


void hb_post_lp_init(tw_pe *pe){
    tw_lp * test_core_lp = tw_getlp(0);
    std::sort(heartbeat_schedule.begin(), heartbeat_schedule.end());
    std::reverse(heartbeat_schedule.begin(), heartbeat_schedule.end());

    auto sched_time = heartbeat_schedule.back();
    heartbeat_schedule.pop_back();

            if(sched_time < 0){
            sched_time = 1;
        }
    double send_time = sched_time - 1;
    send_time += JITTER(test_core_lp->rng);
    tw_event * e = tw_event_new(0,send_time, test_core_lp);
    nemo_message *message = (nemo_message *) tw_event_data(e);
    message->intended_neuro_tick = (int) sched_time;
    message->source_core = -1;
    message->dest_axon = 1;
    message->debug_time = send_time;
    std::cout << "Msg send time:  " << send_time << "\n";
    tw_event_send(e);


}

void send_hb_test_events(tw_lp *lp){
    tw_lp * test_core_lp = lp; //tw_getlocal_lp(0); // zero is the test core for our tests.
    //we want to set the weights of the core's neurons to 0 so that we can ignore spikes and generate heartbeats only
    std::sort(heartbeat_schedule.begin(), heartbeat_schedule.end());
    std::reverse(heartbeat_schedule.begin(), heartbeat_schedule.end());
    for(auto sched_time : heartbeat_schedule){
        if(sched_time != 1){
            if(sched_time < 0){
                sched_time = 1;
            }

            double send_time = sched_time - 1;
            send_time += JITTER(test_core_lp->rng);
            tw_event * e = tw_event_new(0,send_time, test_core_lp);
            auto *message = (nemo_message *) tw_event_data(e);
            message->intended_neuro_tick = (int) sched_time;
            message->source_core = -1;
            message->dest_axon = 1;
            message->debug_time = send_time;
            std::cout << "Msg send time:  " << send_time << "\n";

            tw_event_send(e);
        }
    }
    for (int i = 0; i < heartbeat_schedule.size(); i ++){
        heartbeat_schedule.pop_back();
    }
}
void hb_commit_fun(CoreLP *lpd, tw_bf *bf, nemo_message *m, tw_lp *lp){
    if (lp->gid != 0){
        return;
    }
    auto core = (CoreLP*) lp->cur_state;

    auto core_in = core->get_core();
    test_hb_data data{tw_now(lp),core_in->previous_neuro_tick, core_in->current_neuro_tick,
                      core_in->last_leak_time,core_in->last_active_time, std::string{"HB Tick - PreFWD"}};
    heartbeat_data.push_back(data);
    CoreLP::core_commit(lpd,bf,m,lp);
    test_hb_data data2{tw_now(lp),core_in->previous_neuro_tick, core_in->current_neuro_tick,
                      core_in->last_leak_time,core_in->last_active_time, std::string{"HB Tick - PostFWD"}};
    heartbeat_data.push_back(data2);
    // generate the spikes for HB testing:
    send_hb_test_events(lp);
}
tw_petype heartbeat_lpe_tests{
        //tw_petype main_pe={
       0,
        (pe_init_f) hb_post_lp_init,
        0,
        0
};

class LIFCore_test: public ::testing::Test {


protected:
    void SetUp() override {

        message_trace_string.clear();
        message_trace_elements.clear();
        core_type_map.push_back(LIF);
        core_type_map.push_back(LIF);
        //nemo_config.ne_spike_output_filename = (char*) calloc(128,sizeof(char));
        sprintf(nemo_config.ne_spike_output_filename, "test_case_output");
    }
    void set_init_ross(int nlp){
        g_tw_nlp = nlp;
        g_tw_synchronization_protocol = tw_synch_e :: CONSERVATIVE;
        int *argc = (int *)calloc(sizeof(int),1);
        char **argv = (char **) calloc(sizeof(char*), 3);
        argc[0] = 1;
        argv[0] = (char * ) calloc(sizeof(char), 1);
        //argv[1] = (char * ) calloc(sizeof(char), 64);
        argv[1] = "\0";
        g_tw_events_per_pe = 256 * 256;
        tw_init(argc, &argv);
        tw_define_lps(nlp, sizeof(nemo_message));

        int i;
        for(i =0; i < g_tw_nlp - 1; i ++){
            tw_lp_settype(i, &lif_lps[0]);
            st_model_settype(i, &lif_test_trace[0]);
        }
        tw_lp_settype(i, &lif_lps[1]);

    }





};

TEST_F(LIFCore_test, CoreInit){
    OUTPUT_MODE=0;
    g_tw_ts_end = 30;
    num_of_input_spikes = 10;
    set_init_ross(2);
    tw_run();
}
template <typename T>
std::string read_csv(T filename){
    std::ifstream file;
    file.open(filename);
    std::string data((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    return data;
}
std::string generate_csv(int source_core, int start_neuro, int end_neuro, int dest_offset, int tick) {
    std::ostringstream sgen;
    sgen << "source_core,source_neuron,dest_core,dest_axon,source_neuro_tick,dest_neuro_tick,tw_source_time\n";
    int dest_axon = 0;
    int dest_core = 1;
    for(int i = start_neuro; i < end_neuro; i ++){
        char fmt[1024];
        sprintf(fmt,"%d,%d,%d,%d,%d,%d,%f\n", source_core,i, dest_core,dest_axon,tick,tick + 1,tick + 1 + .00002);

        sgen << fmt;
        dest_axon += dest_offset;
    }
    return sgen.str();

}

/* Heartbeat Timing Tests */

TEST_F(LIFCore_test, LIF_Heartbeat_Timing){
    //we need to test heartbeat mechanics
    //test a schedule of spikes at time 1, 2, 3, 4, 5, 10, 20.
    heartbeat_schedule = vector<int>{1,10,20};
    OUTPUT_MODE=2;
    number_of_active_neurons=4;
    g_tw_ts_end = 60;
    num_of_input_spikes = 0;

    set_init_ross(2);

    auto pe = tw_getpe(0);
    pe->type.post_lp_init = (pe_init_f) hb_post_lp_init;
    auto lp = tw_getlp(0);

    lp->type->commit = (commit_f) hb_commit_fun;
    tw_run();
    std::ofstream file;
    file.open("hb_test_data.csv");
    file << "now,prev_neuro_tick,long cur_neruo_tick,long prev_leak_time,last_active_time,desc\n";
    for (auto hb_data : heartbeat_data){
        file << hb_data.now << "," << hb_data.prev_neuro_tick << ","<< hb_data.cur_neruo_tick << "," << hb_data.prev_leak_time << "," << hb_data.last_active_time << "," << hb_data.desc << "\n";
    }

    //we generated some heartbeats and spikes.
}
TEST_F(LIFCore_test, CoreOutput){
    OUTPUT_MODE=2;
    output_test = true;
    number_of_active_neurons = 32;

    g_tw_ts_end = 30;
    num_of_input_spikes = 10;
    set_init_ross(2);
    tw_run();

    // FILENAME OUTPUT:
#define filename  "test_case_output_0.csv"

    CoreLP * core_lp = (CoreLP *)tw_getlp(0)->cur_state;
    auto core = core_lp->get_core();
    auto core_output = (CoreOutputThreadTester *) core->spike_output;
    core_output->shutdown_thread();

    core_output->close_posix_file();

    std::string file_data = read_csv(filename);


    vector<std::string> input_string;
    vector<std::string> base_string;
    std::string lines;
    std::istringstream core_dump(file_data);
    std::getline(core_dump, lines);
    while(std::getline(core_dump,lines)){
        input_string.push_back(lines);
    }
    auto logger_output = ((DummyLP * )tw_getlp(1) -> cur_state);

    for(auto spd : *logger_output->messages){
        base_string.push_back(spd->to_string());
    }
    ASSERT_GT(input_string.size(), 0);
    ASSERT_EQ(input_string.size(),base_string.size());


    //
#undef filename


}