TDG=output_TDG
BLOCK=output_original_blocking
THREAD=output_original_threading
INPUT=../test.dat
NUM_BLOCKS=2
for i in {1..50};do
  ../heat_cudagraph $INPUT $NUM_BLOCKS 4000 | grep "we spent" >> $TDG;
  ../heat_target $INPUT $NUM_BLOCKS 4000 | grep "we spent" >> $BLOCK;
  ../heat_for_target $INPUT $NUM_BLOCKS 4000 | grep "we spent" >> $THREAD;
done;
