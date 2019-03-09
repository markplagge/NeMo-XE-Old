#include <utility>

//
// Created by Mark Plagge on 2019-03-09.
//

#ifndef NEMO2_COREOUTPUT_H
#define NEMO2_COREOUTPUT_H
#include <iostream>
#include <fstream>
#include <cstdio>
#include "globals.h"
#include <mpi.h>

/**
 *
 */
enum output_mode{
      POSIX_CSV,
     MPI_BINARY
 };
struct SpikeOutput{
    nemo_id_type source_core;
    nemo_id_type dest_core;
    nemo_id_type source_neuron;
    nemo_id_type dest_axon;
    int source_neuro_tick;
    int dest_neuro_tick;
    double tw_source_time;
};
/**
 * Manages output from neurosynaptic cores.
 * This class holds the functions and information needed to save spike information to disk.
 *
 */
class CoreOutput {
    std::string output_filename;
    int output_method;
    bool use_threads;
    std::ofstream posix_file;

    MPI_File  mpi_file;
    int rank;
    int nprocs;
    int filesize;
    int buffer_size;

    void save_spike_csv();
    void save_spike_csv_thread();


    void save_spike_mpi();


    void open_files_posix();

public:

    MPI_Comm file_communicator = MPI_COMM_WORLD;
    CoreOutput(std::string outputFilename, int outputMethod, bool useThreads);
    template <typename T>
    CoreOutput(std::string &outputFilename, T initial_number_of_spikes);
    template <typename T>
    void init_mpi_file(T initial_number_of_spikes);
    void save_spike(SpikeOutput spike);

    virtual ~CoreOutput() {

    }

};


#endif //NEMO2_COREOUTPUT_H
