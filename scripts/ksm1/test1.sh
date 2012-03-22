#!/bin/bash

KSM_DIR=$HOME"/multicast"
SCRIPTS_DIR=$KSM_DIR"/scripts"
source $SCRIPTS_DIR"/init.sh"
TEST_USERNAME="ksm1_test1_user"

cd $KRB5_ADMIN_DIR
# Creates Krb principal for client 
sudo ./krb_princ_create.sh $TEST_USERNAME $TEST_PASSWORD

cd "$SCRIPTS_DIR/basic"
# Retrieve ticket and store in cache
./ticket_create.sh $TEST_USERNAME $TEST_PASSWORD

echo "Created client and ticketcache for $TEST_USERNAME!"
echo "===================================================="
cd $RUNTIME_DIR
$KMTOOLS_DIR/sclient $REALM $KSM1_PORT_NUMBER $KSM1_GCKS_SNAME group1 $RUNTIME_DIR/$TEST_USERNAME.cache

echo "===================================================="
kdestroy -c $RUNTIME_DIR/$TEST_USERNAME.cache
cd $KRB5_ADMIN_DIR
sudo ./krb_princ_remove.sh $TEST_USERNAME

exit 0
