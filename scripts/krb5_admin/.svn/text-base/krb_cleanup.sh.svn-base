#!/bin/bash

KSM_DIR=$HOME"/multicast"
SCRIPTS_DIR=$KSM_DIR"/scripts"
source $SCRIPTS_DIR"/init.sh"

# Root privileges required
if [ "$UID" -ne "$ROOT_UID" ]
then
  echo "Must be root to run script."
  exit $ERR_NOTROOT
fi

if [ -z "$1" ] 
then
  echo "Usage: $0 <krb5_principal_name> "
  echo "<krb5_principal_name>   Name of new Kerberos principal to be added"
  exit $ERR_NOARGS
fi

if [ -z "$2" ] 
then
  COUNT=1
else 
  COUNT=$2
fi

for (( i=0; i<= $COUNT; i++))
do
  username=$1$i
  echo "$0.expect"
  $KRB5_ADMIN_DIR/expect/krb_princ_remove.sh.expect $username 

  rm $RUNTIME_DIR/$username.cache
  #{ # Begin code block
done

exit 0
