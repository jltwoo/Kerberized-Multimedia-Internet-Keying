#!/bin/bash

KSM_DIR=$HOME"/multicast"
SCRIPTS_DIR=$KSM_DIR"/scripts"
source $SCRIPTS_DIR"/init.sh"
PORT_NUMBER=8888
BASE_TEST_USERNAME="ksm2_test2_user"
KSM2_TEST2_LOG=$RUNTIME_DIR"/ksm2_test2.log"

echo "Starting KSM2 test2" > $KSM2_TEST2_LOG

if [ -z "$1" ]; then
  MAX_USER_COUNT=5
else  
  MAX_USER_COUNT=$1
fi

if [ -z "$2" ]; then
  INCREMENT=1
else
  INCREMENT=$2
fi

for(( i=1 ; i <= MAX_USER_COUNT; i+=INCREMENT )) 
do
  # i = # of clients in system

  # Prepare client and ticket caches
  for(( j=1; j <= i; j++ ))
  do 
    TEST_USERNAME=$BASE_TEST_USERNAME$j
    
    cd $KRB5_ADMIN_DIR
    # Creates Krb principal for client 
    sudo ./krb_princ_create.sh $TEST_USERNAME $TEST_PASSWORD \ 
        >> $KSM2_TEST2_LOG
    
    cd "$SCRIPTS_DIR/basic"
    # Retrieve ticket and store in cache
    ./ticket_create.sh $TEST_USERNAME $TEST_PASSWORD \ 
        >> $KSM2_TEST2_LOG  

    echo "Created client and ticketcache for $TEST_USERNAME!"  
        >> $RUNTIME_DIR/ksm2_test2.log
  done

  # Perform Kerberos multicast request
  for(( j=1; j <= i; j++ ))
  do 

    TEST_USERNAME=$BASE_TEST_USERNAME$j

    echo "===================================================="
    cd $RUNTIME_DIR
    
    $KMTOOLS_DIR/sclient2 LOCALHOST group1 $RUNTIME_DIR/$TEST_USERNAME.cache >> $KSM2_TEST2_LOG 
    
    echo "===================================================="
  done 

  # Clearing all client ticket cache 
  for(( j=1; j <= i; j++ ))
  do 
    TEST_USERNAME=$BASE_TEST_USERNAME$j
    
    kdestroy -c $RUNTIME_DIR/$TEST_USERNAME.cache
    cd $KRB5_ADMIN_DIR
    sudo ./krb_princ_remove.sh $TEST_USERNAME >> $KSM2_TEST2_LOG

  done
done

exit 0
