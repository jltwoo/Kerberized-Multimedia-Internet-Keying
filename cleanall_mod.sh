#!/bin/bash
# To be executed in the root minisip repository directory
#
# http://www.tslab.ssvl.kth.se/csd/projects/0931139/sites/default/files/h263.txt
# Modified by Jeffrey Woo, July 2010 

echo "//---------cleaning libmutil installation------------------------"
cd trunk/libmutil/
./make distclean
cd ../..
echo "//---------------------------------------------------------------"
echo "//---------cleaning libmnetutil installation---------------------"
cd trunk/libmnetutil
./make distclean
cd ../..
echo "//---------------------------------------------------------------"
echo "//-----------cleaning libmcrypto installation--------------------"
cd trunk/libmcrypto 
./make distclean
cd ../..
echo "//---------------------------------------------------------------"
echo "//-----------cleaning libmikey installation----------------------"
cd trunk/libmikey
./make distclean
cd ../..
echo "//---------------------------------------------------------------"
echo "//-----------cleaning libmsip installation-----------------------"
cd trunk/libmsip
./make distclean
cd ../..
echo "//---------------------------------------------------------------"
echo "//-----------cleaning libmstun installation----------------------"
cd trunk/libmstun
./make distclean
cd ../..
echo "//---------------------------------------------------------------"
echo "//-----------cleaning libminisip installation--------------------"
cd trunk/libminisip
./make distclean
cd ../..
echo "//---------------------------------------------------------------"
echo "//-----------cleaning minisip installation-----------------------"
cd trunk/minisip
./make distclean
cd ../..
