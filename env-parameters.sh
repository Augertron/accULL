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

CPU_ARCH=`uname -m`
export ACCULL_CPU_ARCH=${CPU_ARCH}
if [ ${CPU_ARCH} == 'x86_64' ]; then
  export ACCULL_x86_64=1
fi

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

# If exists loads params.sh
params="$ACCULLBASE/params.sh"
eval "if [ -f $params ]; then source $params; fi"

# CUDA and OpenCL PATH
if [[ $(uname -i) == 'armv7l' ]]; then	# Parallella arch armv7l
	OPCL_LIB_DIR='lib'
else
	OPCL_LIB_DIR='lib/x86_64-linux-gnu'
fi
export ACCULL_CUDA_LIB_DIR=$CUDADIR/lib:$CUDADIR/lib64
export ACCULL_OPCL_LIB_DIR=$OCLSDKDIR/$OPCL_LIB_DIR

export PATH=$ACCULLBASE/:$OCLSDKDIR/bin/:$CUDADIR/bin/:$PATH
export LD_LIBRARY_PATH=$=$ACCULL_OPCL_LIB_DIR:$ACCULL_CUDA_LIB_DIR:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=$OCLSDKDIR'/include/':$CUDADIR'/include/':$FRANGOLLODIR'/src/interface/':$C_INCLUDE_PATH
export CPPFLAGS="-I$OCLSDKDIR/include -I$CUDADIR/include "$CPPFLAGS
export LDFLAGS="-L$ACCULL_OPCL_LIB_DIR -L$ACCULL_CUDA_LIB_DIR "$LDFLAGS

