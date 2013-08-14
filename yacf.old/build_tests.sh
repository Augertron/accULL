#!/bin/bash

# Global settings
max_test_num=26
testdir=tmp_tests
base=$(pwd)
export PYTHONPATH=$(pwd)
YACF_DRIVER="python bin/c2frangollo.py "
VERBOSE_STS=0

# check_output
# -----------------------------
# Check that the output of the executable given in $1 
#  which is the output of test $2
#
function check_output ( ) 
{
   local name=$1
   local testn=$2
   if [ -e $name ]; then
     output=`$WRAPER_CMMD./$name`
     rc=$?
     echo "$output" > $base/$testdir/outputs/exe_${name}_${testn}
    if [[ $rc -ne 0 ]]; then 
		if [[ $rc -eq 2 ]]; then
			echo " Test $testn  not implemented in runtime (IGNORED)"
		else
  			echo " Test $testn FAILED (Result incorrect) "
		fi
    else
  		echo " Test $testn OK "
    fi
  		
   else
	echo " Test $testn FAILED (No $name file generated)"
   fi


}

# usage
# -------------------------------------
# Display script usage instructions
function usage ( ) 
{
  echo " This scripts runs a set of verification tests available on the examples/acc/ directory. "
  echo " For this script to work, both yacf and Frangollo needs to be properly installed. "
  echo " The config_local.py file of yacf must contain the proper Makefile header to build Frangollo "
  echo " Refer to config_local_example.py for information on how to write the file. "
  echo " Execution with no parameters builds test up to test $max_test_num but does not execute them. "
  echo " Parameters: "
  echo " -b BACKEND_NAME  : specifies backends to be run and check the output "
  echo "                    CUDA   :  Checks the CUDA component of frangollo "
  echo "                    OPENCL :  Checks the OpenCL component of frangollo "
  echo " -k               : Keeps the previous test results " 
  echo " -t TEST_NUM      : Run a particular test number "
  echo " -w WRAPER_CMMD   : Wraper command over executables like optirun"
}

# run_test
# ---------------------------------------
# Generates the code for a test, build the project directory and checks the results
function run_test ( ) 
{
   local tname=$1
   local TEST_CUDA=$2
   local TEST_OPENCL=$3

   if [ ! -e examples/acc/$tname ]; then
	echo "* $tname does not exists "
	return
   else
	echo "* Building $tname "
   fi
   
   if [[ $VERBOSE_STS -eq 1 ]]; then
      $YACF_DRIVER examples/acc/$tname $testdir/$tname 
   else
      $YACF_DRIVER examples/acc/$tname $testdir/$tname &> $testdir/outputs/out_$tname
   fi

   if [ ! -d $testdir/$tname/Frangollo ]; then
	echo "* Test $tname FAILED (no Project directory build )"
	return
   fi

   cd $testdir/$tname/Frangollo/

   if [[ $TEST_CUDA -eq 1 ]]; then
        make cuda_exe &> $base/$testdir/outputs/make_cuda_$tname
   	check_output main.exe $tname
   else
	echo "* Not testing CUDA binary "
   fi

   if [[ $TEST_OPENCL -eq 1 ]]; then
        make opencl_exe &> $base/$testdir/outputs/make_opencl_$tname
   	check_output main_ocl.exe $tname
   else
	echo "* Not testing OpenCL binary "
   fi
   cd ../../../
   return 
}


TEST_CUDA=0
TEST_OPENCL=0
KEEP_FILES=0
tname=0
WRAPER_CMMD=''
while getopts ":w:b:kt:" opt; do
   case $opt in
       w)
           if [[ -n $OPTARG ]]; then 
              WRAPER_CMMD=$OPTARG' '
              echo "* Wraping exe with $WRAPER_CMMD"
           else
              echo "! Option -$OPTARG requires an argument "
              usage 
              exit 1
           fi
           ;;
       b)
	   if [[ $OPTARG == 'CUDA' ]]; then
		TEST_CUDA=1
           	echo "* Testing $OPTARG backend" 
           elif [[ $OPTARG == 'OPENCL' ]]; then
	        TEST_OPENCL=1
           	echo "* Testing $OPTARG backend " 
	   else
		echo "! Backend $OPTARG not recognised "
		usage
		exit 1
           fi
           ;;
       k)  
	   if [ ! -d $testdir ]; then
		echo "* No old files to keep "
	   else
	   	echo "* Preserving old files "
	   	KEEP_FILES=1
	   fi
	   ;;
       t) 
	   echo "* Running only test num $OPTARG "
	   tname=$OPTARG
	   ;;
       \?)  
           echo "! Invalid option -$OPTARG"
           exit 1
           ;;
       :)
           echo "! Option -$OPTARG requires an argument "
	   usage 
           exit 1
           ;;
   esac
done

if [[ $KEEP_FILES -eq 0 ]]; then
  rm -Rf $testdir
  mkdir $testdir
  mkdir $testdir/outputs/
fi


if [[ $tname == "0" ]]; then
	echo "* Running $max_test_num tests "
	for i in `seq 1 $max_test_num`; do
	   if [[ $i -le 9 ]]; then
		tname=test0$i.c
	   else 
		tname=test$i.c
	   fi
	   run_test $tname $TEST_CUDA $TEST_OPENCL
	done

else
	echo "* Running only $tname " 
	run_test $tname $TEST_CUDA $TEST_OPENCL
fi

echo "* Finished "
