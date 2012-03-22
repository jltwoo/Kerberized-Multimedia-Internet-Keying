#!/bin/bash

KSM_DIR=$HOME"/multicast"
SCRIPTS_DIR=$KSM_DIR"/scripts"
source $SCRIPTS_DIR"/init.sh"

echo "Running $(basename $0)"
if [ -z "$1" ] || [ -z "$2" ]
then
  echo "Usage: $0 <client_name> <password>"
  echo "<client_name>   Full client principal name e.g. test@LOCALHOST"
  echo "<password>      client's password"
  exit $ERR_NOARGS
fi

CLIENT_NAME=$1
CLIENT_PASSWORD=$2

cd "$SCRIPTS_DIR/basic"
expect ./expect/$(basename $0).expect $CLIENT_NAME $CLIENT_PASSWORD $RUNTIME_DIR/$CLIENT_NAME.cache

chmod 666 $RUNTIME_DIR/$CLIENT_NAME.cache

exit 0
