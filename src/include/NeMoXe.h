//
// Created by plaggm on 4/3/19.
//

#ifndef NEMO2_NEMOXE_H
#define NEMO2_NEMOXE_H
#define NEMO_BUILD 1

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
    static  int map_fn(tw_lpid gid){
        return   (tw_peid) gid/g_tw_nlp ;
    }
    static void nemo_post_lp_init(tw_pe *pe){
        cout << "Init PE\n";
    }
    static void nemo_pre_lp_init(tw_pe *pe){
        cout << "Pre LP init\n";
    }
    static void nemo_pe_gvt_f(tw_pe *pe){
        cout <<"at GVT\n";
    }

    tw_petype nemo_pe[4] = {
            (pe_init_f) NeMoXe::nemo_pre_lp_init,
            (pe_init_f) NeMoXe::nemo_post_lp_init,
            (pe_gvt_f) NeMoXe::nemo_pe_gvt_f,
            0
    };

    tw_lptype mylps[9] = {
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
NeMoConfig &nemo_config = NeMoConfig::get_config();
#endif //NEMO2_NEMOXE_H
