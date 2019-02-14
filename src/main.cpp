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

#include <random>
int test_omp_target(int n){
// Simple vector multiply
// Input vector X, Y
// Result vector is Z
float X[n],Y[n],Z[n];
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(1,65535.0);

#pragma parallel for
    for (int i =0; i < n; i ++){
        X[i] = distribution(generator);
        Y[i] = distribution(generator);
        Z[i] = 0;
    }
// GPU:
int i;
#pragma omp target
#pragma omp parallel for private(i)
for (i = 0; i < n; i ++) {
    Z[i] = X[i] * Y[i];
    //Z[i] =  sqrt(Z[i]);

}



return Z[0];
}
#include "math.h"

float extest2(int n) {
    int m = n;
    float A[n][n];
    float Anew[n][n];
    float error = 000;
    int iter = 0;
#pragma omp target
#pragma omp parallel for
    for(int j = 0; j < n; j ++){
        for(int i = 0; i < m; i ++){
            A[j][i]= (float)rand() / (float) rand();
            Anew[j][i] = 0;
        }
    }

    int iter_max = 65535;
    float tol = 0.1;

    while (error > tol && iter < iter_max) {
        error = 0.0;
#pragma omp target
        {
#pragma omp parallel for reduction(max:error)
            for (int j = 1; j < n - 1; j++) {
                for (int i = 1; i < m - 1; i++) {
                    Anew[j][i] = 0.25 * (A[j][i + 1] + A[j][i - 1]
                                         + A[j - 1][i] + A[j + 1][i]);
                    error = fmax(error, fabs(Anew[j][i] - A[j][i]));
                }
            }
#pragma omp parallel for
            for (int j = 1; j < n - 1; j++) {
                for (int i = 1; i < m - 1; i++) {
                    A[j][i] = Anew[j][i];
                }
            }
        }
        if (iter++ % 100 == 0) printf("%5d, %0.6f\n", iter, error);
    }

#pragma omp target teams distribute
    for( int j = 1; j < n-1; j++)
    {
#pragma omp parallel for reduction(max:error)
        for( int i = 1; i < m-1; i++ ) {
            Anew[j][i] = 0.25 * ( A[j][i+1] + A[j][i-1]
                                  + A[j-1][i] + A[j+1][i]);
            error = fmax( error, fabs(Anew[j][i] - A[j][i])); }
    }
#pragma omp target teams distribute
    for( int j = 1; j < n-1; j++)
    {
#pragma omp parallel for
        for( int i = 1; i < m-1; i++ ) {
            A[j][i] = Anew[j][i]; }
    }

    return error;
}

int main(int argc, char *argv[]){
    cout <<"Test omp\n";
    test_omp_target(165535);
    extest2(1024*512);
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