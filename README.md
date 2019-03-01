
# *NeMo*: A Generic Neuromorphic Hardware Simulation Model built on top of ROSS
Author: Mark Plagge, Rensselear Polytechnic Institute
Copyright: 2019 - MIT LICENCE
Version 2.0
## About ##


*NeMo* is a simulation model designed to allow experimentation with different neuromorphic hardware. Built on top of ROSS, *NeMo* features excellent scalability across HPC clusters. *NeMo* is targeted at current neuromorphic hardware designs, and as such currently simulates spiking neural networks in a grid layout, similar to the [IBM TrueNorth Processor](http://science.sciencemag.org/content/345/6197/668.short). 

*NeMo* has undergone significant changes since the initial release. As such, NeMo v2 and later are now contained in the NeMo2 repository, while the original *NeMo* code base is in the *NeMo* repo. 

Benchmarks and results of the first edition of *NeMo* is available at the [ACM Digital Library](http://doi.acm.org/10.1145/2901378.2901392). 


Version 0.1.0 supports generating probabilistic networks for benchmarking purposes. It also only supports the IBM TrueNorth neuron specification, [available here](http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=6707077&tag=1).

[Wiki](https://github.com/markplagge/NeMo/wiki)

[![Stories in Ready](https://badge.waffle.io/markplagge/NeMo.svg?label=ready&title=Ready)](http://waffle.io/markplagge/NeMo)


Some notes:


Config BGRS for OMP Target Offload:

openmp offload BGRS:
  
  *Configure* /w CUDA 9.2 & XLC
  
 ` xlc_configure -o config_xlc -cuda $CUDA_PATH` 
  `xlc++ -F config_xlc -qsmp=omp -qoffload`
  
  Possibly the same for mpicc
  
  XLC++ expects CUDA 9.2 (XLC 16.0.1)
  
