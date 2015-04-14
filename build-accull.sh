#! /bin/bash

echo "####################################"
echo "          accULL installation"
echo "####################################"
echo ""

echo " -------------------------------------------------------------------------------- "
echo " Copyright (c) 2012, Francisco de Sande Gonzalez. "
echo " All rights reserved. "
echo " "
echo " Redistribution and use in source and binary forms, with or without modification, "
echo " are permitted provided that the following conditions are met: "
echo " "
echo " * Redistributions of source code must retain the above copyright notice, this "
echo "   list of conditions and the following disclaimer. "
echo " * Redistributions in binary form must reproduce the above copyright notice, "
echo "   this list of conditions and the following disclaimer in the documentation "
echo "   and/or other materials provided with the distribution. "
echo " * Neither the name of Francisco de Sande nor the names of its contributors may "
echo "   be used to endorse or promote products derived from this software without "
echo "   specific prior written permission. "
echo " "
echo " THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND "
echo " ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED "
echo " WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE "
echo " DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE "
echo " LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR "
echo " CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE "
echo " GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) "
echo " HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT "
echo " LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT "
echo " OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE "
echo " -------------------------------------------------------------------------------- "

source ./env-parameters.sh
echo " ACCULLBASE = $ACCULLBASE "
echo ""

CUDADEFAULTDIR=/usr/local/cuda
OCLDEFAULTDIR=/usr/

if [ -f $ACCULLBASE/params.sh ]; then
	echo "$ACCULLBASE/params.sh exists ..."
	TMP=`cat $ACCULLBASE/params.sh|grep CUDADIR|cut -d'=' -f2`
	if [ -n $TMP ]; then
		CUDADEFAULTDIR=$TMP
	fi
	TMP=`cat $ACCULLBASE/params.sh|grep OCLSDKDIR|cut -d'=' -f2`
	if [ -n $TMP ]; then
		OCLDEFAULTDIR=$TMP
	fi
fi

eval "if [ -f $ACCULLBASE/params.sh ]; then
	echo 'Backing up params.sh to params.sh.bak'
	eval mv $ACCULLBASE/params.sh $ACCULLBASE/params.sh.bak
fi"
echo -n "" > params.sh

read -p "Do you want to compile CUDA Backend (Y/y/n)? [Y]es: " -n 1 -r
case $REPLY in
    "y" | "Y" | "")
    	COMPILEWITHCUDA="--enable-cuda"
	cudasdk=1
	while [ $cudasdk -ne 0 ]; do
	echo ""
        read -p "Select CUDA SDK directory ($CUDADEFAULTDIR): " -r
	case $REPLY in
		"")
			echo "export CUDADIR=$CUDADEFAULTDIR" >> $ACCULLBASE/params.sh
			echo "export ACCULL_NVIDIA_CUDA_BACKEND=1" >> $ACCULLBASE/params.sh
			cudasdk=0
		;;
		*)
	                eval "if [ -d $REPLY ]; then
			  echo 'export CUDADIR=$REPLY' >> $ACCULLBASE/params.sh
			  echo 'export ACCULL_NVIDIA_CUDA_BACKEND=1' >> $ACCULLBASE/params.sh
			  cudasdk=0
			  else
                            echo '$REPLY directory does not exists!'
			fi
                	"
		;;
	esac
	done
    ;;
esac
echo ""
read -p "Do you want to compile OPENCL Backend (Y/y/n)? [Y]es: " -n 1 -r
case $REPLY in
    "y" | "Y" | "")
    	COMPILEWITHOPCL="--enable-ocl"
	oclsdk=1
	while [ $oclsdk -ne 0 ]; do
	echo ""
        read -p "Select OCL SDK directory ($OCLDEFAULTDIR): " -r
	case $REPLY in
		"")
			echo "export OCLSDKDIR=$OCLDEFAULTDIR" >> $ACCULLBASE/params.sh
			if [ -z $COMPILEWITHCUDA ]; then
				echo "unset ACCULL_NVIDIA_CUDA_BACKEND" >>  $ACCULLBASE/params.sh
			fi
			echo "export ACCULL_AMD_OPENCL_BACKEND=1" >> $ACCULLBASE/params.sh
			oclsdk=0
		;;
		*)
	                eval "if [ -d $REPLY ]; then
				echo 'export OCLSDKDIR=$REPLY' >> $ACCULLBASE/params.sh
				if [ -z $COMPILEWITHCUDA ]; then
					echo 'unset ACCULL_NVIDIA_CUDA_BACKEND' >>  $ACCULLBASE/params.sh
				fi
				echo 'export ACCULL_AMD_OPENCL_BACKEND=1' >> $ACCULLBASE/params.sh
				oclsdk=0
			  else
                            echo '$REPLY directory does not exists!'
			fi
                	"
		;;
	esac
	done
    ;;
    *) 
	if [ -z "${COMPILEWITHCUDA}" ]; then
		echo ""
		echo ""
		echo " >>> ERROR: accull needs at least one backend. Exiting ... "
		echo ""
		exit 1
	fi
    ;;
esac
echo ""


read -p "Do you want to compile Debug options (Y/y/n)? [N]o: " -n 1 -r
case $REPLY in
    "y" | "Y")
		echo ""
		DEBUG='--enable-debug'
    	;;
    *)  
		echo ""
		DEBUG=''
    	;;
esac
echo ""

echo "All environment variables are in env-parameters.sh"

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
if [ $? -eq 0 ]; then
	echo " ................................. [ok]"
else
	exit 1
fi

./configure --enable-acc $COMPILEWITHCUDA $COMPILEWITHOPCL $DEBUG
if [ $? -eq 0 ]; then
	echo " ................................. [ok]"
else
	exit 1
fi
make clean; make
if [ $? -eq 0 ]; then
	echo " ................................. [ok]"
else
	exit 1
fi	
echo "#######################################"
echo "       accULL installation completed"
echo "#######################################"
echo " * "
echo " * Add '. $ACCULLBASE/env-parameters.sh' to your shell profile."
echo " * "
