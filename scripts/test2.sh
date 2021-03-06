#!/bin/bash

KSM_DIR=$HOME"/multicast"
SCRIPTS_DIR=$KSM_DIR"/scripts"
source $SCRIPTS_DIR"/init.sh"

GENERAL_TEST_LOG=$RUNTIME_DIR"/general_test2.log"
KSM1_TEST2_LOG=$GENERAL_TEST_LOG
KSM2_TEST2_LOG=$GENERAL_TEST_LOG


START_TIME=$(date +%s)

echo "Starting General test2" > $GENERAL_TEST_LOG

if [ -z "$1" ]; then
  MAX_USER_COUNT=100
else  
  MAX_USER_COUNT=$1
fi

if [ -z "$2" ]; then
  REPITITION=20
else
  REPITITION=$2
fi

RESULTS=$RESULTS_DIR"/test2_"$MAX_USER_COUNT"_"$REPITITION".dat"
KSM1_RESULTS=$RESULTS".ksm1"
KSM2_RESULTS=$RESULTS".ksm2"

echo "KSM1 KSM2 ${MAX_USER_COUNT} ${REPITITION}" > $RESULTS
echo -n "" > $KSM1_RESULTS
echo -n "" > $KSM2_RESULTS

cd $KRB5_ADMIN_DIR
# Creates Krb principal for groups 
sudo ./service_create.sh group 1 >> $GENERAL_TEST_LOG

for(( i=0; i < REPITITION; i++ ))
do
  BASE_TEST_USERNAME="ksm1_test2_user"
  echo -e "\nGeneral Test 1: KSM1"
  # Prepare client and ticket caches
  for(( j=1; j <= MAX_USER_COUNT; j++ ))
  do 
    TEST_USERNAME=$BASE_TEST_USERNAME$j
    
    cd $KRB5_ADMIN_DIR
    # Creates Krb principal for client 
    sudo ./krb_princ_create.sh $TEST_USERNAME $TEST_PASSWORD >> $KSM1_TEST2_LOG
    
    cd "$SCRIPTS_DIR/basic"
    # Retrieve ticket and store in cache
    ./ticket_create.sh $TEST_USERNAME $TEST_PASSWORD >> $KSM1_TEST2_LOG  

    echo -ne "Created client and ticketcache for $TEST_USERNAME ($j/$MAX_USER_COUNT)\r" 
  done

  echo "=======================KSM1========================="
  KSM1_BEFORE=$(date +%s)
  # Perform Kerberos multicast request
  for(( j=1; j <= MAX_USER_COUNT; j++ ))
  do 

    TEST_USERNAME=$BASE_TEST_USERNAME$j

    cd $RUNTIME_DIR
    
    $KMTOOLS_DIR/sclient LOCALHOST $KSM1_PORT_NUMBER $KSM1_GCKS_SNAME          group1 $RUNTIME_DIR/$TEST_USERNAME.cache $KSM1_RESULTS >> $KSM1_TEST2_LOG 
    
    if [ "$?" -ne "0" ]; then
      echo "\rKSM1: $TEST_USERNAME Failed.\n"
    else
      echo -ne "KSM1: Test $j/$MAX_USER_COUNT\r"
    fi
  done 
  KSM1_AFTER=$(date +%s)

  echo "===================================================="
  # Clearing all client ticket cache 
  for(( j=1; j <= MAX_USER_COUNT; j++ ))
  do 
    TEST_USERNAME=$BASE_TEST_USERNAME$j
    
    kdestroy -c $RUNTIME_DIR/$TEST_USERNAME.cache
    cd $KRB5_ADMIN_DIR
    echo -ne "Removing client and ticketcache $TEST_USERNAME ($j/$MAX_USER_COUNT)\r"  
    sudo ./krb_princ_remove.sh $TEST_USERNAME >> $KSM1_TEST2_LOG

  done
  
  echo -e "\nGeneral Test 1: KSM2"
  BASE_TEST_USERNAME="ksm2_test2_user"
  
  for(( j=1; j <= MAX_USER_COUNT; j++ ))
  do 
    TEST_USERNAME=$BASE_TEST_USERNAME$j
    
    cd $KRB5_ADMIN_DIR
    # Creates Krb principal for client 
    sudo ./krb_princ_create.sh $TEST_USERNAME $TEST_PASSWORD  >> $KSM2_TEST2_LOG
    
    cd "$SCRIPTS_DIR/basic"
    # Retrieve ticket and store in cache
    ./ticket_create.sh $TEST_USERNAME $TEST_PASSWORD  >> $KSM2_TEST2_LOG  

    echo -ne "Created client and ticketcache for $TEST_USERNAME ($j/$MAX_USER_COUNT)\r"  
  done

  echo "=========================KSM2======================"
  KSM2_BEFORE=$(date +%s)
  # Perform Kerberos multicast request
  for(( j=1; j <= MAX_USER_COUNT; j++ ))
  do 

    TEST_USERNAME=$BASE_TEST_USERNAME$j

    cd $RUNTIME_DIR
    
   $KMTOOLS_DIR/sclient2 $REALM group1 $RUNTIME_DIR/$TEST_USERNAME.cache $KSM2_RESULTS >> $KSM2_TEST2_LOG 
    if [ "$?" -ne "0" ]; then
      echo "\rKSM2: $TEST_USERNAME Failed.\n"
    else
      echo -ne "KSM2: Test $j/$MAX_USER_COUNT\r"
    fi
  done 
  KSM2_AFTER=$(date +%s)

  echo "===================================================="
  # Clearing all client ticket cache 
  for(( j=1; j <= MAX_USER_COUNT; j++ ))
  do 
    TEST_USERNAME=$BASE_TEST_USERNAME$j
    
    kdestroy -c $RUNTIME_DIR/$TEST_USERNAME.cache
    cd $KRB5_ADMIN_DIR
    echo -ne "Removing client and ticketcache $TEST_USERNAME ($j/$MAX_USER_COUNT)\r"  
    sudo ./krb_princ_remove.sh $TEST_USERNAME >> $KSM2_TEST2_LOG

  done

  KSM1_ELAPSED=$(( $KSM1_AFTER - $KSM1_BEFORE))
  KSM2_ELAPSED=$(( $KSM2_AFTER - $KSM2_BEFORE))
done

cd $KRB5_ADMIN_DIR
# Removes Krb principal for groups 
sudo ./service_remove.sh group 1 >> $GENERAL_TEST_LOG

ELAPSED=$(( $(date +%s)-$START_TIME ))
echo "Completed simulation with MAX_USER_COUNT(${MAX_USER_COUNT}) and INCREMENT(${INCREMENT}), Time elapsed ${ELAPSED}"

paste $KSM1_RESULTS $KSM2_RESULTS > $RESULTS
rm $KSM1_RESULTS $KSM2_RESULTS
exit 0
