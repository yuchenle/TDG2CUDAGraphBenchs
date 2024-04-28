CUR_DIR=$(dirname "$(readlink -f "$0")")
EXEC_FOLDER=${CUR_DIR}/..
TDG=${CUR_DIR}/output_TDG
BLOCK=${CUR_DIR}/output_original_blocking
THREAD=${CUR_DIR}/output_original_threading

INPUT=problems/csp.params
for i in {1..50};do
  cd $EXEC_FOLDER;
  echo $BLOCK
  ./original_block_neutral.omp4 $INPUT | grep "Wallclock" >> $BLOCK;
  cd $EXEC_FOLDER;
  ./taskgraph_neutral.omp4 $INPUT | grep "Wallclock" >> $TDG;
  cd $EXEC_FOLDER;
  ./original_neutral.omp4 $INPUT | grep "Wallclock" >> $THREAD;
done;

