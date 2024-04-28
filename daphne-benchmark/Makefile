.DEFAULT_GOAL := all
.PHONY: clean all cpu cuda opencl openmp openmp-offload

cpu:
	$(MAKE) -C src/CPU

cuda:
	$(MAKE) -C src/Cuda

opencl:
	$(MAKE) -C src/OpenCl

openmp:
	$(MAKE) -C src/OpenMP

openmp-offload:
	$(MAKE) -C src/OpenMP-offload

all: cpu cuda opencl openmp openmp-offload

clean:
	@$(MAKE) -C src/CPU clean
	@$(MAKE) -C src/Cuda clean
	@$(MAKE) -C src/OpenCl clean
	@$(MAKE) -C src/OpenMP clean	
	@$(MAKE) -C src/OpenMP-offload clean
