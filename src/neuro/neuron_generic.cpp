//
// Created by Mark Plagge on 2019-02-11.
//

#include "neuron_generic.h"

NeuronLIF::NeuronLIF(bool stochastic,unsigned int membranePotential, unsigned int numInputAxons, unsigned int threshold, int leakValue,
                     int *weights, int *axonTypes, unsigned int *inputConnectivity) : stochastic(stochastic),
                     membrane_potential(membranePotential), num_input_axons(numInputAxons), threshold(threshold), leak_value(leakValue),
                                                                                      axon_types(axonTypes),
                                                                                      input_connectivity(
                                                                                              inputConnectivity) {
    NeuronLIF::weights = (int *)malloc(sizeof(int) * numInputAxons);
    for(int i = 0; i < numInputAxons; i ++){
        int weight = inputConnectivity[i] * weights[axon_types[i]];
        NeuronLIF::weights[i] = weight;
    }
}

unsigned int NeuronLIF::getThreshold() const {
    return threshold;
}

