//
// Created by Mark Plagge on 2019-03-25.
//





#ifndef NEMO2_NEMO_INPUT_H
#define NEMO2_NEMO_INPUT_H

#include <vector>
#include <ostream>
#include <string>
#include <fstream>
#include <strstream>
#include <istream>
#include <ostream>

#include "nemo_xe.h"
#include "../neuro/TrueNorthCore.h"
#include "../../external/model_reader/include/tn_parser.hh"
#include "../../external/model_reader/src/tn_neuron_interface.h"



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

/**
 * Given a neurosynaptic TrueNorth core (this class is specific to the standard TrueNorth configuration),
 * initialize the core using the EEDN/NSCS JSON data parsed by the TN_Main et.al. libs.
 * @param core
 */
    void set_core(TrueNorthCore &core);

private:
    /**
     * Sets a single neuron in the passed in core.
     * @param core
     * @param neuron
     * @param neuron_id
     */
    void set_neuron_in_core(TrueNorthCore &core, unsigned int neuron_id);

    unique_ptr<TN_Main> model;
};



class NeMoTypeMap{
    ifstream typemap_file;
    vector<char> neuron_type;
public:
    NeMoTypeMap(const string &filename){
        typemap_file.open(filename);
        neuron_type.reserve(nemo_config.ne_num_cores_in_sim);
    }
    ~NeMoTypeMap(){
        typemap_file.close();
    }
};


#endif //NEMO2_NEMO_INPUT_H