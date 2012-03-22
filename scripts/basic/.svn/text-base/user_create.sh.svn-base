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
  echo "Usage: $0 <base_client_name> <count>"
  echo "<base_client_name>   Base name of new client principal"
  echo "<count>              Number of clients to create"
  exit $ERR_NOARGS
fi

TEST_USERNAME=$1
USER_COUNT=$2

cd "$SCRIPTS_DIR/krb5_admin"

for (( i=1 ; i <= $USER_COUNT; i++ ))
do
./krb_princ_create.sh $TEST_USERNAME$i $TEST_PASSWORD
done

#for (( i=1 ; i <= $USER_COUNT; i++ ))
#do
#./krb_princ_remove.sh $TEST_USERNAME$i
#done

exit 0
