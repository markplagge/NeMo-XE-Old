//
// Created by Mark Plagge on 2019-03-09.
//

#include <gtest/gtest.h>
#include "../src/include/CoreOutput.h"
#include <random>

class SpikeOutputTest : public ::testing::Test{
    void SetUp() override {
        header = "source_core,source_neuron,dest_core,dest_axon,source_neuro_tick,dest_neuro_tick,tw_source_time\n";
        d1 = new SpikeData;
        d1->tw_source_time = 3.14;
        d1->source_core = 0;
        d1->source_neuron = 1;
        d1->dest_core = 2;
        d1->dest_axon = 3;
        d1->dest_neuro_tick = 4;
        d1->source_neuro_tick = 3;
        std::string d1_dat = "0,1,2,3,3,4,3.14\n";
        d1_expected = header + d1_dat;

        d2 = new SpikeData;

        d2->source_core = 32;
        d2->source_neuron = 33;
        d2->dest_core = 34;
        d2->dest_axon=35;
        d2->source_neuro_tick = 54;
        d2->dest_neuro_tick = 42;
        d2->tw_source_time = 4.14;
        std::string d2_dat = "32,33,34,35,54,42,4.14\n";
        d2_expected = header + d2_dat;

        dual_expected = header + d1_dat + d2_dat;




        int *argc = (int *) calloc(sizeof(int), 1);
        char **argv = (char **) calloc(sizeof(char *), 3);
        argc[0] = 1;
        argv[0] = (char *) calloc(sizeof(char), 10);
        sprintf(argv[0], "test");
        MPI_Init(argc, &argv);

    }
public:
    SpikeData *d1;
    std::string d1_expected;
    std::string header;
    std::string d2_expected;
    std::string dual_expected;
    SpikeData *d2;
    SpikeData *generate_random_spike(){
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(0,65535);
        std::uniform_int_distribution<int> distribution1(-1024,1024);
        SpikeData *spk = new SpikeData;
        spk->source_core = distribution(generator);
        spk->source_neuron = distribution(generator);
        spk->dest_core = distribution1(generator);
        spk->dest_axon = distribution1(generator);
        spk->source_neuro_tick = distribution(generator);
        spk->dest_neuro_tick = distribution(generator);
        spk->tw_source_time = drand48() * 10;
        return spk;
    }
    std::vector<std::string>spike_strs;
    bool find_spike(std::string spike_rep){
        return std::find(spike_strs.begin(), spike_strs.end(), spike_rep) != spike_strs.end();
    }

};
std::string read_the_csv(std::string filename){
    std::ifstream file;
    file.open(filename);
    std::string data((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    return data;
}
TEST_F(SpikeOutputTest, internal_spike_rep){
    std::vector<std::string> base;
    std::vector<SpikeData *> rnd_spikes;
    for(int i = 0; i < 128; i ++){
        SpikeData *d = generate_random_spike();
        spike_strs.push_back(d->to_csv());
        base.push_back(d->to_csv());
        rnd_spikes.push_back(d);
    }
    for(auto s : base){
        ASSERT_TRUE(find_spike(s));
    }
    int ctr = 0;
    for(auto s : rnd_spikes){
        if(not find_spike(s->to_csv())){
            ASSERT_FALSE(true);
        }
        ctr ++;
    }
}

TEST_F(SpikeOutputTest, basic_single_save){

    {
        CoreOutput output_tester("basic_single_save");

        output_tester.save_spike(*d1);
    }
    std::string data = read_the_csv("basic_single_save_0.csv");

    //ASSERT_EQ(data, d1_expected);
    ASSERT_STREQ(data.c_str(), d1_expected.c_str());
}
TEST_F(SpikeOutputTest, basic_dual_save){
    {
        CoreOutput output_tester("basic_dual_save");

        output_tester.save_spike(*d1);

        output_tester.save_spike(*d2);
    }
    std::string data = read_the_csv("basic_dual_save_0.csv");

    ASSERT_EQ(data,dual_expected);

}
SpikeData copySpike(SpikeData *a){
    SpikeData b;

    b.tw_source_time = a->tw_source_time;
    b.dest_core = a->dest_core;
    b.source_neuro_tick = a->source_neuro_tick;
    b.dest_neuro_tick = a->dest_neuro_tick;
    b.dest_axon = a->dest_axon;
    b.source_core = a->source_core;
    b.source_neuron  = a->source_neuron;
    b.tw_source_time = a->tw_source_time;
    return b;
}
std::unique_ptr<SpikeData> generate_spike_ptr(SpikeData *a){
    std::unique_ptr<SpikeData> b = std::make_unique<SpikeData>();
    b->dest_core = a->dest_core;
    b->tw_source_time = a->tw_source_time;
    b->source_neuro_tick = a->source_neuro_tick;
    b->dest_neuro_tick = a->dest_neuro_tick;
    b->dest_axon = a->dest_axon;
    b->source_core = a->source_core;
    b->source_neuron  = a->source_neuron;
    b->tw_source_time = a->tw_source_time;
    return b;
}
//
//bool match(SpikeData *a, SpikeData b){
//    return strcmp(a->to_csv().c_str(), b.to_csv().c_str()) == 0;
//}
TEST_F(SpikeOutputTest, complex_thread_save){
    int num_of_spikes = -1;
    const int spike_inc = 32;
    const int num_test_runs = 40;
#pragma parallel for
    for(int test_iteration = 1; test_iteration < num_test_runs; test_iteration ++) {
        num_of_spikes = spike_inc * test_iteration;
        std::vector<SpikeData *> spikes;
        for (int i = 0; i < num_of_spikes; i++) {
            SpikeData *d = generate_random_spike();
            spikes.push_back(d);
            spike_strs.push_back(d->to_csv());

        }
        {
            CoreOutputThread output_tester("complex_thread_save");
            CoreOutput output_base("complex_thread_base");
            for (int i = 0; i < spikes.size(); i++) {
                SpikeData th_sp = copySpike(spikes[i]);
                output_tester.save_spike(th_sp);
                output_base.save_spike(*spikes[i]);
            }

        }
        std::cout << "Completed save of random spikes.\n";
        auto saved_data = read_the_csv("complex_thread_save_0.csv");


        auto base_saved_data = read_the_csv("complex_thread_base_0.csv");

        std::istringstream thread_sd(saved_data);
        std::string base_lines;
        std::string thread_lines;

        EXPECT_EQ(saved_data, base_saved_data);
        std::vector<std::string> base_vec;
        std::vector<std::string> thread_vec;
        {
            std::istringstream base_sd(base_saved_data);
            while (std::getline(base_sd, base_lines)) {
                base_vec.push_back(base_lines);
            }
            while (std::getline(thread_sd, thread_lines)) {
                thread_vec.push_back(thread_lines);
            }
            ASSERT_EQ(base_vec, thread_vec);
        }

    }


}

