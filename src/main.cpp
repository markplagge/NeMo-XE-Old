#include <ross.h>
#include <iostream>
#include <mpi.h>
#include "neuro/core.h"
#include "globals.h"
using namespace std;



/*
 * PHOLD Globals
 */
tw_stime lookahead = 1.0;
static unsigned int stagger = 0;
static unsigned int offset_lpid = 0;
static tw_stime mult = 1.4;
static tw_stime percent_remote = 0.25;
static unsigned int ttl_lps = 0;
static unsigned int nlp_per_pe = 8;
static int g_phold_start_events = 1;
static int optimistic_memory = 100;

// rate for timestamp exponential distribution
static tw_stime mean = 1.0;

static char run_id[1024] = "undefined";
int map_fn(tw_lpid gid){
    return (tw_peid) gid/g_tw_nlp;
}
tw_lptype mylps[] = {
        {(init_f) Core::core_init,
                /* (pre_run_f) phold_pre_run, */
                (pre_run_f) NULL,
                (event_f) Core::forward_event,
                (revent_f) Core::reverse_event,
                (commit_f) Core::core_commit,
                (final_f) Core::core_finish,
                (map_f) map_fn,
                sizeof(Core)},
        {0},
};



int main(int argc, char *argv[]){
  tw_init(&argc, &argv);
  tw_define_lps(nlp_per_pe,sizeof(nemo_message));
    for(int i = 0; i < g_tw_nlp; i++)
    {
        tw_lp_settype(i, &mylps[0]);

    }
    if( g_tw_mynode == 0 )
    {
        printf("========================================\n");
        printf("PHOLD Model Configuration..............\n");
        printf("   Lookahead..............%lf\n", lookahead);
        printf("   Start-events...........%u\n", g_phold_start_events);
        printf("   stagger................%u\n", stagger);
        printf("   Mean...................%lf\n", mean);
        printf("   Mult...................%lf\n", mult);
        printf("   Memory.................%u\n", optimistic_memory);
        printf("   Remote.................%lf\n", percent_remote);
        printf("========================================\n\n");
    }
    tw_run();
    tw_end();
}