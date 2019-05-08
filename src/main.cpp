#include <ross.h>
#include <iostream>
#include <mpi.h>
#include "include/nemo_xe.h"

using namespace std;



// rate for timestamp exponential distribution
static tw_stime mean = 1.0;
static char run_id[1024] = "undefined";

//tw_lptype mylps[] = {
//        {(init_f) CoreLP::core_init,
//
//                (pre_run_f) CoreLP::pre_run,
//                (event_f) CoreLP::forward_event,
//                (revent_f) CoreLP::reverse_event,
//                (commit_f) CoreLP::core_commit,
//                (final_f) CoreLP::core_finish,
//                (map_f) nemo_map_linear,
//                sizeof(INeuroCoreBase)},
//        {0},
//};



// test bf logic:

// manage core and IO init
//nemo_post_lp_init()
int main(int argc, char *argv[]){

    //TrueNorthCore test_core(1) ;
    //core_init(&test_core, nullptr);

    // test out some stuff

    //test two
    // g_tw_nlp = 4096;
  tw_init(&argc, &argv);
  tw_pe_settype(tw_getpe(0),&main_pe);
  tw_define_lps(g_tw_nlp,sizeof(nemo_message));

    for(int i = 0; i < g_tw_nlp; i++)
    {
        tw_lp_settype(i, &mylps[0]);

    }
    if( g_tw_mynode == 0 ) {
        cout << "NeMo2 \n";
    }
    tw_run();
    tw_end();
    return 0;
}