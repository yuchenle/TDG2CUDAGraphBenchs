# TDG2CUDAGraphBenchs
This repository is composed of 6 applications/kernels that make use
of OpenMP offloading feature.
These applications are used in the paper: https://dl.acm.org/doi/10.1145/3673038.3673050
The objective is to assess the ability of Taskgraph framework to generate,
at run-time, CUDA graphs based on OpenMP target tasks and evaluate
its performance, comparing to the original target tasking and target 
threading models.

## Quick Start
The repo is packed within a docker image where the dependencies are installed.
It should be easy to reproduce the experiments from that image. The address to download:
`https://doi.org/10.5281/zenodo.11634155`

The steps to follow is in "artifact.pdf"

The following steps should be avoided, as it makes life more compliated. They are left over
for consistency purposes.

## Compilation
Each application should be compiled separately, as they have their 
own dependencies. README files are from the original repositories.

## Run
_todo_ Need to update run\_all.sh so that it launches the execution
of all applications

## dependent env. variables
**OMP\_PATH**:  set to the openmp runtime library path. Required to link the binaries.
**CUDA\_PATH**: set to CUDA root path, needed by TestSNAP and Neutral.
