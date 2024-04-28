# TDG2CUDAGraphBenchs
This repository is composed of 6 applications/kernels that make use
of OpenMP offloading feature.
The objective is to assess the ability of Taskgraph framework to generate,
at run-time, CUDA graphs based on OpenMP target tasks and evaluate
its performance, comparing to the original target tasking and target 
threading models.


## Compilation
Each application should be compiled separately, as they have their 
own dependencies. README files are from the original repositories.

## Run
_todo_ Need to update run\_all.sh so that it launches the execution
of all applications

## dependent env. variables
**OMP\_PATH**:  set to the openmp runtime library path. Required to link the binaries.
**CUDA\_PATH**: set to CUDA root path, needed by TestSNAP and Neutral.
