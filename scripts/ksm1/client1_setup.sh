#!/bin/bash

KSM_DIR=$HOME"/multicast"
SCRIPTS_DIR=$KSM_DIR"/scripts"
KMTOOLS_DIR=$KSM_DIR"/kmtools"
source $SCRIPTS_DIR"/init.sh"
TEST_USERNAME="ksm1_testuser"

cd "$SCRIPTS_DIR/basic"
# Creates Krb principal for client 
sudo ./user_create.sh $TEST_USERNAME 1

# Retrieve ticket and store in cache
./ticket_create.sh $TEST_USERNAME"1" $TEST_PASSWORD

echo "Created client and ticketcache for $TEST_USERNAME"1" !\n"


exit 0
