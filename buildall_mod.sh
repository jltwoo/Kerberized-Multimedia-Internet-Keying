#!/bin/bash
# Packages that were installed on Ubuntu 10.04 before running a script:
# sudo apt-get install libssl-dev
# sudo apt-get install libglademm-2.4-dev
# sudo apt-get install libsdl-dev
# sudo apt-get install subversion
# sudo apt-get install automake
# sudo apt-get install libtool
# sudo apt-get install libltdl3-dev
# sudo apt-get install build-essential
# sudo apt-get install libavcodec-dev
# 
# To be executed in the root minisip repository directory
#
# http://www.tslab.ssvl.kth.se/csd/projects/0931139/sites/default/files/h263.txt
# Modified by Jeffrey Woo, July 2010 : Enables sctp by default

echo "//--------------------------------------------------------------"
echo "//---------libmutil installation---------------------------------"
cd trunk/libmutil/
./bootstrap
./configure
make
make install
cd ../..
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//---------libmnetutil installation------------------------------------------------------"
cd trunk/libmnetutil
./bootstrap
./configure --enable-sctp
make 
make install
cd ../..
echo "//---------------------------------------------------------------"
echo "//-----------libmcrypto installation-----------------------------"
cd trunk/libmcrypto 
./bootstrap
./configure 
make 
make install
cd ../..
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//-----------libmikey installation-------------------------------"
cd trunk/libmikey
./bootstrap
./configure 
make 
make install
cd ../..
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//-----------libmsip installation--------------------------------"
cd trunk/libmsip
./bootstrap
./configure 
make 
make install
cd ../..
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//-----------libmstun installation-------------------------------"
cd trunk/libmstun
./bootstrap
./configure 
make 
make install
cd ../..
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//-----------libminisip installation------------------------------"
cd trunk/libminisip
./bootstrap
./configure 
make 
make install
cd ../..
echo "//---------------------------------------------------------------"
echo "//---------------------------------------------------------------"
echo "//-----------minisip installation--------------------------------"
cd trunk/minisip
./bootstrap
./configure --enable-textui 
make 
make install 
cd ../..
