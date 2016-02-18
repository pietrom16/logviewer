#!/bin/sh
# 
# File:   test_linux_ramdisk_log.sh
#
# Author: Pietro Mele
# Created on 03-Aug-2012, 15:43:38
#

# Status: TO BE TESTED!

# Test using the ram disk on Linux

#	References:
#		http://en.wikipedia.org/wiki/Tmpfs
#		http://www.cyberciti.biz/faq/howto-create-linux-ram-disk-filesystem/
#		http://www.ibm.com/developerworks/linux/library/l-initrd/index.html
#		http://e2enetworks.com/2008/10/25/linux-in-memory-filesystems-tmpfs-vs-ramdisk/
#		http://superuser.com/questions/319479/how-do-i-deallocate-a-linux-ramdisk-e-g-dev-ram0

# Create the ram disk
# Use /tmpfs

# Copy some logs on the ram disk

cp ./test.log /tmpfs/test.log

# Run logviewer

logviewer --input /tmpfs/test.log --verbose --minLevel 3

