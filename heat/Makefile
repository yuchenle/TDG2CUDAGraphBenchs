CC 		= clang
OMP 		= -fopenmp -fopenmp-targets=nvptx64
TDG		= -DTDG -fopenmp-taskgraph
TDG_LIB		= $(OMP_PATH)
CFLAGS = -O3 -g
# -DVERBOSE
LFLAGS = -lm

BIN=  heat_cudagraph heat_target heat_for_target

all: $(BIN)

misc.o: misc.c
	$(CC) -c $(CFLAGS) $< -o $@

heat_cudagraph: heat_target.c misc.o
	$(CC) $(OMP) $(CFLAGS) $+ $(TDG) $(LFLAGS) -o $@ -L$(TDG_LIB)

heat_target: heat_target.c misc.o
	$(CC) $(OMP) $(CFLAGS) $+ $(LFLAGS) -o $@ -L$(TDG_LIB)

heat_for_target: heat_for_target.c misc.o
	$(CC) $(OMP) $(CFLAGS) $+ $(LFLAGS) -o $@ -L$(TDG_LIB)
clean:
	rm -fr *.o $(BIN) *ppm tdg.dot tdg.c *_tdg.c 
