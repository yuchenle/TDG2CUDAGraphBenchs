
CUR_DIR="$(dirname "$(readlink -f "$0")")"
OUTPUT=${CUR_DIR}/output_CUDA

for i in {1..50};do
  cd .. ; ./kernel | grep "total" >> $OUTPUT ; cd $CUR_DIR;
done;
