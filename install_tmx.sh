#!/bin/bash 
echo "Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>"

if [ $# S ]
then
	echo "Error [ Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>] \nExiting"
	exit
fi

echo "Top level dir: $1"
TOPDIR=$1

### Setup tmxcore 

cd $TOPDIR/src/tmx
cmake .
make 
sudo make install 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

sudo sed -i '$a /usr/local/lib/' /etc/ld.so.conf.d/x86_64-linux-gnu.conf
sudo ldconfig

