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



#include "../../external/include/atomicops.h"
#include "../../external/include/readerwriterqueue.h"
#include <mpi.h>
#include <string>
#include <sstream>
#include <thread>
#include <utility>
#include <stdio.h>

using namespace moodycamel;

/**
 *
 */
enum output_mode{
      POSIX_CSV,
     MPI_BINARY
 };
struct SpikeData{
    nemo_id_type source_core;
    nemo_id_type dest_core;
    nemo_id_type source_neuron;
    nemo_id_type dest_axon;
    int source_neuro_tick;
    int dest_neuro_tick;
    double tw_source_time;


    SpikeData() = default;

    std::string const to_csv() const;
    static SpikeData from_csv(std::string data);
    bool operator==( SpikeData & rho ) const{

       return this->to_csv().compare(rho.to_csv()) == 0;
    }

};

/**
 * Manages output from neurosynaptic cores.
 * This class holds the functions and information needed to save spike information to disk.
 *
 */
class CoreOutput {
protected:
    const char *csv_header = "source_core,source_neuron,dest_core,dest_axon,source_neuro_tick,dest_neuro_tick,tw_source_time\n";
    bool verbose_output = false;
    std::string output_filename;
    std::ofstream posix_file;
public:
    const std::ofstream &getPosixFile() const {
        return posix_file;
    }
    void close_posix_file(){
        this->posix_file.close();
    }

protected:
    void open_files_posix();
    void set_filename(int rank);
public:
    CoreOutput(){
        output_filename = "nemo_spike_output";
    }
    CoreOutput(std::string outputFilename);
    virtual void save_spike(SpikeData spike);
    virtual ~CoreOutput();
};


/**
 * Threaded (producer/consumer) version of the spike file output class
 */
class CoreOutputThread:public CoreOutput {
    BlockingReaderWriterQueue<SpikeData> spike_queue;
    std::atomic_bool producer_running;

    void writer();

public:
    std::thread writer_thread;
    void setProducerRunning(const std::atomic_bool &producerRunning);

    CoreOutputThread(const std::string &outputFilename);
    void shutdown_thread();

    void save_spike(SpikeData spike) override ;

    virtual ~CoreOutputThread();

};
/**
 * MPI enabled version of the spike file output class
 * Saves binary structs. initial_number_of_spikes determines the MPI window for saving.
 * Currently this does not dynamically resize the number of expected spikes - that will be
 * implemented later.
 *
 */
class CoreOutputMPI:public CoreOutput{
    MPI_File  mpi_file;
    int rank;
    int nprocs;
    unsigned long filesize;
    unsigned long buffer_size;
    unsigned long num_spikes_to_save;
    unsigned long num_spikes_saved_in_rank;
    void init_mpi_file();
public:
    template <typename T>
    CoreOutputMPI(std::string &outputFilename, T initial_number_of_spikes);

    void save_spike(SpikeData spike) override;


    MPI_Comm file_communicator = MPI_COMM_WORLD;

};
#endif //NEMO2_COREOUTPUT_H
