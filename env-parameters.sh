#############################################
#          ACCULL ENV VARIABLES
#############################################

# SKDs directories 
# $ACCULLBASE/params.sh override those variables
#   if it exists. Run ./build-accull.sh to 
#   create it in any case.
export CUDADIR=/usr/local/cuda
export OCLSDKDIR=/usr

# Default NVIDIA CUDA architecture
export DEFAULTARCH="sm_20"

# ---- Do not edit below this line -----


# Get directory where is located this script
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

# accULL & CUDA directories
export ACCULLBASE=$DIR

# yacf directory
export YACFDIR=$ACCULLBASE/yacf/

# frangollo directory
export FRANGOLLODIR=$ACCULLBASE/frangollo

# PYTHONPATH
export PYTHONPATH=$YACFDIR

# If exists loads params.sh
params=$ACCULLBASE"/params.sh"
eval "if [ -f $params ]; then source $params; fi"

# CUDA and OpenCL PATH

export ACCULL_CUDA_LIB_DIR=$CUDADIR/lib:$CUDADIR/lib64
export ACCULL_OPCL_LIB_DIR=$OCLSDKDIR/lib/x86_64-linux-gnu

export PATH=$ACCULLBASE/:$OCLSDKDIR/bin/:$CUDADIR/bin/:$PATH
export LD_LIBRARY_PATH=$=$ACCULL_OPCL_LIB_DIR:$ACCULL_CUDA_LIB_DIR:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=$OCLSDKDIR'/include/':$CUDADIR'/include/':$FRANGOLLODIR'/src/interface/':$C_INCLUDE_PATH
export CPPFLAGS="-I$OCLSDKDIR/include -I$CUDADIR/include "$CPPFLAGS
export LDFLAGS="-L$ACCULL_OPCL_LIB_DIR -L$ACCULL_CUDA_LIB_DIR "$LDFLAGS

