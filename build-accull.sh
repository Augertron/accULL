#! /bin/bash

echo "####################################"
echo "          accULL installation"
echo "####################################"
echo ""
echo "All environment variables are in env-parameters.sh"
#cat env-parameters.sh
echo "--------------------------------------"
source env-parameters.sh
echo " ACCULLBASE = $ACCULLBASE "
echo " CUDADIR = $CUDADIR "
echo "#######################################"
echo "          Configuring yacf "
echo "#######################################"
if [ ! -e $YACFDIR/config_local.py ]; then
   echo " config_local.oy not found. Copying it from config_local.example.py ... "
   cp $YACFDIR/config_local.example.py $YACFDIR/config_local.py
fi
if [ ! -e $YACFDIR/makefile.mk ]; then
   echo " makefile.mk not found. Copying it from makefile_example.mk ... "
   cp $YACFDIR/makefile_example.mk $YACFDIR/makefile.mk
fi
cd $YACFDIR/Frontend
python _ast_gen.py
cd -
echo " ................................. [ok]"
echo " "
echo "#######################################"
echo "         Configuring frangollo "
echo "#######################################"
cd $FRANGOLLODIR
autoreconf -fiv
read -p "Are you want to compile CUDA Backend (Y/y/n)? " -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]
then
    COMPILEWITHCUDA="--enable-cuda"
fi
echo ""
read -p "Are you want to compile OPENCL Backend (Y/y/n)? " -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]
then
    COMPILEWITHOPCL="--enable-ocl"
fi
echo ""
./configure --enable-acc $COMPILEWITHCUDA $COMPILEWITHOPCL
make
echo "#######################################"
echo "       accULL installation completed"
echo "#######################################"
