#############################################
#          ACCULL ENV VARIABLES
#############################################

# CUDA Directory
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

# CUDA and OpenCL PATH

export PATH=$ACCULLBASE/:$OCLSDKDIR/bin/:$CUDADIR/bin/:$PATH
export LD_LIBRARY_PATH=$=$OCLSDKDIR/lib64:$CUDADIR/lib:$CUDADIR/lib64:$CUDADIR/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=$OCLSDKDIR'/include/':$CUDADIR'/include/':$FRANGOLLODIR'/src/interface/':$C_INCLUDE_PATH
export CPPFLAGS="-I$OCLSDKDIR/include -I$CUDADIR/include "$CPPFLAGS
export LDFLAGS="-L$OCLSDKDIR/lib -L$CUDADIR/lib "$LDFLAGS
