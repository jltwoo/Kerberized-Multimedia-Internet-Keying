#!/bin/bash

KSM_DIR=$HOME"/multicast"
SCRIPTS_DIR=$KSM_DIR"/scripts"
source $SCRIPTS_DIR"/init.sh"

echo "Running $(basename $0)"
# Root privileges required
if [ "$UID" -ne "$ROOT_UID" ]
then
  echo "Must be root to run script."
  exit $ERR_NOTROOT
fi

if [ -z "$1" ] || [ -z "$2" ]
then
  echo "Usage: $0 <krb5_principal_name> <password>"
  echo "<krb5_principal_name>   Name of new Kerberos principal to be added"
  echo "<password>              Password used by principal"
  exit $ERR_NOARGS
fi

username=$1
password=$2

$KRB5_ADMIN_DIR/expect/$(basename $0).expect $1 $2 $REALM 


exit 0
