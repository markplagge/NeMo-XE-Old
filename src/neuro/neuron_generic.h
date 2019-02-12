//
// Created by Mark Plagge on 2019-02-11.
//
/** neuron_generic.h - defines the basic structure for neurons */






#ifndef NEMO2_NEURON_GENERIC_H
#define NEMO2_NEURON_GENERIC_H
class INeuron {
public:

    virtual bool forward_event() = 0;
    virtual bool reverse_event() = 0;


};

class IntegrateFireNeuron : INeuron{
public:
    IntegrateFireNeuron();

};
#endif //NEMO2_NEURON_GENERIC_H


class IVecNeuron {
    virtual void integrate()  = 0;
};
/////
///
/*
 * vector:
 *   Connectivity|Weight|neuron_function
 *  [0][INeuron::weight][INeuron::integrate]
 * [
 *
 */