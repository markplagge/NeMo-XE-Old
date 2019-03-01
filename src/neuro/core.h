//
// Created by Mark Plagge on 2019-02-11.
//

#ifndef NEMO2_CORE_H
#define NEMO2_CORE_H
#include <ross.h>
#include "../include/globals.h"
#include "neuron_generic.h"
#include <vector>


/**
 * @defgroup tn_const TrueNorth Neuron Limitations
 * Contains TrueNorth network constants
 * @{
 */
constexpr int NEURONS_PER_TN_CORE = 256;
constexpr int WEIGHTS_PER_TN_NEURON = 4;
constexpr int MAX_OUTPUT_PER_TN_NEURON = 1;
/** @} */

/**
 * @defgroup nemo_cores NeMo2 Core Definitions
 * NeMo2 neurosynaptic cores, as well as the ROSS LP wrapper.
 * NeMo2 uses a "fat lp" technique. Neurons are defined in a core.
 * To wrap the LP state from ROSS, we use the CoreLP class, which
 * holds a INeuroCoreBase object.
 * @{
 */
/**
 * INeuroCoreBase
 * Base interface for neuromorphic cores in NeMo. Defines the expected functions
 * that will be used by neuromorphic cores.
 */
struct INeuroCoreBase {

    virtual void core_init(tw_lp *lp) = 0;
    virtual void pre_run(tw_lp *lp) = 0;
    virtual void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) = 0;
    virtual void core_finish(tw_lp *lp) = 0;



};

//////////////////////
/**
 * CoreLP - This class is an adapter for ROSS to the C++ object system in NeMo2.
 * Each LP in ROSS is of type CoreLP - and each CoreLP contains one reference to a
 * neuromorphic core class.
 *
 * Hopefully this abstraction will not hurt performance.
 * This class breaks some OO rules, as it will need to talk to the mapping and configuation
 * parsing systems directly - can't pass it anything but what is expected from ROSS.
 *
 *
 */
class CoreLP{
public:
    static void event_trace(nemo_message *m, tw_lp *lp, char *buffer, int *collect_flag);
    static void core_init(CoreLP *s, tw_lp *lp) {
        //determine the type of core we want through mapping
        s->create_core(lp);
    }

    static void pre_run(CoreLP *s, tw_lp *lp) {s->core->pre_run(lp);}

