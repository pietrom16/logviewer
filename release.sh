#!/bin/sh
############################################################
# 
# File:    release.sh
#
# Version: 1.0.0
#
# Author:  Pietro Mele
# Email:   pietrom16@gmail.com
#
# Copy the executable to the directory it has to be run from.
#
# 2012.07.28
#
############################################################

############################################################
EXE_NAME=logviewer
LNK_NAME=
SOURCE_DIR=./dist/Release/CLang-MacOSX/
TARGET_DIR=~/bin/
############################################################


BAR="---------------------------------------------------------------------------------------"
echo $BAR
echo "Copying the \""$EXE_NAME"\" executable to the target device:" $TARGET_DIR

if [ -z "$EXE_NAME" ]; then
	echo "No executable specified. No copy has been done!"
	exit 1
fi

if [[ $SOURCE_DIR == $TARGET_DIR ]]; then
	echo "The target directory must be different from the source directory. No copy has been done!"
	exit 2
fi

# Copy the executable to the target directory:
cp  $SOURCE_DIR/$EXE_NAME  $TARGET_DIR/$EXE_NAME

# Make a link to the executable, so that the
# same file name can be used in other scripts:

if [ -n "$LNK_NAME" ]; then
	rm $TARGET_DIR/$LNK_NAME
	ln -s $TARGET_DIR/$EXE_NAME $TARGET_DIR/$LNK_NAME
fi

echo
ls -l $TARGET_DIR/$EXE_NAME
ls -l $TARGET_DIR/$LNK_NAME
echo $BAR
echo

