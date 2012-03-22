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

cd "$SCRIPTS_DIR/basic"
# Creates Krb principal and keytab stored in $RUNTIME_DIR
./service_create.sh ksm2_gcks 1

echo "Created Service and keytab for ksm2_gcks1 !\n"


exit 0
