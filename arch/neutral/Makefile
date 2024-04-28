# User defined parameters
KERNELS  					 = omp3
#COMPILER 					 = INTEL
COMPILER 					 = clang
MPI      					 = no
OPTIONS		 				+= -DTILES -g -DENABLE_PROFILING
#-DENABLE_PROFILING
#-g
#ARCH_COMPILER_CC   = icc
ARCH_COMPILER_CC   = clang++

# Compiler-specific flags
CFLAGS_INTEL			 = -O3 -qopenmp -no-prec-div -std=gnu99 -DINTEL \
										 -Wall -qopt-report=5 -xhost
CFLAGS_INTEL_KNL	 = -O3 -qopenmp -no-prec-div -std=gnu99 -DINTEL \
										 -xMIC-AVX512 -Wall -qopt-report=5
CFLAGS_GCC				 = -O3 -std=gnu99 -fopenmp -march=native -Wall
CFLAGS_GCCTX2			 = -O3 -std=gnu99 -fopenmp -Wall
CFLAGS_GCC_KNL   	 = -O3 -fopenmp -std=gnu99 \
										 -mavx512f -mavx512cd -mavx512er -mavx512pf
CFLAGS_GCC_POWER   = -O3 -mcpu=power8 -mtune=power8 -fopenmp -std=gnu99
CFLAGS_CRAY				 = -hfp3
CFLAGS_XL					 = -O3 -qsmp=omp
CFLAGS_XL_OMP4		 = -qsmp -qoffload
#CFLAGS_CLANG			 = -std=gnu99 -fopenmp=libiomp5 -march=native -Wall
CFLAGS_CLANG			 = -std=gnu99 -fopenmp -march=native -Wall
#CFLAGS_CLANG_OMP4  = -O3 -Wall -fopenmp-targets=nvptx64-nvidia-cuda -fopenmp-nonaliased-maps \
										 -fopenmp=libomp --cuda-path=$(CUDA_PATH) -DCLANG
CFLAGS_CLANG_OMP4  = -O3 -Wall -fopenmp-targets=nvptx64 -fopenmp -fopenmp-taskgraph
#--cuda-path=$(CUDA_PATH) -DCLANG
CFLAGS_PGI				 = -fast -mp
CFLAGS_PGI_NV			 = -fast -acc -ta=tesla:cc60 -Minfo=acc
CFLAGS_PGI_MC			 = -ta=multicore -fast 
CFLAGS_INTEL_RAJA  = -O3 -qopenmp -std=c++11 -DINTEL -Wall
CFLAGS_NVCC_RAJA   = -O3 -arch=sm_60 -x cu -std=c++11 --expt-extended-lambda -DRAJA_USE_CUDA

OPTIONS  					+= -D__STDC_CONSTANT_MACROS

ifeq ($(KERNELS), cuda)
  CHECK_CUDA_ROOT = yes
endif
ifeq ($(COMPILER), CLANG_OMP4)
  CHECK_CUDA_ROOT = yes
endif

ifeq ($(CHECK_CUDA_ROOT), yes)
ifeq ("${CUDA_PATH}", "")
$(error "$$CUDA_PATH is not set, please set this to the root of your CUDA install.")
endif
endif

ifeq ($(DEBUG), yes)
  OPTIONS += -O0 -DDEBUG -g
endif

ifeq ($(MPI), yes)
  OPTIONS += -DMPI
endif

# Default compiler
ARCH_LINKER    		= $(ARCH_COMPILER_CC)
ARCH_FLAGS		= $(CFLAGS_$(COMPILER)) $(OPTIONS) -fopenmp -fopenmp-targets=nvptx64 -fopenmp-taskgraph
ARCH_LDFLAGS   		= $(ARCH_FLAGS) -lm -fopenmp -fopenmp-targets=nvptx64
ARCH_BUILD_DIR 		= ../obj/neutral/
ARCH_DIR       		= ..

ifeq ($(KERNELS), cuda)
  include Makefile.cuda
  OPTIONS += -DSoA
endif

ifeq ($(KERNELS), omp4)
  OPTIONS += -DSoA
endif

