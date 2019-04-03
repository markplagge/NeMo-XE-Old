//
// Created by Mark Plagge on 2019-03-26.
//

#include "../include/nemo_input.h"


void TNJSWrapper::set_core(const TrueNorthCore& core){

    //iterate through the neurons in this core,
    //set the internal neuron state - neuron - using the model_reader lib
    //using the set_neuron function:
    auto max_core = this->model->get_max_core();
    if (core.core_local_id > max_core){
        return; // out of bounds.
    }
    for(int i=0; i < NEURONS_PER_TN_CORE; i ++) {
        set_neuron(core, i);
    }

}

void TNJSWrapper::set_neuron(const TrueNorthCore& core, int neuron_id) {
    auto core_id = core.core_local_id;
    //const auto neuron_hdlr = &this->neuron;
    //auto new_neuron = this->model->create_neuron_from_id(core_id,neuron_id);
    auto neuron_wrapper = this->model->generate_neurons_in_core_vec(core_id);
    for(auto neuron : neuron_wrapper){
        neuron.initialize_state(core);
    }
    //use the core's neuron setup function to initialize core values
    //core->create_tn_neuron

    }

    void core_wrapper(const TrueNorthCore& core, unique_ptr<tn_neuron_state> n){
    //core.create_tn_neuron(n->myLocalID,n->synapticConnectivity,)
}