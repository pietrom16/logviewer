#!/bin/sh
# 
# File:   test_osx_ramdisk_log.sh
#
# Author: Pietro Mele
# Created on 02-Aug-2012, 10:15:25
#

# Test using the ram disk on Mac OS-X

#	References:
#		http://apple.wikia.com/wiki/RAM_disk#Creating_a_RAM_disk
#		http://osxdaily.com/2007/03/23/create-a-ram-disk-in-mac-os-x/
#		http://apple.stackexchange.com/questions/55794/why-do-mac-os-x-ramdisks-appear-to-be-limited-to-550mb-and-how-can-i-change-this


# Create the ram disk

diskutil erasevolume HFS+ "ramdisk" `hdiutil attach -nomount ram://2048`

# Copy some logs on the ram disk

cp ./test.log /Volumes/ramdisk/test.log

# Run logviewer

./dist/Debug/CLang-MacOSX/logviewer --input /Volumes/ramdisk/test.log --verbose --minLevel 3

umount  /Volumes/ramdisk
