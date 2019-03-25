//
// Created by Mark Plagge on 2019-03-11.
//





#ifndef NEMO2_TRUENORTHCORE_H
#define NEMO2_TRUENORTHCORE_H



/**
 * TrueNorthCore - A neurosynaptic core containing TrueNorth neurons.
 * @addtogroup nemo_cores @{
 */
struct TrueNorthCore: public INeuroCoreBase {
    //TrueNorth Neuron State Vectors






    /**
     * If true, then one of the neurons in this core is self-firing.
     * When a core has a self firing neuron, it will send a heartbeat every
     * neurosynaptic tick.
     */
    bool has_self_firing_neuron=false;




    /** Firing status of the neurons.
     * For each neuron, when the fire_reset() function is called,
     * if a neuron fired, this value is set to true.
     * @todo: reset this when a new epoch (neurosynaptic tick) has occured.
     */
    bool fire_status[NEURONS_PER_TN_CORE];
    // core arrays

    /**
     * Weights for each TrueNorth neuron. Based on some quick math,
     * if we use primitive variable types (and don't do bit packing),
     * a single array that contains the computed weights for each neuron
     * with each of its synapses would consume less memory than individual
     * arrays for each TN paramater.
     *
     * On init, the weight sign, neuron type [4], and other weight parameters
     * from the True North neuron are computed, and stored in this array.
     */
    int weights[NEURONS_PER_TN_CORE][NEURONS_PER_TN_CORE];
    /**
     * Leak weights. \f$\lambda\f$
     */
    short lambdas[NEURONS_PER_TN_CORE];
    /**
     * \f$\gamma\f$. Represents the three types of reset modes.
     * Valid options are 0,1,2.
     * @todo: perhaps change this to a uchar, or even bitpack these small values
     * into a single struct, and use that inside of an array.
     */
    short reset_modes[NEURONS_PER_TN_CORE];
    /**
     * \f$\sigma_{VR}\f$ - Reset voltage sign bits.
     */
    short sigma_VRs[NEURONS_PER_TN_CORE];
    /**
     * Neuron reset voltage values: \f$R\f$.
     */
    short reset_voltages[NEURONS_PER_TN_CORE];
    /**
     * Contains the \f$\sigma_{l}\f$ values. These are the sign bits for leak values.
     */
    short sigma_ls[NEURONS_PER_TN_CORE];
    /**
     * TrueNorth neurons can have a delay when sending a spike. This defines that delay.
     */
    short delays[NEURONS_PER_TN_CORE];
    /**
     * epsilon function - leak reversal flag. from the paper this
     * changes the function of the leak from always directly being
     * integrated (false), or having the leak directly integrated
     * when membrane potential is above zero, and the sign is
     * reversed when the membrane potential is below zero.
     */
    short epsilons[NEURONS_PER_TN_CORE];
    /**
     * leak weight selection. If true, this is a stochastic leak
    function and the \a leakRateProb value is a probability, otherwise
    it is a leak rate. Is \f$c\f$ in the paper.
     */
    short c_vals[NEURONS_PER_TN_CORE];
    /**
     * Kappa or negative reset mode. From the paper's ,\f$\kappa_j\f$, negative threshold setting to reset or saturate.
     */
    short kappa_vals[NEURONS_PER_TN_CORE];
    /**
     * Chooses a stochastic or regular weight mode. \f$b_j^{G_i}\f$
     */
    short stochastic_weight_mode[NEURONS_PER_TN_CORE][NEURONS_PER_TN_CORE];

    nemo_id_type destination_cores[NEURONS_PER_TN_CORE][MAX_OUTPUT_PER_TN_NEURON];
    nemo_id_type destination_axons[NEURONS_PER_TN_CORE][MAX_OUTPUT_PER_TN_NEURON];

    nemo_volt_type  membrane_potentials[NEURONS_PER_TN_CORE];

    nemo_thresh_type positive_threshold[NEURONS_PER_TN_CORE];
    nemo_thresh_type negative_threshold[NEURONS_PER_TN_CORE];


    /**
     * Maximum PRN value for Leak
     */
    nemo_random_type random_range_leak[NEURONS_PER_TN_CORE];
    /**
     * Maximum PRN value for Reset
     */
    nemo_random_type random_range_rst[NEURONS_PER_TN_CORE];


    explicit TrueNorthCore(int coreLocalId, int outputMode);

    bool is_output_neuron(int neuron_id);
    bool is_self_firing_neuron(int neuron_id);

    /**
     * Creates a blank TrueNorth core. Sets this core's local ID to the lpid.
     * @todo: add a mapping function so non-linear mapping is valid.
     * @param lp
     */
    void core_init(tw_lp *lp) override;
    /**
     * TrueNorthCore pre-run.
     * @param lp
     */
    void pre_run(tw_lp *lp) override;

