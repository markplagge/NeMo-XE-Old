
//NOTE! This file is generated automatically using cmake from
//nemo_build_options.h.in. Changes to this file will be lost!
        //
// Created by plaggm on 4/18/19.
//

#ifndef NEMO2_NEMO_BUILD_OPTIONS_H
#define NEMO2_NEMO_BUILD_OPTIONS_H
#pragma once

#define DEF_TN_NEURONS_PER_CORE 256
#define DEF_TN_WEIGHTS_PER_NEURON 4
#define DEF_TN_MAX_NEURON_OUTPUTS 1

#define DEF_THREADED_WRITER 1 

#define DEF_LIF_NEURONS_PER_CORE 256
#define DEF_LIF_NUM_OUTPUTS 1


 // Build Options Structure 
 struct NeMoBuildOptions {
			constexpr static int TN_NEURONS_PER_CORE = DEF_TN_NEURONS_PER_CORE;
	constexpr static int TN_WEIGHTS_PER_NEURON = DEF_TN_WEIGHTS_PER_NEURON;
	constexpr static int TN_MAX_NEURON_OUTPUTS = DEF_TN_MAX_NEURON_OUTPUTS;
			constexpr static int LIF_NEURONS_PER_CORE = DEF_LIF_NEURONS_PER_CORE;
	constexpr static int LIF_NUM_OUTPUTS = DEF_LIF_NUM_OUTPUTS;
		constexpr static bool THREADED_WRITER = static_cast<bool>(DEF_THREADED_WRITER);
};
 inline constexpr NeMoBuildOptions nemo_build_options;
#endif //NEMO2_NEMO_BUILD_OPTIONS_H
