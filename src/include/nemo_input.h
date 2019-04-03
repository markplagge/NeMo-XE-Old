//
// Created by Mark Plagge on 2019-03-25.
//





#ifndef NEMO2_NEMO_INPUT_H
#define NEMO2_NEMO_INPUT_H

#include <vector>
#include <ostream>
#include <string>
#include "../neuro/TrueNorthCore.h"
#include <model_reader/src/tn_neuron_interface.h>
#include <model_reader/include/tn_parser.hh>
using namespace std;

/**
 * Main Wrapper for the TN Struct used in NeMo
 * Creates a new tn_neuron_state on construction
 * Will contain to data functionality as well.
 */
class TNJSWrapper {
private:
public:
    explicit TNJSWrapper(const string &filename) {
    //neuron = (neuron*) calloc(sizeof(tn_neuron_state), 1);
    //neuron = ;
    model = make_unique<TN_Main>(create_tn_data(filename));
}

private:
    tn_neuron_state *neuron;
    unique_ptr<TN_Main> model;

    void set_neuron(const TrueNorthCore& core, int neuron_id);
    void set_core(const TrueNorthCore& core);

public:

    void create_core_from_json(const TrueNorthCore& core);




//public:
//    bool isValid;
//    int myCore;
//    int write_bin_data(int big_end, ostream out);
//    string generate_csv();
//    json generate_json(json j);
//    string generate_pycode();
//
//
//
////  ~TN_State_Wrapper(){
////    delete(tn);
////  }
//    tn_neuron_state *getTn() {
//        return tn;
//    }
//    TN_State_Wrapper() {
//        tn = (tn_neuron_state *) calloc(sizeof(tn_neuron_state),1);
//        isValid = true;
//    }
//    void init_empty() {
//        //tn = (tn_neuron_state *) calloc(sizeof(tn_neuron_state), 1);
//        //this->tn = tn;
//        for (int i = 0; i < AXONS_IN_CORE; i++) {
//            tn->axonTypes[i] = -1;
//            tn->synapticConnectivity[i] = false;
//        }
//        for (int j = 0; j < NUM_NEURON_WEIGHTS; ++j) {
//            tn->synapticWeight[j] = -1;
//            tn->weightSelection[j] = false;
//
//        }
//    }
//
//    void initialize_state(vector<int> input_axon_connectivity,
//                          vector<short> input_axon_types,
//                          int output_core,
//                          int output_neuron,
//                          int source_core,
//                          int source_local,
//                          int dest_delay);
//
//    void initialize_state(int input_axon_connectivity[],
//                          short input_axon_types[],
//                          int output_core,
//                          int output_neuron,
//                          int source_core,
//                          int source_local,
//                          int dest_delay);
//
//    void initialize_state(tn_neuron_state *ext_n);
//    char *generate_lua();

};

#endif //NEMO2_NEMO_INPUT_H
