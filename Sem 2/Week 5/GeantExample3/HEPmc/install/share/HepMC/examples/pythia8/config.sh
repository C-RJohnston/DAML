#!/bin/sh
if [ ! $?LD_LIBRARY_PATH ]; then
  export LD_LIBRARY_PATH=/home/s1711493/Desktop/GeantExample3/HEPmc/build/../install/lib
fi
if [ $?LD_LIBRARY_PATH ]; then
  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/s1711493/Desktop/GeantExample3/HEPmc/build/../install/lib
fi
export PYTHIA8DATA=${PYTHIA8_HOME}/xmldoc
