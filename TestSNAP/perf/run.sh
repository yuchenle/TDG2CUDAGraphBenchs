TDG=output_TDG
BLOCK=output_original_blocking
THREAD=output_original_threading
CUR_DIR=$(dirname "$(readlink -f "$0")")
for i in {1..50};do
  ${CUR_DIR}/../bld/test_snap | grep "step time" -A 1 >> $TDG;
  ${CUR_DIR}/../bld/original_blocking_test_snap | grep "step time" -A 1 >> $BLOCK;
  ${CUR_DIR}/../bld//original_test_snap | grep "step time" -A 1 >> $THREAD;
done;
