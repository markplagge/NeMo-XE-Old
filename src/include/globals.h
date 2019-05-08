//
// Created by Mark Plagge on 2019-02-11.
//





#ifndef NEMO2_GLOBALS_H
#define NEMO2_GLOBALS_H

#include <stdint.h>
#include <type_traits>
#include <string>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <vector>

/** @defgroup global_macros Global Macro helper functions  *{ */
/**
 * JITTER(rng) -> macro for adding a jitter value to sent messages.
 */
constexpr int JITTER_SCALE = 1000;

template <typename T>
constexpr double  JITTER(T rng) { return tw_rand_unif(rng) / JITTER_SCALE;}

//#define JITTER (tw_rand_unif(lp->rng,0,JITTER_SCALE) / 10000)

/** @} */

/** @defgroup time_helpers
 * Helper functions and macros for ROSS timing
 * @{
 */
constexpr double LITTLE_TICK = (double) 1/1000;
constexpr int BIG_TICK = 1;


unsigned long get_neurosynaptic_tick(double now);

/** @todo: use this macro rather than calling yet another function and write more macros for timing */
#define GET_NEUROSYNAPTIC_TICK(now) long(now)

unsigned long get_next_neurosynaptic_tick(double now);

/**
 * lt_offset - This is the value of the next little tick. Use this when creating events in the
 * tw_offset.
 */
#define lt_offset(rng) = JITTER(rng) + LITTLE_TICK
/**
 * bt_offset - This is offset for the next big tick. Can use this when creating events in the tw_offset for
 * the next big tick event.
 */
#define bt_offset(rng) JITTER(rng) + BIG_TICK


/**@} */


/** @defgroup types Typedef Vars
 * Typedefs to ensure proper types for the neuron parameters/mapping calculations
 * @{  */

typedef int_fast64_t
        nemo_id_type; //!< id type is used for local mapping functions - there should be $n$ of them depending on CORE_SIZE
typedef int32_t nemo_volt_type; //!< volt_type stores voltage values for membrane potential calculations
typedef int64_t nemo_weight_type;//!< seperate type for synaptic weights.
typedef int32_t nemo_thresh_type;//!< Type for weights internal to the neurons.
typedef uint16_t nemo_random_type;//!< Type for random values in neuron models.

typedef uint64_t size_type; //!< size_type holds sizes of the sim - core size, neurons per core, etc.

typedef uint64_t stat_type;
/**@}*/
template<typename ... Args>
std::string string_format( const std::string& format, Args ... args );

typedef enum CoreTypes{
    TN,
    LIF
}core_types;


enum nemo_message_type{
    NEURON_SPIKE,
    HEARTBEAT
};
/** @} */

/** @defgroup bf_evt BF_EventStatus
 * Event Status enum / bitfield group. This group contains elements that manage the
 * enum / flags for event status created in NeMo.
 * @{
 */



/**
 * BF_Event_Status - A descriptive specialized replacement for tw_bf.
 * Flags the program flow when handling events.
 */
enum class BF_Event_Status:uint32_t{
    None = 0x00, //! No state changes happened
    Heartbeat_Sent = (1u << 1), //! a heartbeat message was sent
    Spike_Sent = (1u << 2), //a spike message was sent
    Output_Spike_Sent = (1u << 3), //! a spike message to an output layer was sent
    Heartbeat_Rec = (1u << 4), //! A heartbeat was received
    Spike_Rec = (1u << 5), //! A spike message was received
    NS_Tick_Update = (1u << 6), //! We updated the neurosynaptic tick value
    Leak_Update = (1u << 7), //! We updated the membrane potentials through a leak
    FR_Update = (1u << 8) //! We updated the membrane potentials through fire/reset computations

};
template <typename Enumeration>
auto as_integer(Enumeration const value)-> typename std::underlying_type<Enumeration>::type{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}




inline BF_Event_Status operator|(BF_Event_Status a, BF_Event_Status b){
    return static_cast<BF_Event_Status> (as_integer(a) | as_integer(b));
}

/**
 * Function that determines if the BF_Event_Status contains the supplied event.
 * @tparam I
 * @tparam E BF_Event_Status status or an integer.
 * @param a Base event status. Is the event b in this flag?
 * @param b Flag option - is this value in a?
 * @return if the value b is in a, then true.
 */



inline BF_Event_Status operator&(BF_Event_Status a, BF_Event_Status b){
    return static_cast<BF_Event_Status> (as_integer(a) & as_integer(b) );
}

template < typename E>
inline bool in_the(unsigned int a , E b){
    return bool (a  & as_integer(b));
}
inline bool in_the(BF_Event_Status a, BF_Event_Status b){
    return as_integer(a & b);
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
template <typename NEW_EVT>
inline BF_Event_Status add_evt_status(BF_Event_Status event_status, NEW_EVT new_event){
    return event_status | new_event;
}
template <typename ... NEW_EVT>
inline BF_Event_Status add_evt_status(BF_Event_Status  event_status, NEW_EVT ... new_event){
    return event_status | add_evt_status(new_event...);
}





/** @}  @defgroup global_help Global Helpers.
 * Global helper functions / classes which are used throughout NeMo @{ */

/**
 * crtp helper / basis class. Helps keep static polymorphism function boilerplate code managable.
 * @tparam T
 */
template <typename T>
struct crtp
{
    T& underlying() { return static_cast<T&>(*this); }
    T const& underlying() const { return static_cast<T const&>(*this); }
};

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
    std::string to_string();
}nemo_message;

// Helper functon for BF logic:

inline uint64_t get_gid_from_core_local(nemo_id_type dest_core, nemo_id_type dest_axon){
    //currently, cores are GIDs since this is a strict linear map
    return (uint64_t) dest_core;
}
/**
 * 2D Array helper template. Matrix is a 2D array using STD::Array
 * @tparam T
 * @tparam ROW
 * @tparam COL
 */
template <class T, size_t ROW, size_t COL>
using Matrix = std::array<std::array<T, COL>, ROW>;
/**
 * Matrix helper - VectorMatrix is a matrix of elements, ROW x COL size
 */
template <class T, size_t ROW, size_t COL>
using VectorMatrix = std::vector<std::vector<T>>;





/** @} */

/** @defgroup LIF_Model_Settings LIFModelSettings:
 * Model settings - define the size of arrays, number of outputs per neuron, etc..
 * @{
 */

/** @} */
/** @} */

#endif //NEMO2_GLOBALS_H
