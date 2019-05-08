//
// Created by plaggm on 4/3/19.
//

#ifndef NEMO2_NEMO_XE_H
#define NEMO2_NEMO_XE_H
// LIF Core settings:
constexpr int LIF_NEURONS_PER_CORE = 256;
constexpr int LIF_NUM_OUTPUTS = 256;
/**
 * @defgroup tn_const TrueNorth Neuron Limitations
 * Contains TrueNorth network constants
 * @{
 */
constexpr int NEURONS_PER_TN_CORE = 256;
constexpr int WEIGHTS_PER_TN_NEURON = 4;
constexpr int MAX_OUTPUT_PER_TN_NEURON = 1;

#include "CoreLP.h"
#include <ross.h>
#include "CoreOutput.h"
#include "NeMoConfig.h"
#include "globals.h"
#include "nemo_input.h"
#include "mapping.h"
#include "../neuro/INeuroCoreBase.h"
#include "../neuro/LIFCore.h"
#include "../neuro/TrueNorthCore.h"

class NeMoXe {
public:
    NeMoConfig configuation_state;
    tw_optdef cli_options;
    // Defines for static work - replace with metaprogramming once profiling data is done
    static constexpr int map_fn(tw_lpid gid){
        return (tw_peid) gid/g_tw_nlp;
    }
    void nemo_post_lp_init(tw_pe *pe){
        cout << "Init PE\n";
    }
    void nemo_pre_lp_init(tw_pe *pe){
        cout << "Pre LP init\n";
    }
    void nemo_pe_gvt_f(tw_pe *pe){
        cout <<"at GVT\n";
    }

    tw_petype nemo_pe = {
            (pe_init_f) nemo_pre_lp_init,
            (pe_init_f) nemo_post_lp_init,
            (pe_gvt_f) nemo_pe_gvt_f,
            0
    };

    tw_lptype mylps[] = {
            {(init_f) CoreLP::core_init,

                    (pre_run_f) CoreLP::pre_run,
                    (event_f) CoreLP::forward_event,
                    (revent_f) CoreLP::reverse_event,
                    (commit_f) CoreLP::core_commit,
                    (final_f) CoreLP::core_finish,
                    (map_f) nemo_map_linear,
                    sizeof(INeuroCoreBase)},
            {0},
    };

};


//@todo: Move this to a config file that will be set up by CMAKE
//#define THREADED_WRITER 1
extern std::vector<core_types> core_type_map;
#endif //NEMO2_NEMO_XE_H
