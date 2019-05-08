//
// Created by Mark Plagge on 2019-03-26.


#include "../include/nemo_input.h"
#include "nemo_xe.h"

void TNJSWrapper::set_neuron_in_core(TrueNorthCore& core, unsigned int neuron_id){
    //create a neuron on the stack
    tn_neuron_state *neu = {};
    this->model->populate_neuron_from_id(core.core_local_id,neuron_id,neu);
    const  long dest_core[1] = {(long)neu->dest_core[0]};
    const  long dest_axon[1] = {(long)neu->dest_axon[0]};

    core.create_tn_neuron(neuron_id,neu->synapticConnectivity,neu->G_i,neu->sigma,neu->S,neu->weightSelection,neu->epsilon,
            neu->sigma_l,neu->lambda,neu->c, neu->posThreshold,neu->negThreshold,neu->TM,neu->VR,neu->sigmaVR,neu->gamma,
            neu->kappa,neu->delayVal,dest_core,dest_axon);
//    core->create_tn_neuron(neuron_id,neu->synapticConnectivity,neu->G_i,neu->sigma,neu->S,neu->weightSelection,
//            neu->epsilon,neu->sigma_l,neu->lambda,neu->c,neu->posThreshold,neu->negThreshold,neu->TM,neu->VR,neu->sigmaVR,
//            neu->gamma,neu->kappa,neu->delayVal,neu->dest_core,neu-

}
void TNJSWrapper::set_core(TrueNorthCore& core){

    //iterate through the neurons in this core,
    //set the internal neuron state - neuron - using the model_reader lib
    //using the set_neuron function:
    auto max_core = this->model->get_max_core();
    if (core.core_local_id > max_core){
        return; // out of bounds.
    }
    for(int i=0; i < NEURONS_PER_TN_CORE; i ++) {
        set_neuron_in_core(core, i);
    }

}


