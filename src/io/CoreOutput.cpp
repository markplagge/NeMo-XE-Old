#include <utility>

//
// Created by Mark Plagge on 2019-03-09.
//

#include "../include/CoreOutput.h"

CoreOutput::CoreOutput(std::string outputFilename, int outputMethod, bool useThreads) : output_filename(
        std::move(outputFilename)),output_method(outputMethod), use_threads(useThreads) {

    if (output_method == POSIX_CSV) {
        open_files_posix();
    }else{
        // default MPI constructor - assumes 65535 spikes total
        auto ttl_spikes = 65535;
        init_mpi_file(ttl_spikes);
    }


}


void CoreOutput::save_spike_csv() {

}

void CoreOutput::save_spike_csv_thread() {

}

void CoreOutput::save_spike_mpi() {

}

void CoreOutput::open_files_posix() {
    posix_file.open(output_filename);
}
template <typename T>
void CoreOutput::init_mpi_file(T initial_number_of_spikes) {
    MPI_Comm_rank(file_communicator, &rank);
    MPI_Comm_size(file_communicator, &nprocs);
    filesize = sizeof(SpikeOutput) * initial_number_of_spikes;
    buffer_size = filesize / nprocs;

}

void CoreOutput::save_spike(SpikeOutput spike) {

}

template<typename T>
CoreOutput::CoreOutput(std::string &outputFilename, T initial_number_of_spikes):output_filename(std::move(outputFilename)) {
    output_method = MPI_BINARY;
    init_mpi_file(initial_number_of_spikes);

}


