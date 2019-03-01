//
// Created by Mark Plagge on 2019-02-11.
//





#ifndef NEMO2_GLOBALS_H
#define NEMO2_GLOBALS_H

#include <stdint.h>
#include <type_traits>
/** @defgroup global_macros Global Macro helper functions */
#define JITTER(rng,c) tw_rand_unif((rng))
/** @defgroup types Typedef Vars
 * Typedefs to ensure proper types for the neuron parameters/mapping calculations
 * @{  */

typedef int_fast64_t
        nemo_id_type; //!< id type is used for local mapping functions - there should be $n$ of them depending on CORE_SIZE
typedef int32_t nemo_volt_type; //!< volt_type stores voltage values for membrane potential calculations
typedef int64_t nemo_weight_type;//!< seperate type for synaptic weights.
typedef uint32_t nemo_thresh_type;//!< Type for weights internal to the neurons.
typedef uint16_t nemo_random_type;//!< Type for random values in neuron models.

typedef uint64_t size_type; //!< size_type holds sizes of the sim - core size, neurons per core, etc.

typedef uint64_t stat_type;
/**@}*/


enum nemo_message_type{
    NEURON_SPIKE,
    HEARTBEAT
};
enum class BF_Event_Status:uint32_t{
    None = 0x00, // No state changes happened
    Heartbeat_Sent = (1u << 1), // a heartbeat message was sent
    Spike_Sent = (1u << 2), //a spike message was sent
    Output_Spike_Sent = (1u << 3), // a spike message to an output layer was sent
    Heartbeat_Rec = (1u << 4), // A heartbeat was received
    Spike_Rec = (1u << 5),
    NS_Tick_Update = (1u << 6),

};
template <typename Enumeration>
auto as_integer(Enumeration const value)-> typename std::underlying_type<Enumeration>::type{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

inline BF_Event_Status operator|(BF_Event_Status a, BF_Event_Status b){
    return static_cast<BF_Event_Status> (as_integer(a) | as_integer(b));
}

inline BF_Event_Status operator&(BF_Event_Status a, BF_Event_Status b){
    return static_cast<BF_Event_Status> (as_integer(a) & as_integer(b) );
}

inline BF_Event_Status& operator|=(BF_Event_Status &a, BF_Event_Status b){
    a = static_cast<BF_Event_Status> (
            as_integer(a) | as_integer(b)
            );
    return a;
}

inline bool is_hb_sent(BF_Event_Status event_status){
    return as_integer(event_status & BF_Event_Status::Heartbeat_Sent);
}
inline bool is_spike_sent(BF_Event_Status event_status){
    return as_integer(event_status & BF_Event_Status::Spike_Sent);
}
inline bool is_spike_recv(BF_Event_Status event_status){
    return as_integer(event_status & BF_Event_Status::Spike_Rec);
}

inline bool is_output_spike_sent(BF_Event_Status event_status){
    return as_integer(event_status & BF_Event_Status::Output_Spike_Sent);
}

inline bool is_heartbeat_rec(BF_Event_Status event_status){
    return as_integer(event_status & BF_Event_Status::Heartbeat_Rec);
}
inline bool is_nstick_updated(BF_Event_Status event_status){
    return as_integer(event_status & BF_Event_Status ::NS_Tick_Update);
}

template <typename EST>
inline bool in_event(BF_Event_Status status, EST event){
    return as_integer(status & event);
}

inline BF_Event_Status add_event_condition(BF_Event_Status event_status, BF_Event_Status new_event){
    //no check for accuracy, could add it here.
    return event_status | new_event;
}

inline BF_Event_Status add_event_condiditon(BF_Event_Status new_event){
    return new_event;
}

/** Gives us the BINCOMP (binary comparison) function used for stochastic weight modes.
 * Takes the absolute value of the first value, and compares it to the seocnd. */
template <typename T1, typename T2>
bool bincomp(T1 val1, T2 val2){
    return abs(val1) >= val2;
}
/** Kroniker Delta Function for TrueNorth */
template <typename T1>
constexpr auto dt(T1 x){
    return !(x);
}
/** SGN Function */
template <typename T1>
constexpr auto sgn(T1 x){
    return ((x > 0) - (x < 0));
}

/**
 * Main message data structure.
 *
 */
typedef struct Nemo_Message {
    int message_type;
    int source_core;
    int dest_axon;
    unsigned long intended_neuro_tick;
    uint32_t nemo_event_status;
    unsigned int random_call_count;
    double debug_time;

}nemo_message;

// Helper functon for BF logic:

inline uint64_t get_gid_from_core_local(nemo_id_type dest_core, nemo_id_type dest_axon){
    //currently, cores are GIDs since this is a strict linear map
    return (uint64_t) dest_core;
}



extern int NEURONS_PER_CORE;


unsigned long get_neurosynaptic_tick(double now);
unsigned long get_next_neurosynaptic_tick(double now);

#endif //NEMO2_GLOBALS_H
