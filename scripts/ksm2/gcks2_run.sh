#!/bin/bash

KSM_DIR=$HOME"/multicast"
SCRIPTS_DIR=$KSM_DIR"/scripts"
KMTOOLS_DIR=$KSM_DIR"/kmtools"
source $SCRIPTS_DIR"/init.sh"

if [ "$UID" -ne "$ROOT_UID" ]
then
   echo "Must be root to run script."
   exit $ERR_NOTROOT
fi


cd $RUNTIME_DIR
while [ 1 ]
do
  echo "Running GCKS 2..."
  $KMTOOLS_DIR/gcks2 $KSM2_GCKS_SNAME $RUNTIME_DIR/$KSM2_GCKS_SNAME.keytab
done

exit 0
