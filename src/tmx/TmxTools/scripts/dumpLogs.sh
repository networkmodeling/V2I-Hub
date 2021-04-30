#!/bin/bash

PATH=/bin:/usr/bin:/usr/local/bin:/usr/local/sbin:$PATH
export PATH

cd /var/tmp

if [ $# -gt 0 ]; then
	mnt="${*}"
else
	mnt=`pwd`
fi

mkdir -p "${mnt}/tmx/log" || exit 1

rm /mnt/ubi
ln -s "${mnt}/tmx" /mnt/ubi

# Is this an RBS or a VBS
myip=`ifconfig | awk -F: '$1 ~ /inet addr/{print $2}' | grep 10\.30\.100 | awk '{print $1}'`
if ping -w 1 -c 1 tmx-obu >/dev/null; then
	# Remove any bad key from known_hosts
	ssh-keygen -f "$HOME/.ssh/known_hosts" -R tmx-obu
	
	# This adds a new entry to the known_hosts
	sshpass -p user ssh user@tmx-obu -o StrictHostKeyChecking=no -o CheckHostIP=no \
		grep WSMFwdRx_0_DestIP /mnt/ubi/obu.conf | \
		awk -F= '{print $2}' | grep -q "${myip}"
	if [ $? -eq 0 ]; then
		echo "Pulling OBU logs from tmx-obu..."
		sshpass -p user rsync -av -C user@tmx-obu:/mnt/ubi/log/ "${mnt}/tmx/log"
	fi
	archiveLogs.sh -a -o "${mnt}/vbs-`date '+%d-%b-%Y-%H%M%S'`.zip" || exit 1
elif ping -w 1 -c 1 tmx-rsu >/dev/null; then
	# Remove any bad key from known_hosts
	ssh-keygen -f "$HOME/.ssh/known_hosts" -R tmx-rsu
	
	sshpass -p rsuadmin ssh rsu@tmx-rsu -o StrictHostKeyChecking=no -o CheckHostIP=no ls >/dev/null
	if [ $? -eq 0 ]; then
		echo "Pulling RSU logs from tmx-rsu..."
		sshpass -p rsuadmin rsync -av -C rsu@tmx-rsu:/mnt/ubi/log/ "${mnt}/tmx/log" || exit 1
	fi
	archiveLogs.sh -a -o "${mnt}/rbs-`date '+%d-%b-%Y-%H%M%S'`.zip" || exit 1
fi	