    static void forward_event(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {s->core->forward_event(bf,m,lp);}

    static void reverse_event(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {s->core->reverse_event(bf,m,lp); }

    static void core_commit(CoreLP *s, tw_bf *bf, nemo_message *m, tw_lp *lp) {s->core->core_commit(bf,m,lp);}

    static void core_finish(CoreLP *s, tw_lp *lp) {s->core->core_finish(lp);}

    void create_core(tw_lp *lp);

    INeuroCoreBase *getCore() const {
        return core;
    }
private:
    INeuroCoreBase  *core;
    int active;





};

/**
 * TrueNorthCore - A neurosynaptic core containing TrueNorth neurons.
 */
struct TrueNorthCore: public INeuroCoreBase {
    //TrueNorth Neuron State Vectors

    /**
     * The last time that this core had activity. This refers to any  message.
     */
    tw_stime last_active_time = 0 ;
    /**
     * The last time this core computed the leak.
     */
    tw_stime last_leak_time= 0 ;
    unsigned int current_neuro_tick = 0;
    /**
     * the local core id. If linear mapping is enabled, then this will be equal to the GID/PE id
     */
    int core_local_id;
    /**
     * A heartbeat check value.
     */
    bool heartbeat_sent = false;
    /**
     * If true, then one of the neurons in this core is self-firing.
     * When a core has a self firing neuron, it will send a heartbeat every
     * neurosynaptic tick.
     */
    bool has_self_firing_neuron=false;
    /**
     * evt_stat holds the event status for the current event. This is used to compute
     * reverse computation. BF_Event_Stats is used instead of the tw_bf as it allows
     * more explicit naming. The concept is the same, however.
     */
    BF_Event_Status evt_stat;

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
    bool epsilons[NEURONS_PER_TN_CORE];
    /**
     * leak weight selection. If true, this is a stochastic leak
    function and the \a leakRateProb value is a probability, otherwise
    it is a leak rate. Is \f$c\f$ in the paper.
     */
    bool c_vals[NEURONS_PER_TN_CORE];
    /**
     * Kappa or negative reset mode. From the paper's ,\f$\kappa_j\f$, negative threshold setting to reset or saturate.
     */
    bool kappa_vals[NEURONS_PER_TN_CORE];
    /**
     * Chooses a stochastic or regular weight mode. \f$b_j^{G_i}\f$
     */
    bool stochastic_weight_mode[NEURONS_PER_TN_CORE][NEURONS_PER_TN_CORE];

    nemo_id_type destination_cores[NEURONS_PER_TN_CORE][MAX_OUTPUT_PER_TN_NEURON];
    nemo_id_type destination_axons[NEURONS_PER_TN_CORE][MAX_OUTPUT_PER_TN_NEURON];
    nemo_volt_type  membrane_potentials[NEURONS_PER_TN_CORE];
    nemo_thresh_type positive_threshold[NEURONS_PER_TN_CORE];
    nemo_thresh_type negative_threshold[NEURONS_PER_TN_CORE];
    nemo_random_type drawn_random_numbers[NEURONS_PER_TN_CORE];
    nemo_random_type threshold_PRN_mask[NEURONS_PER_TN_CORE];


    explicit TrueNorthCore(int coreLocalId);

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

    void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

    void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

    void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

    void core_finish(tw_lp *lp) override;

    //construction of TN

    /**
     * Configures a TrueNorth neuron in this core. This function sets up the values required for a specific
     * neuron, passed via neuron_id.
     * @param neuron_id The neuron to configure
     * @param synaptic_connectivity This neuron's synaptic connectivity - must be an array
     * @param G_i
     * @param sigma
     * @param S
     * @param b
     * @param epsilon
     * @param sigma_l
     * @param lambda
     * @param c
     * @param alpha
     * @param beta
     * @param TM
     * @param VR
     * @param sigmaVR
     * @param gamma
     * @param kappa
     * @param signal_delay
     * @param dest_core
     * @param dest_axon
     */
    void create_tn_neuron(nemo_id_type neuron_id, bool synaptic_connectivity[NEURONS_PER_TN_CORE],
                          short G_i[NEURONS_PER_TN_CORE], short sigma[WEIGHTS_PER_TN_NEURON],
                          short S[WEIGHTS_PER_TN_NEURON], bool b[WEIGHTS_PER_TN_NEURON], bool epsilon,
                          short sigma_l, short lambda, bool c, int alpha, int beta, short TM, short VR, short sigmaVR,
                          short gamma, bool kappa, int signal_delay, nemo_id_type dest_core, nemo_id_type dest_axon);

    /**
     * TN Neurons compute a PRN value based on a random number and a bit mask. This function recreates this behavior.
     * @param pj
     * @param p_mask
     * @return
     */
    unsigned int gen_encoded_random(unsigned int pj, unsigned int p_mask);
    /**
     * This function generates PRN values for all TrueNorth neurons in the core.
     * @tparam T
     * @param holder
     * @param lp
     */
    template <typename T>
    void generate_prns(T *holder, tw_lp *lp){
        for(int i = 0; i < NEURONS_PER_TN_CORE; i ++){
            holder[i] = tw_rand_integer(lp->rng, 0, 255);
        }
    }
    /**
     * Fire and Reset functionality. This checks to see if the neuron needs to fire,
     * and if so sets the binary table fire_status[neuron_id] to true and resets the neuron.
     * @param lp
     */
    void fire_reset(tw_lp *lp);
    /**
     * Ringing check - eliminates "ringing" behavior in a neuron.
     * @param neuron_id
     */
    void ringing(nemo_id_type neuron_id);
    /**
     * called when a spike is received, this integrates across neurons in the core.
     * Will be updated with efficient OpenMP / Vector versions as well as a GPU version.
     *
     * @param lp
     * @param input_axon
     */
    void integrate(tw_lp *lp, nemo_id_type input_axon);
    //void post_integrate(tw_lp *lp, bool did_fire);
    /**
     * Leak functionality. Will compute leak values for the amount of time that has
     * passed since the last time this neurosynaptic core was active.
     */
    void leak(tw_lp *lp, tw_bf *bf);
    void send_heartbeat(tw_lp *lp, tw_bf *bf);
    void send_heartbeat(tw_lp *lp);





};
/** @} */
#endif //NEMO2_CORE_H