    /**
     * Primary forward event handler for the TN Neuron.
     * @param bf
     * @param m
     * @param lp
     */
    void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;
    /**
     * Reverse event handler for the TN Neuron. @bug not implemented
     * @param bf
     * @param m
     * @param lp
     */
    void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

    void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

    void core_finish(tw_lp *lp) override;


    //construction of TN

    /**
     * Configures a TrueNorth neuron in this core. This function sets up the values required for a specific
     * neuron, passed via neuron_id.
     * @param neuron_id The neuron to configure
     * @param synaptic_connectivity This neuron's synaptic connectivity - must be an array
     * @param G_i The type of the ith axon
     * @param sigma the sign of the ith axon weight
     * @param S synaptic weight
     * @param b selects between stochastic and deterministic integration
     * @param epsilon Selects between monotonic and divergent / convergent leak
     * @param sigma_l leak sign
     * @param lambda The leak
     * @param c
     * @param alpha
     * @param beta
     * @param TM The (encoded) threshold pseudo-random number mask; expands to \f$(2^{TM} - 1)\f$
     * @param VR The (encoded) reset potential VR; expands to \f$ σVR(2^{VR} - 1)\f$
     * @param sigmaVR The reset sign
     * @param gamma
     * @param kappa
     * @param signal_delay
     * @param dest_core
     * @param dest_axon
     */
    void create_tn_neuron(nemo_id_type neuron_id, bool *synaptic_connectivity, short *G_i, short *sigma, short *S,
                          bool *b, bool epsilon, short sigma_l, short lambda, bool c, int alpha, int beta,
                          short TM, short VR, short sigmaVR, short gamma, bool kappa, int signal_delay,
                          const nemo_id_type dest_core[], const nemo_id_type dest_axon[]);
    /**
     * TN Neurons compute a PRN value based on a random number and a bit mask. This function recreates this behavior.
     * @param p_mask
     * @return the maximum random value
     */
    unsigned int gen_encoded_random(unsigned int p_mask);

    /**
     * Fire and Reset functionality. This checks to see if the neuron needs to fire,
     * and if so sets the binary table fire_status[neuron_id] to true and resets the neuron.
     * ηj = ρjTM&Mj
     * if (γj = 2) and Vj(t) > Mj
     * • spike, Vj(t) = Mj
     * else if Vj(t) > αj + ηj
     * • spike, Vj(t) = δ( γj)Vrstj + δ (γj -1)( Vj(t) – αj) + d(γj – 2) Vj(t)
     * else if Vj(t) < -[( βj κj + (βj + ηj)(1 - κj)]
     * • Vj(t) = - βj κj + (1 – κj)[ δ(γj)Vrstj + δ( γj – 1)( Vj(t) – βj) + δ (γj – 2) Vj(t)]
     * @param lp
     */
    void fire_reset(tw_lp *lp);
    /**
     * Ringing check - eliminates "ringing" behavior in a neuron.
     * Paper Ref:
     * Vj(t) = 0 if (εj = 1) and [sgn(Vj*(t)) ≠ sgn(Vj(t))]
     * Vj(t) = Vj*(t) otherwise
     * @param neuron_id
     */
    void ringing();
    /**
     * called when a spike is received, this integrates across neurons in the core.
     * Will be updated with efficient OpenMP / Vector versions as well as a GPU version.
     * EQ:
     * \f$ V_j(t) = V_j(t-1) + \sum_{i=0}^{255} A_i(t)w_{i,j}\sigma_j^{G_i} [(1-b_j^{G_i}) s_j^{G_i} + b_j^{G_i} F(S_j^{G_i}, P_j^{G_i})] \f$
     * @param lp
     * @param input_axon
     */
    void integrate(tw_lp *lp, nemo_id_type input_axon);
    //void post_integrate(tw_lp *lp, bool did_fire);
    /**
     * Leak functionality. Will compute leak values for the amount of time that has
     * passed since the last time this neurosynaptic core was active.
     * Paper ref:
     * Ω = σjλ(1-εj) + sgn(Vj(t))σjλεj
     * Vj*(t) = Vj(t) + Ω[(1 - cjλ)λj + cjλF(λj,ρjλ)]
     */
    void leak(tw_lp *lp, tw_bf *bf);
    void send_heartbeat(tw_lp *lp, tw_bf *bf);
    void send_heartbeat(tw_lp *lp);

private:
    void compute_weight(nemo_id_type neuron_id, int *synaptic_connectivity, int *G_i, int*sigma, int *b, int *S);



};
/** @} */
#include "INeuroCoreBase.h"

#endif //NEMO2_TRUENORTHCORE_H
