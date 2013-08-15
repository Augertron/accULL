#! /bin/bash

echo "####################################"
echo "          accULL installation"
echo "####################################"
echo ""
echo "All environment variables are in env-parameters.sh"
cat env-parameters.sh
echo "--------------------------------------"
source env-parameters.sh
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
./configure --enable-acc --enable-cuda --enable-ocl
make
echo "#######################################"
echo "       accULL installation completed"
echo "#######################################"
