//
// Created by Mark Plagge on 2019-03-16.
//


#include <gtest/gtest.h>
#include <gmock/gmock.h>

//#include "../src/include/globals.h"
#include "../src/include/CoreLP.h"
#include "TrueNorthCoreLogger.h"
#include "ross_testing_wrapper.h"
std::vector<core_types> core_type_map;
using namespace std;
std::vector<nemo_message*> message_trace_elements;
std::vector<std::string> message_trace_string;
int OUTPUT_MODE = 2;

int num_of_input_spikes = 3;
void lif_pre_run(CoreLP *s, tw_lp *lp){
    auto cclp = (CoreLP * ) lp->cur_state;
    cclp->getCore()->core_init(lp);
    //cclp->getCore()->pre_run(lp);
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
    auto core = (LIFCore * ) ((CoreLP *)lp->cur_state)->getCore();
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
        core->spike_output = (CoreOutput *) new CoreOutputThreadTester(SPIKE_OUTPUT_FILENAME);
    }
}
tw_lptype lif_lps[] = {
        {(init_f)lif_core_init_test,

                (pre_run_f) lif_pre_run,
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

class LIFCore_test: public ::testing::Test {


protected:
    void SetUp() override {

        message_trace_string.clear();
        message_trace_elements.clear();
        core_type_map.push_back(LIF);
        core_type_map.push_back(LIF);
        SPIKE_OUTPUT_FILENAME = (char*) calloc(128,sizeof(char));
        sprintf(SPIKE_OUTPUT_FILENAME, "test_case_output");
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
        tw_define_lps(2, sizeof(nemo_message));

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
    auto core = core_lp->getCore();
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
    for(auto spd : core_output->spikes){
        base_string.push_back(spd.to_csv());
    }
    ASSERT_EQ(input_string.size(),base_string.size());


    //
#undef filename


}