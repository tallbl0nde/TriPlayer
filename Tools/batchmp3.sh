#!/bin/bash

# Check for path as argument
if [ $# -ne 1 ]; then
	echo "Usage: batchmp3.sh <directory containing .m4a files>"
	exit
fi

# Check ffmpeg is installed
if ! [ -x "$(command -v ffmpeg)" ]; then
	echo "This script uses ffmpeg, which is not present on your system. Please install it and try again!"
	exit
fi

set -f
echo "Beginning conversion: .m4a -> .mp3"

# Get list of files to convert
files=`find $1 | grep ".m4a$"`

# Iterate over each file
echo "$files" | while read -r file; do
	echo ""
	echo Converting "$file" to mp3...

	# Split each path into it's respective parts
	path=${file%/*}
	name=${file##*/}
	name=${name%.*}

	# Convert
	bitrate=$(ffmpeg -y -nostdin -i "$file" 2>&1 | grep Audio | awk -F', ' '{print $5}' | cut -d' ' -f1)
	ffmpeg -y -nostdin -i "$file" -ab "$bitrate"k -map_metadata 0 -id3v2_version 3 -write_id3v1 1 "$path/$name.mp3" 2>/dev/null

	# Only remove original file if no errors occurred
	if [ $? -eq 0 ]; then
		echo "Conversion succeeded! Removing original file..."
		rm "$file"
	fi
done

set +f
echo "All done!"
