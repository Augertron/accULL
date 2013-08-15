#############################################
#          ACCULL ENV VARIABLES
#############################################

# CUDA Directory
export CUDADIR=

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
export PYTHONPATH=$YACFDIR:$PYTHONPATH

# CUDA and OpenCL PATH

export PATH=$CUDADIR/bin/:$PATH
export LD_LIBRARY_PATH=$CUDADIR/lib64:$CUDADIR/lib:/opt/soft/cuda/5.0/lib64:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=$CUDADIR'/include/':$FRANGOLLODIR'/src/interface/':$C_INCLUDE_PATH
export CPPFLAGS="-I$CUDADIR/include "$CPPFLAGS
export LDFLAGS="-L$CUDADIR/lib "$LDFLAGS
