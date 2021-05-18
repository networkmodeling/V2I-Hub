V2I Hub is no longer maintained; for an updated and supported version of this software, please visit the V2X Hub project.

https://github.com/usdot-fhwa-OPS/V2X-Hub

# Project Description

V2I Hub

V2I Hub was developed to support jurisdictions in deploying connected vehicle technology by reducing 
integration issues and enabling use of their existing transportation management hardware and systems. 
V2I Hub is a software platform that utilizes plugins to translate messages between different devices 
and run transportation management and connected vehicle applications on roadside equipment.

V2I Hub is middleware that runs on Linux Ubuntu 16.04 LTS. It is recommended that appropriate security 
and firewall settings be used on the computer running Linux, including conforming to your agency's 
security best practices and IT protocols.

# Prerequisites

The V2I Hub software can run on most Linux based computers with 
Pentium core processers, with at least two gigabytes of RAM and at least 64GB of drive space.
Performance of the software will be based on the computing power and available RAM in 
the system.  The project was developed and tested on a machine 
with a core i3 processor, 4GB of memory, 64GB of hard drive space, running Ubuntu 16.04 LTS.

The V2I Hub software was developed using c and c++ and requires the following packages installed via apt-get:
```
cmake
gcc-5
g++-5
libboost1.58-dev
libboost-thread1.58-dev
libboost-regex1.58-dev
libboost-log1.58-dev
libboost-program
options1.58-dev
libboost1.58-all-dev
libxerces-c-dev
libcurl4-openssl-dev
libsnmp-dev
libmysqlclient-dev
libjsoncpp-dev
uuid-dev
git
libusb-dev
libusb-1.0.0-dev
libftdi-dev
swig
liboctave-dev
libev-dev
libuv-dev
libglib2.0-dev
libglibmm-2.4-dev
libpcre3-dev
libsigc++-2.0-dev
libxml++2.6-dev
libxml2-dev
liblzma-dev
dpkg-dev
scons
```
Run the following command to install prerequisites via apt-get:
```
$ sudo apt-get install cmake gcc-5 g++-5 libboost1.58-dev libboost-thread1.58-dev libboost-regex1.58-dev libboost-log1.58-dev libboost-program-options1.58-dev libboost1.58-all-dev libxerces-c-dev libcurl4-openssl-dev libsnmp-dev libmysqlclient-dev libjsoncpp-dev uuid-dev libusb-dev libusb-1.0-0-dev libftdi-dev swig liboctave-dev portaudio19-dev libsndfile1-dev libglib2.0-dev libglibmm-2.4-dev libpcre3-dev libsigc++-2.0-dev libxml++2.6-dev libxml2-dev liblzma-dev dpkg-dev libmysqlcppconn-dev libev-dev libuv-dev git scons
```

This version of V2I Hub utilizes a customized libgps on Ubuntu 16 and 18 for support of newer uBlox GPS receivers with RTK such as the ZED-F9P. The gpsd-release-3.21 sources files are already included with this V2I Hub repository. Make sure to compile and install this library before proceeding:
```
$ cd V2I-Hub/gpsd-release-3.21
$ sudo scons install
```
Additionally, the libwebsockets 3.0 is used for the CommandPlugin, which is also included with this repository. To build and install:
```
$ cd V2I-Hub/libwebsockets-3.0.0
$ cmake -DLWS_WITH_SHARED=OFF .
$ make 
$ sudo make install
```

# Usage

## Building

To Compile the V2I Hub software
Run the following from the src directory
```
$ cd tmx
$ cmake .
$ make 
$ sudo make install
```
The library path to the new libraries needs to added to the LD_LIBRARY_PATH variable for the libraries to be found. This can be done in a per session basis with the command line.
```
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```
When the unit is rebooted this variable will not be set. To add the path at bootup modify the correct file in the /etc/ld.so.conf.d/ directory and add a line with the path “/usr/local/lib/”. For an Intel 64 bit system the correct file is “x86_64-linux-gnu.conf” but this will vary based on your platform. After the file is modified run this command to update the paths.
```
$ sudo ldconfig
```
Now, run the following from the src directory
```
$ cd v2i-hub
$ cmake .
$ make
```
This will create a bin directory that contains the plugin executable, as well as a directory for each plugin.  However, a V2I Hub plugin must be packaged in a ZIP file to be installed to a system.  In order to package up any one of the plugins from the v2i-hub directory, do the following:
```
$ ln -s ../bin <PluginName>/bin
$ zip <PluginName>.zip <PluginName>/bin/<PluginName> <PluginName>/manifest.json
```
The binary and the manifest file are the minimum number of files needed for any V2I Hub plugin.  It is possible some specific plugins require more files from the sub-directory to be included in the installable ZIP.

## Execution

Installation Instructions
- install lamp-server
```
$ sudo apt-get install lamp-server^
```
	enter a root password (i.e. ivp)
- install database
  - modify the install_db.sh script located in the DatabaseSetup directory.  Modify the value for DBROOTPASS to the password that was used for root during the previous step
  - save the script
  - execute the script using the following commands
```
$ chmod +x install_db.sh
$ sudo ./install_db.sh
```
- To setup a service to start tmxcore on Ubuntu copy the tmxcore.service file located in the "src/tmx/TmxCore" directory to the “/lib/systemd/system/” directory. Execute the following commands to enable the application at startup.
```
$ sudo systemctl daemon-reload
$ sudo systemctl enable tmxcore.service
$ sudo systemctl start tmxcore.service
```
## Set Up and Configuration Instructions

The CommandPlugin plugin must be running to access the Administration Portal. Follow the instructions above to build the CommandPlugin.zip package and then refer to Chapter 3 of the V2I Hub Administration Portal User Guide for installation and configuration instructions.

Instructions can be found to install additional plugins in the V2I Hub Software Configuration Guide.

## Administration Portal

The Administrator Portal can be launched by opening the v2i-webportal/index.html file with either Chrome or Firefox. Further instructions for hosting the portal on a web server can be found in the V2I_Hub_AdministrationPortalUserGuide.pdf.
The installation of the lamp-server package will install the Apache web server. 

Copy all directories and files for the admin portal to the /var/www/html directory.
The Administration Portal can then be reached by using the system’s IP address. 
The index.html file will automatically forward the browser to the Administration Portal.

http://<ip address>

NOTE: The MAP plugin will need an input file in order to run.  A sample XML input file for Turner Fairbank has been included in this deployment in the Sample MAP Input folder.

- Copy sample MAP input file
```
$ sudo cp Sample MAP Input\ STOL_MAP.xml /var/www/plugins/MAPr41/
$ cd /var/www/plugins/MAP/
$ sudo chmod 644 STOL_MAP.xml
$ sudo chown www-data STOL_MAP.xml
$ sudo chgrp www-data STOL_MAP.xml
$ cd src
```
## ISD Map Creator UPER File

The input needed by the MAP plugin can alternatively be created by using the ISD Message Creator tool to export binary (UPER Hex) data, saved to a file with a .txt extension and uploaded via the admin portal.  The next section explains the map input files in more detail. The input file(s) to be used must be placed in the /var/www/plugings/MAP directory and the permissions and owners changed via the commands above substituting your file name with the .txt extension.


# Version History and Retention
Version 3.3.0	- May, 2021 - This version was created for use with the RCVW Phase II application prototype.

# License

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
file except in compliance with the License.
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied. See the License for the specific language governing
permissions and limitations under the License.

# Contact Information

Contact Name: Jared Withers (FRA)
Contact Information: Jared.Withers@dot.gov, 202-493-6014

