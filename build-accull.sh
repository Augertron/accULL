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
cp $YACFDIR/config_local.example.py $YACFDIR/config_local.py
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
