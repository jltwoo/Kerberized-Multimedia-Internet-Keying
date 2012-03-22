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

if [ -z "$1" ] || [ -z "$2" ]
then
  echo "Usage: $0 <base_service_name> <count>"
  echo "<base_service_name>   Base name of service principal"
  echo "<count>               Number of services to create"
  exit $ERR_NOARGS
fi

TEST_SERVICENAME=$1
SERVICE_COUNT=$2

cd "$SCRIPTS_DIR/krb5_admin"

for (( i=1 ; i <= $SERVICE_COUNT; i++ ))
do
./krb_princ_remove.sh $TEST_SERVICENAME$i/krb.localhost $TEST_PASSWORD
done

if [ ! -d "$RUNTIME_DIR" ]; then
   exit 0
fi

echo "Removing keytab file"
for (( i=1 ; i <= $SERVICE_COUNT; i++ ))
do
  filepath=$RUNTIME_DIR"/$TEST_SERVICENAME$i.keytab"
  sudo rm $filepath
done

#for (( i=1 ; i <= $SERVICE_COUNT; i++ ))
#do
#./krb_princ_remove.sh $TEST_SERVICENAME$i
#done

#for (( i=1 ; i <= $SERVICE_COUNT; i++ ))
#do
#  filepath=$RUNTIME_DIR"/$TEST_SERVICENAME$i.keytab"
#  rm $filepath
#done

exit 0
