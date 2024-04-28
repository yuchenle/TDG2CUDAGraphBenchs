TDG=output_TDG
BLOCK=output_original_blocking
THREAD=output_original_threading
INPUT=../data/input/nbody_input.in_8192

#nbody_CUDAGraph  nbody_TARGET_BLOCKING	nbody_TEAMS_DISTRIBUTE

for i in {1..50};do
  ../nbody_CUDAGraph $INPUT 8192 256 | grep "passed" >> $TDG;
  ../nbody_TARGET_BLOCKING $INPUT 8192 256 | grep "passed" >> $BLOCK;
  ../nbody_TEAMS_DISTRIBUTE $INPUT 8192 256| grep "passed" >> $THREAD;
done;
