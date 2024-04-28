.DEFAULT_GOAL := all
.PHONY: all clean checkdata

-include Makefile.deps
-include Configuration.cfg
CXX=clang++
CXXFLAGS=-O3 -g
CXXFLAGS+= -std=c++11
OMP= -fopenmp -fopenmp-targets=nvptx64
TDG=-fopenmp-taskgraph

all: kernel_threading kernel_blocking kernel_hetero_taskgraph

kernel_threading: ../../common/main.o kernel.o 
	$(CXX) -O3 $(OMP) $^ -o $@

kernel_blocking: ../../common/main.o block_kernel.o
	$(CXX) -O3 $(OMP) -DBLOCKING $^ -o $@

kernel_hetero_taskgraph: ../../common/main.o hetero_taskgraph_kernel.o
	$(CXX) -O3 $(OMP) $(TDG) -DBLOCKING -DTASKGRAPH $^ -o $@

../../common/main.o: ../../common/main.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -I../../include $< -o $@

kernel.o: kernel.cpp
	$(CXX) -O3 $(OMP) -c $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -I../../include $< -o $@

block_kernel.o: kernel.cpp
	$(CXX) -O3 $(OMP) -c -DBLOCKING $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -I../../include $< -o $@

hetero_taskgraph_kernel.o: hetero_tdg_kernel.cpp
	$(CXX) -O3 $(OMP) -c $(TDG) -DBLOCKING -DTASKGRAPH $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -I../../include $< -o $@

checkdata:
ifeq ($(wildcard ../../../data/p2i_input.dat),)
	$(warning p2i_input.dat not found. Did you forget to extract the test data?)
endif
ifeq ($(wildcard ../../../data/p2i_output.dat),)
	$(warning p2i_output.dat not found. Did you forget to extract the test data?)
endif

clean:
	rm -f kernel kernel.o ../../common/main.o Makefile.deps

Makefile.deps:
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -I../../include -MM ../../common/main.cpp *.cpp > Makefile.deps

