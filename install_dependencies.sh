#!/bin/bash 
echo "Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>"

if [ $# S ]
then
	echo "Error [ Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>] \nExiting"
	exit
fi

echo "Top level dir: $1"
TOPDIR=$1

### chec for dependencies 

sudo apt-get update
sudo apt install -y cmake gcc-7 g++-7 libboost1.65-dev libboost-thread1.65-dev libboost-regex1.65-dev libboost-log1.65-dev libboost-program-options1.65-dev libboost1.65-all-dev libxerces-c-dev libcurl4-openssl-dev libsnmp-dev libmysqlclient-dev libjsoncpp-dev uuid-dev libusb-dev libusb-1.0-0-dev libftdi-dev swig liboctave-dev gpsd libgps-dev portaudio19-dev libsndfile1-dev libglib2.0-dev libglibmm-2.4-dev libpcre3-dev libsigc++-2.0-dev libxml++2.6-dev libxml2-dev liblzma-dev dpkg-dev libmysqlcppconn-dev libev-dev libuv-dev git vim zip build-essential libssl-dev qtbase5-dev qtbase5-dev-tools curl libqhttpengine-dev googletest libgtest-dev lamp-server^


## work with libwebsockets

#mkdir -p $TOPDIR/ext

cd $TOPDIR/ext/
git clone https://github.com/usdot-fhwa-OPS/libwebsockets.git

cd $TOPDIR/ext/libwebsockets
cmake -DLWS_WITH_SHARED=OFF .
make
sudo make install

### make server shared object for Pedestrian Plugin 

cd $TOPDIR/ext/
git clone https://github.com/nitroshare/qhttpengine.git
cd $TOPDIR/ext/qhttpengine
cmake . 
make 
sudo make install

cd $TOPDIR/ext/server
cmake .
make 
sudo make install 


##setup gtest
cd /usr/src/googletest/googletest
sudo mkdir build
cd build
sudo cmake ..
sudo make
sudo cp libgtest* /usr/lib/
cd ..
sudo rm -rf build


sudo mkdir /usr/local/lib/googletest
sudo ln -s /usr/lib/libgtest.a /usr/local/lib/googletest/libgtest.a
sudo ln -s /usr/lib/libgtest_main.a /usr/local/lib/googletest/libgtest_main.a

