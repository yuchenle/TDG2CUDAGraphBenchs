CC = clang
PREFIX = src
NVCC = nvcc
PREFIX_TARGET = src_target
PREFIX_CUDA = src_cuda
NVCCFLAGS =
#using fopenmp-targets to link against C++ lib, so that libomp does complain about using c++ map
OMP = -fopenmp -fopenmp-targets=nvptx64
CFLAGS = -O3 -g
LIBS = -lm
TDG = -DTDG -fopenmp-taskgraph
TDG_LIB = $(OMP_PATH)
BIN = nbody

SRC = $(PREFIX)/util.c $(PREFIX)/kernel.c
SRC_CUDA = $(PREFIX_CUDA)/util.cu $(PREFIX_CUDA)/global_kernel.cu $(PREFIX_CUDA)/global_nbody_cuda.cu

SRC_TARGET = $(PREFIX_TARGET)/util.c $(PREFIX_TARGET)/kernel.c

all: $(BIN)_CUDAGraph $(BIN)_TARGET_BLOCKING $(BIN)_TEAMS_DISTRIBUTE $(BIN)_CUDA

$(BIN)_CUDAGraph: $(SRC_TARGET) $(PREFIX_TARGET)/main.c
	$(CC) -DBLOCKING $(CFLAGS) $(OMP) $(TDG) $^ -o $@ -g $(LIBS) -L$(TDG_LIB)

$(BIN)_TARGET_BLOCKING: $(SRC_TARGET) $(PREFIX_TARGET)/main.c
	$(CC) -DBLOCKING $(CFLAGS) $(OMP) $^ -o $@ -g $(LIBS)

$(BIN)_TEAMS_DISTRIBUTE: $(SRC_TARGET) $(PREFIX_TARGET)/main.c
	$(CC) $(CFLAGS) $(CFLAGS) $(OMP) $^ -o $@ -g $(LIBS)

$(BIN)_CUDA: $(SRC_CUDA) $(PREFIX_CUDA)/cuda_main.cu
	$(NVCC) $(CFLAGS) $(NVCCFLAGS) $^ -o $@ $(LIBS) -I$(PREFIX_CUDA)

.PHONY: clean

clean:
	rm $(BIN)_CUDAGraph $(BIN)_TARGET_BLOCKING $(BIN)_TEAMS_DISTRIBUTE $(BIN)_CUDA
