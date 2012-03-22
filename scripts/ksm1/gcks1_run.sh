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
  echo "Running GCKS 1..."
  $KMTOOLS_DIR/gcks1 -p $KSM1_PORT_NUMBER -s $KSM1_GCKS_SNAME -S $RUNTIME_DIR/$KSM1_GCKS_SNAME.keytab

exit 0
