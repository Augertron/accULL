#!/bin/bash


if [ -z $WORKDIR ]; then
    echo " Please, set WORKDIR to the base directory of yacf installation "
    echo " For example,  export WORKDIR=/home/$USER/yacf "
    exit -1
fi

export PYTHONPATH=$WORKDIR:$PYTHONPATH

#export YACF_CFLAGS=$YACF_CFLAGS

#if [[ $# -eq 3 ]]; then
#_CFLAGS=$1
#_SOURCE=$2
#_DEST=$3
#else
##_CFLAGS="$1"
_SOURCE=$1
_DEST=$2
_LIB=$3
#fi
#

echo "python $WORKDIR/bin/c2frangollo.py  -I $_LIB $_SOURCE $_DEST"
python $WORKDIR/bin/c2frangollo.py  -I $_LIB $_SOURCE $_DEST
