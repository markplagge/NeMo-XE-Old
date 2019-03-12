//
// Created by Mark Plagge on 2019-02-11.
//

#include "core.h"
#include "../include/globals.h"
#include "../mapping.h"
#include "TrueNorthCore.h"
#include "LIFCore.h"
void CoreLP::event_trace(nemo_message *m, tw_lp *lp, char *buffer, int *collect_flag) {

}
void CoreLP::create_core(tw_lp *lp){
    // right now we are going to just create the test TN Cores
    // @TODO: Call the LP->Core mapping function
    // and make this point to whatever core type we are creating.

    // Create a neuro core based on a function return value
    // @TODO: Implement this
    ////
    auto coreid = get_core_from_gid(lp->gid);
    core_type = core_type_map[coreid];
    int coreLocalId = get_core_from_gid(lp->gid);
    if(core_type == TN) {


        auto ccore = new TrueNorthCore(coreLocalId, 0);
        ccore->core_init(lp);
        this->core = ccore;
        this->active = 1;
    }else if(core_type == LIF){
        auto ccore = new LIFCore(coreLocalId,0);
        this->core = ccore;
        this->active = 1;
    }else{
        std::cout << "Got an invalid core type\n";
    }

}

void CoreLP::setCore(INeuroCoreBase *core) {
    CoreLP::core = core;
}




// TrueNorth Core





INeuroCoreBase::INeuroCoreBase()  {
    
}