ifeq ($(KERNELS), oacc)
  OPTIONS += -DSoA
endif

ifeq ($(KERNELS), raja)
ifeq ("${RAJA_PATH}", "")
$(error "$$RAJA_PATH is not set, please set this to the root of your RAJA install.")
endif
  OPTIONS += -I$(RAJA_PATH)/include/
  ARCH_LDFLAGS = -arch=sm_60 -Xcompiler "-fopenmp" -lRAJA -L$(RAJA_PATH)/lib
endif

# Get specialised kernels
SRC  			 = $(wildcard *.c)
SRC  			+= $(wildcard $(KERNELS)/*.c)
SRC  			+= $(wildcard $(ARCH_DIR)/$(KERNELS)/*.c)
SRC 			+= $(subst main.c,, $(wildcard $(ARCH_DIR)/*.c))
SRC_CLEAN  		= $(subst $(ARCH_DIR)/,,$(SRC))
PREV_OBJS		+= $(patsubst %.c, $(ARCH_BUILD_DIR)/%.o, $(SRC_CLEAN))
#PREV_OBJS contains all the object files
OBJS 			+= $(subst ../obj/neutral//omp4/original_neutral.o,,$(PREV_OBJS))
#TASKGRAPH_OBJS doesn't have neutral.o (blocking version) and original_neutral.o
TASKGRAPH_OBJS  = $(subst ../obj/neutral//omp4/neutral.o,,$(PREV_OBJS))
TDG_OBJS  := $(subst ../obj/neutral//omp4/original_neutral.o,,$(TASKGRAPH_OBJS))
#FOR_OBJS doesn't have neutral.o (blocking version)
FOR_OBJS 		= $(subst ../obj/neutral//omp4/neutral.o,, $(OBJS))

all: original_block_neutral taskgraph_neutral original_neutral

original_block_neutral: make_build_dir $(OBJS) Makefile
	$(ARCH_LINKER) $(OBJS) $(ARCH_LDFLAGS) -o original_block_neutral.$(KERNELS)

taskgraph_neutral:make_build_dir $(TDG_OBJS) $(ARCH_BUILD_DIR)/$(KERNELS)/taskgraph_neutral.o Makefile
	$(ARCH_LINKER) $(TDG_OBJS) $(ARCH_BUILD_DIR)/$(KERNELS)/taskgraph_neutral.o $(ARCH_LDFLAGS) -o taskgraph_neutral.$(KERNELS)

original_neutral: make_build_dir $(FOR_OBJS) $(ARCH_BUILD_DIR)/$(KERNELS)/original_neutral.o Makefile
	$(ARCH_LINKER) $(FOR_OBJS) $(ARCH_BUILD_DIR)/$(KERNELS)/original_neutral.o  $(ARCH_LDFLAGS)  -o original_neutral.$(KERNELS)

# Rule to make controlling code
$(ARCH_BUILD_DIR)/%.o: %.c Makefile 
	$(ARCH_COMPILER_CC) $(ARCH_FLAGS) -c $< -o $@

$(ARCH_BUILD_DIR)/%.o: $(ARCH_DIR)/%.c Makefile 
	$(ARCH_COMPILER_CC) $(ARCH_FLAGS) -c $< -o $@

$(ARCH_BUILD_DIR)/$(KERNELS)/original_neutral.o: $(KERNELS)/original_neutral.c Makefile
	$(ARCH_COMPILER_CC) $(ARCH_FLAGS) -c $< -o $@

$(ARCH_BUILD_DIR)/$(KERNELS)/taskgraph_neutral.o: $(KERNELS)/neutral.c
	$(ARCH_COMPILER_CC) $(ARCH_FLAGS) -DTASKGRAPH -c $< -o $@

make_build_dir:
	@mkdir -p $(ARCH_BUILD_DIR)/
	@mkdir -p $(ARCH_BUILD_DIR)/$(KERNELS)

clean:
	rm -rf $(ARCH_BUILD_DIR)/* neutral.$(KERNELS) *.vtk *.bov \
		*.dat *.optrpt *.cub *.ptx *.ap2 *.xf *.ptx1

