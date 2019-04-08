//
// Created by plaggm on 4/8/19.
//

#ifndef NEMO2_NEMOCONFIG_H
#define NEMO2_NEMOCONFIG_H

/**
 * NeMoXE Configuration Holder Object
 * This object contains the NeMo configuration information. Using a singleton/global object to
 * wrap all of the NeMo1(2) C style global parameters into a single holder makes sense.
 * Global configuration parameters and global settings are stored here.
 */
class NeMoConfig{
public:
    static NeMoConfig& get_config(){
        static NeMoConfig instance;
        if (instance.is_init == 0){
            instance.ne_tn_json_input_filename[0] = 'B';
            instance.ne_tn_json_input_filename[1] = 'X';
            instance.ne_tn_json_input_filename[2] = '\0';
        }
        return instance;
    }

    /**
     * Output spike save mode. 0: Save output spikes. 1: Save all spikes in simulation. 2: save no spikes
     */
    int ne_output_mode = 0;
    int is_init = 0;
    int buffer_size = 4096;

    int save_complete_model_execution = 0;

    char *ne_tn_json_input_filename = new char[buffer_size]();
    char *ne_model_input_filename   = new char[buffer_size]();
    char *ne_spike_input_filename   = new char[buffer_size]();
    char *ne_output_filename        = new char[buffer_size]();
    char *ne_spike_output_filename  = new char[buffer_size]();
    char *ne_core_typemap_filename  = new char[buffer_size]();
    int ne_num_cores_in_sim = 4096;
    int ne_num_neurons_per_core = 4096;
private:
    NeMoConfig() = default;
    ~NeMoConfig() = default;

public:
    //Delete copy constructor etc..
    NeMoConfig(const NeMoConfig&) = delete;
    NeMoConfig& operator=(const NeMoConfig&) = delete;
    NeMoConfig(NeMoConfig&&) = delete;
    NeMoConfig& operator=(NeMoConfig&&) = delete;
};

/**
 * Main configuration holder
 */
extern NeMoConfig &nemo_config;

#include <stdint.h>
#include <type_traits>
#include <string>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <vector>

#endif //NEMO2_NEMOCONFIG_H
