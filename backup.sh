#!/bin/sh
############################################################
# 
# File:    backup.sh
#
# Version: 1.1.0
#
# Author:  Pietro Mele
# Email:   pietrom16@gmail.com
#
# Backup utility.
# Specify configuration parameters in "backup_config.txt".
# Specify files to be excluded from backup in "backup_exclude.txt".
#
# 2012.07.28
#
############################################################

# Default parameters

PRJOWNER=Unknown

## Backup directories

# Local backup directory
LOCAL_BACKUP_DIR=../backups

# Remote backup directory
BACKUP_SERVER_DIR=

source backup_config.txt

# Current developer (not necessarily the owner...)
USER=$(whoami)

# Set the project name as the current directory name
PROJECT=${PWD##*/}

### Files to be excluded from backups ###
EXCLUDE_PATTERN_FILE=./backup_exclude.txt
if [ ! -f "$EXCLUDE_PATTERN_FILE" ]
then
	touch "$EXCLUDE_PATTERN_FILE"
fi

# Exclude executables
#? find dir -executable -type f > execlist.txt
touch execlist.txt

BAR="---------------------------------------------------------------------------------------"

# Help message
if [ ! -z $1 ]
then
	echo $BAR
	echo
	echo "$0 - A backup utility."
	echo
	echo "Makes a backup copy of the directory where this script is located, including all subdirectories."
	echo "The backup file will be written both locally and on a backup server."
	echo
	echo "This script file must be copied in the root directory of your project."
	echo
	echo "The first time you use this script, in the \"backup_config.txt\" file:"
	echo "  - Set the PRJOWNER variable to your name, if you own the project."
	echo "  - Set the LOCAL_BACKUP_DIR variable to specify your local backup directory."
	echo "  - Set the BACKUP_SERVER_DIR variable to specify the backup server directory, if any."
	echo
	echo "If there are files you do not want to backup, you can write their file names in"
	echo "the \"backup_exclude.txt\" file, one per line (wildcards accepted, e.g. *.tmp)."
	echo
	echo "Usage:"
	echo "    $0 [-h]"
	echo
	echo "Project:                  " $PROJECT
	echo "Project owner:            " $PRJOWNER
	echo "Developer:                " $USER
	echo "Local backup directory:   " $LOCAL_BACKUP_DIR
	echo "Remote backup directory:  " $BACKUP_SERVER_DIR
	echo
	echo "Files/directories excluded from backup:"
	more "$EXCLUDE_PATTERN_FILE"
	more execlist.txt
	echo
	echo $BAR
	exit 0
fi

echo $BAR

# Create the local backup directory if it does not exist
mkdir -v -p "$LOCAL_BACKUP_DIR"

# Create the remote backup directory if it does not exist
if [ -d "$BACKUP_SERVER_DIR" ];
then
	REMOTE_BACKUP_DIR="$BACKUP_SERVER_DIR"/"$USER"/"$PROJECT"
	mkdir -v -p "$REMOTE_BACKUP_DIR"
else
	echo "Backup server not available. Making a local backup only..."
fi

# Create the compressed file's name
TIME=$(date +"%Y%m%d_%H%M")
PROJECT_BACKUP=${PROJECT}_${TIME}

# Compress
tar --create --preserve-permissions \
	--exclude-from="$EXCLUDE_PATTERN_FILE" --exclude-from=execlist.txt \
	-f "$LOCAL_BACKUP_DIR"/"$PROJECT_BACKUP".tar ../"$PROJECT"
bzip2 --force "$LOCAL_BACKUP_DIR"/"$PROJECT_BACKUP".tar

# Encryption
#TODO gpg

# Copy on the backup server
if [ -d "$REMOTE_BACKUP_DIR" ]; then
	cp "$LOCAL_BACKUP_DIR"/"$PROJECT_BACKUP".tar.bz2 "$REMOTE_BACKUP_DIR"/"$PROJECT_BACKUP".tar.bz2
	chmod a-x "$REMOTE_BACKUP_DIR"/"$PROJECT_BACKUP".tar.bz2
fi

# Result
echo
echo "Generated backups:"
[ -f "$LOCAL_BACKUP_DIR"/"$PROJECT_BACKUP".tar.bz2 ] && echo "    ""$LOCAL_BACKUP_DIR"/"$PROJECT_BACKUP".tar.bz2
if [ -d "$REMOTE_BACKUP_DIR" ]; then
	[ -f "$REMOTE_BACKUP_DIR"/"$PROJECT_BACKUP".tar.bz2 ] && echo "    ""$REMOTE_BACKUP_DIR"/"$PROJECT_BACKUP".tar.bz2
fi
echo

echo $BAR
echo

# ISSUES
# - chmod has no effect on Windows.

# TODO
# - Add encryption.
# - Automatically identify an executable file, and exclude it from backup.
#	Warning: some files could have execute permissions without being executables!
