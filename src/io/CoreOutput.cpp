

//
// Created by Mark Plagge on 2019-03-09.
//

#include <ross.h>
#include "../include/CoreOutput.h"

std::string const SpikeData::to_csv() const{
    //this is the expected format - perhaps define it somewhere?
    std::ostringstream output;
    output << source_core << "," << source_neuron << "," << dest_core << "," << dest_axon << "," << source_neuro_tick
           << "," << dest_neuro_tick <<"," << tw_source_time;
    return output.str();
}


SpikeData SpikeData::from_csv(std::string data) {
    SpikeData s;
    std::istringstream row;
    sscanf(data.c_str(),"%li,%li,%li,%li,%i,%i,%lf",
            &s.source_core,&s.source_neuron,&s.dest_core,&s.dest_axon,
            &s.source_neuro_tick,&s.dest_neuro_tick,&s.tw_source_time);
    return s;
}


/**
 *
 * @param outputFilename <- filename to save spikes. Will have .csv appeneded to it.
 */
CoreOutput::CoreOutput(std::string outputFilename) : output_filename(
        std::move(outputFilename)){
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    set_filename(rank);
    open_files_posix();
}
/**
 * Saves the spike to the CSV file
 *
 * @param spike
 */
void CoreOutput::save_spike(SpikeData spike) {
    posix_file << spike.to_csv() << "\n";
}
CoreOutput::~CoreOutput() {
    this->posix_file.close();
}

void CoreOutput::open_files_posix() {
    posix_file.open(output_filename);
    posix_file << csv_header;
}

void CoreOutput::set_filename(int rank) {
    std::ostringstream newfn;
    newfn << output_filename << "_" << rank << ".csv";
    output_filename = newfn.str();
}
/**
 * MPI File Ouptut class constructor.
 * @param outputFilename is a filename. Will have .dat appended to it.
 */
template<typename T>
CoreOutputMPI::CoreOutputMPI(std::string &outputFilename, T initial_number_of_spikes){

    //output_filename = std::move(outputFilename);
    output_filename = outputFilename + "_mpi.bin";
    MPI_Comm_rank(file_communicator, &rank);
    MPI_Comm_size(file_communicator, &nprocs);
    filesize = sizeof(SpikeData) * initial_number_of_spikes;
    buffer_size = filesize / nprocs;
    num_spikes_to_save = initial_number_of_spikes;
    init_mpi_file();
}


void CoreOutputMPI::init_mpi_file(){

}
void CoreOutputMPI::save_spike(SpikeData spike) {

}

/**
 * adds the spike to the file IO queue, which will be processed by the
 * running file processing thread. Calls the parent constructor to handle file names and opening
 * @param spike
 */

CoreOutputThread::CoreOutputThread(const std::string &outputFilename) : CoreOutput(outputFilename) {
    producer_running.store(true);
    writer_thread = std::thread(&CoreOutputThread::writer, this);
    spike_queue = BlockingReaderWriterQueue<SpikeData>(1024);
}

void CoreOutputThread::save_spike(SpikeData spike) {
    //try_enq
    if(! spike_queue.enqueue(spike)){
        tw_error(TW_LOC, "Could not enqueue output spike\n");
    }
}

CoreOutputThread::~CoreOutputThread() {
    producer_running.store(false);
    // wait for threads to finish
    writer_thread.join();
    // close the threads
    posix_file.close();
}

void CoreOutputThread::writer() {
    while(producer_running.load()) {

        SpikeData d;
        while( spike_queue.try_dequeue(d)) {
            auto test_str = d.to_csv();
            posix_file << d.to_csv() << "\n";
        }

    }
    //now the producer is not generating any more data, empty out the queue
    SpikeData d;
    while(spike_queue.try_dequeue(d)){
        auto test_str = d.to_csv();
        posix_file << d.to_csv() << "\n";
    }
}

