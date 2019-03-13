//
// Created by Mark Plagge on 2019-02-11.
//

#include "INeuroCoreBase.h"
#include "../include/globals.h"
#include "../include/mapping.h"
#include "TrueNorthCore.h"
#include "LIFCore.h"




// TrueNorth Core





INeuroCoreBase::INeuroCoreBase()  {
    
}

/**
 * Manages a heartbeat message. If this LP has not sent a heartbeat message, send it, and set the
 * heartbeat status to true.
 * If cur_message->message_type is NEURON_SPIKE, then this sets heartbeat_sent to false.
 *
 * !Note: using the my_bf (set from the calling ROSS function to tw_bf *bf) here:
 * c1 = heartbeat_sent was set to true (implies NEURON_SPIKE message type)
 * c2 = heartbeat_sent was set to false (implies HEARTBEAT message type)
 * c3 =
 *
 * Heartbeat logic:
 * At time t + e, core C receives a spike message.
 * If C has not sent a heartbeat, send one scheduled for t + 1.
 * If C has sent a heartbeat, then just integrate.
 * Last neurosynaptic tick time is stored in previous neuro tick: t_p
 *
 * At time t + 1, core C receives a heartbeat message.
 * C sets current neurosynaptic tick to t + 1.
 * C runs the leak function for every nerusynaptic tick between last_leak_time and current_neuro tick.
 * C sets the
 * C runs fire/reset function
 */
void INeuroCoreBase::forward_handler() {
    cur_rng_count = my_lp->rng->count; // set the random number count for reverse event counts.
    if(cur_message->message_type == NEURON_SPIKE){
        if (!this->heartbeat_sent){
            my_bf->c1 = 1;
            this->heartbeat_sent = true;
            // send the heartbeat event
            this->send_heartbeat();

        }else{// some error conditions:
            if(cur_message->intended_neuro_tick > this->current_neuro_tick){
                tw_error(TW_LOC, "Got a spike intended for t %d, but heartbeat has been sent and LP is active at time %d.\n"
                                 "Details:\n"
                                 "CoreID: %i \n"
                                 "Message Data:\n"
                                 "source_core,dest_axon,intended_neuro_tick,nemo_event_status,"
                                 "random_call_count,debug_time %s\n", this->cur_message->intended_neuro_tick,
                                 this->current_neuro_tick,this->coreid, cur_message->to_string().c_str());

            }
        }
    }else{

        //error check:


        //message is heartbeat - We need to call leak, fire, reset logic
        //but first we set the heartbeat event to false.
        my_bf->c2 = 1;
        this->heartbeat_sent = false;
    }

}

void INeuroCoreBase::reverse_handler()  {

}

void INeuroCoreBase::send_heartbeat() {
    auto now = tw_now(my_lp);
    auto next_tick = get_next_neurosynaptic_tick(tw_now(my_lp)) + JITTER(my_lp->rng);
    tw_event *heartbeat_event = tw_event_new(my_lp->gid,next_tick, my_lp);
    nemo_message *msg = (nemo_message *) tw_event_data(heartbeat_event);
    msg->intended_neuro_tick = next_tick;

}