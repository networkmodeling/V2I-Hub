#!/bin/bash

PATH=/bin:/usr/bin:/usr/local/bin:/usr/local/sbin:$PATH
export PATH

cd /var/tmp

# First, grab the dmesg log to see if a usb drive has been mounted in the last minute
dmesg_out=`journalctl -k -S -1min -q --no-pager`
if [ -z "${dmesg_out}" ]; then
	# Nothing in the log
	exit 0
fi

# First, find all the USB drives mounted
df -h --output=source,target /media/usb? | grep "\/media" | sort -u | while read src mnt
do
	# See if this one was the recent mount
	part=`basename $src`
	if echo "${dmesg_out}" | grep -q ": ${part}"; then
		echo "New USB insert to ${src} detected at ${mnt}"
		
		# Find the inode for the physical partition
		inode=`ls -Li "${src}" | awk '{print $1}'`
		
		# Look for details in the disk devices list
		disk=`ls -Li /dev/disk/by-id/ | grep "^${inode}" | awk '{print $2}' | cut -d- -f2 | sed -e 's/_/ /g'`
		if echo "${disk}" | grep -q "SanDisk Cruzer Glide 3.0"; then
			dumpLogs.sh "${mnt}"
		fi 
	fi
done 

exit 0