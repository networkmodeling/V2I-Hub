#!/bin/bash 
echo "Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>"

if [ $# S ]
then
	echo "Error [ Usage: sh ./installFresh.sh <toplevel V2X-Hub directory>] \nExiting"
	exit
fi

echo "Top level dir: $1"
TOPDIR=$1

### setup and install v2x-hub core and plugins 

cd $TOPDIR/src/v2i-hub/ 
cmake . -DqserverPedestrian_DIR=/usr/local/share/qserverPedestrian/cmake
make

### create plugins for installation 

ln -s ../bin CommandPlugin/bin
zip CommandPlugin.zip CommandPlugin/bin/CommandPlugin CommandPlugin/manifest.json
ln -s ../bin DsrcImmediateForwardPlugin/bin
zip DsrcImmediateForwardPlugin.zip DsrcImmediateForwardPlugin/bin/DsrcImmediateForwardPlugin DsrcImmediateForwardPlugin/manifest.json
ln -s ../bin MapPlugin/bin
zip MapPlugin.zip MapPlugin/bin/MapPlugin MapPlugin/manifest.json
ln -s ../bin MessageReceiverPlugin/bin
zip MessageReceiverPlugin.zip MessageReceiverPlugin/bin/MessageReceiverPlugin MessageReceiverPlugin/manifest.json
ln -s ../bin SpatPlugin/bin
zip SpatPlugin.zip SpatPlugin/bin/SpatPlugin SpatPlugin/manifest.json
ln -s ../bin HRIStatusPlugin/bin
zip HRIStatusPlugin.zip HRIStatusPlugin/bin/HRIStatusPlugin HRIStatusPlugin/manifest.json
ln -s ../bin HRIPredictedSpatPlugin/bin
zip HRIPredictedSpatPlugin.zip HRIPredictedSpatPlugin/bin/HRIPredictedSpatPlugin HRIPredictedSpatPlugin/manifest.json


sudo mysql -uroot << EOF
ALTER USER 'root'@'localhost' IDENTIFIED WITH 'mysql_native_password' BY 'ivp';
FLUSH PRIVILEGES;
EOF

cd $TOPDIR/DatabaseSetup
chmod +x install_db.sh
sudo ./install_db.sh

cd $TOPDIR/src/tmx/TmxCore/
sudo cp tmxcore.service /lib/systemd/system/
sudo cp tmxcore.service /usr/sbin/

sudo systemctl daemon-reload
sudo systemctl enable tmxcore.service
sudo systemctl start tmxcore.service
