CUR_DIR="$(dirname "$(readlink -f "$0")")"
TDG=${CUR_DIR}/output_TDG
BLOCK=${CUR_DIR}/output_original_blocking
THREAD=${CUR_DIR}/output_original_threading

for i in {1..50};do
  cd .. ; ./kernel_threading | grep "total" >> $THREAD ; cd $CUR_DIR;
  cd .. ; ./kernel_blocking | grep "total" >> $BLOCK ; cd $CUR_DIR;
  cd .. ; ./kernel_hetero_taskgraph | grep "total" >> $TDG ; cd $CUR_DIR;
done;
