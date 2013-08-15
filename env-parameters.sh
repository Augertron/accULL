#############################################
#          ACCULL ENV VARIABLES
#############################################


# accULL & CUDA directories
export ACCULLBASE=$(pwd)

# CUDA Directory
export CUDADIR=

# Default NVIDIA CUDA architecture
export DEFAULTARCH="sm_20"

# ---- Do not edit below this line -----


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
