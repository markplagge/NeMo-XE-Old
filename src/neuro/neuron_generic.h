//
// Created by Mark Plagge on 2019-02-11.
//
/** neuron_generic.h - defines the basic structure for neurons */






#ifndef NEMO2_NEURON_GENERIC_H
#define NEMO2_NEURON_GENERIC_H
#include <ross.h>
#include "../include/globals.h"
#include <iostream>


template <typename NeuronImpl>
/**
 * NeuronGeneric class. This class exists to define expected functions that every neruon in NeMo must implement.
 * In this case, the following functions are needed:
 * should_fire(): returns true if the neuron should send a spike, false otherwise.
 * integrate(): Forward-event integration function.
 * reverse_integrate(): Reverse integration. Reverse computation for integration must be handled here
 * leak(): neuron leak function
 * reverse_leak(): handles reverse leak function
 * get_weight_table(): In order to compute membrane potentials, weights must be contained in a vector or array of size
 * $n$, where $n$ is the number of input axons to this neuron. Generally, $n$ must be the same across all neurons in a
 * neurosynaptic core
 *
 *
 * Details: Functions here must be implemented by a neuron in a class - this is a static interface for these neurons,
 * which allow the Core class to communicate to them.
 *
 * Parameters to the functions need to be simple types for GPU acceleration. Integrate and leak functions, if a
 * stochastic function is used, must use a ROSS generated random number, NOT a call to the ROSS random number
 * generator.
 *
 * @tparam NeuronImpl
 */
class NeuronGeneric{
public:
    bool should_fire(){
        return neuron_implementation().should_fire_implementation();
    }
    template<typename RND, typename AXID, typename EP>
    void integrate(RND rand_val, AXID axon_id, EP extended_data ){
        neuron_implementation().integrate_implementation(rand_val, axon_id, extended_data);
    };


    template<typename RND, typename AXID, typename EP>
    void reverse_integrate(RND rand_val, AXID axon_id, EP extended_data){
        neuron_implementation().reverse_integrate_implementation(rand_val,axon_id, extended_data);
    }
//    void leak(int rand_val, int args){
//        neuron_implementation().leak_implementation(rand_val, args);
//    }
//    void reverse_leak(int rand_val, int args){
//        neuron_implementation().reverse_leak_implementation(rand_val, args);
//    }
    //some getters that we expect:
    auto get_weight_table(){
        return neuron_implementation().get_weight_table_imp;
    }


private:
    NeuronImpl& neuron_implementation() {
        return *static_cast<NeuronImpl*>(this);
    }
};




/**
 * A basic LIF neuron example - used as the base class for neurons. More of a TN LIF than a true LIF.
 * This class should be the basis for all neuron types in NeMo
 */
class NeuronLIF:public NeuronGeneric<NeuronLIF>{
//! \todo: Need to make these data members protected...
protected:
    unsigned int membrane_potential;
public:
    unsigned int getThreshold() const;

protected:
    unsigned int num_input_axons;
    unsigned int threshold;
    int leak_value;
    int *weights;
    int *axon_types;
    unsigned int *input_connectivity;
    bool stochastic;

public:
    int *getWeights() const {
        return weights;
    }

    NeuronLIF(bool stochastic,unsigned int membranePotential, unsigned int numInputAxons, unsigned int threshold, int leakValue,
              int *weights, int *axonTypes, unsigned int *inputConnectivity);

    auto get_weight_table_imp(){
        return this->getWeights();
    }

public:

    bool should_fire_implementation(){
        return this->membrane_potential >= threshold;
    }
    template<typename RND, typename AXID, typename EP>
    void integrate_implementation(RND rand_val, AXID axon_id, EP extended_data){
        this->membrane_potential = this->membrane_potential + (weights[axon_id]);

    }
    template<typename RND, typename AXID, typename EP>
    void reverse_integrate_implementation(RND rand_val, AXID axon_id, EP extended_data){
        this->membrane_potential = this->membrane_potential - (weights[axon_id]);
    }


};


class NeuronTest{

public:
    template<typename RND, typename AXID, typename EP>
    int integrate_mtx(RND rand_val, AXID axon_id, EP extended_data){
        return this->membrane_potential + (weights[axon_id]);

    }
    int *weights;
    int membrane_potential;
};
#endif