#############################################
#          ACCULL ENV VARIABLES
#############################################

# accULL base
export ACCULLBASE=$(pwd)/

# yacf directory
export YACFDIR=$ACCULLBASE/yacf/

# frangollo directory
export FRANGOLLODIR=$ACCULLBASE/frangollo

# PYTHONPATH
export PYTHONPATH=$YACFDIR:$PYTHONPATH

# Defaults to NVIDIA Cuda setup - edit accordingly for other platforms
# CUDA and OpenCL PATH
export PATH=/usr/local/cuda/bin/:$PATH
export CUDADIR=/usr/local/cuda
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:/usr/local/cuda/lib:$LD_LIBRARY_PATH
export CPPFLAGS="-I/usr/local/cuda/include":$CPPFLAGS
export LDFLAGS="-L/usr/local/cuda/lib":$LDFLAGS

echo " Ensure you have edited the previous values according to your platform "
echo " If your platform does not support CUDA , remove the --enable-cuda option "
echo " from the configure line in the build-accull.sh script "
echo " Remove the exit of the line below in this env script after you have setup the parameters "
exit
