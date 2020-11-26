#!/bin/bash
# Check if we have the right number of arguments
if [ $# -ne 1 ]; then
    echo "Invalid arguments provided."
    echo "Usage:   ./rename.sh <folder to search>"
    echo "Example: ./rename.sh ./music/"
    exit -1
fi

# Confirm this is the directory to search
proceed="no"
echo "This script will remove any non-ASCII characters from the"
echo "names of EVERY file found in '$1'."
while true; do
    read -p "Do you wish to continue? [Y/N]: " yn
    case $yn in
        [Yy] )
            proceed="yes"
            break
        ;;

        [Nn] )
            proceed="no"
            break
        ;;

        * )
        ;;
    esac
done

if [ "$proceed" != "yes" ]; then
    echo "Exited without renaming any files"
    exit 0
fi

# Turn a relative path into absolute path
pushd "$(pwd)" > /dev/null
cd "$(dirname "$1")"
abs=$(printf "%s/%s\n" "$(pwd)" "$(basename "$1")")
popd > /dev/null

# Get a list of all files within the directory
files=$(find $abs -type f)
if [ $? -ne 0 ]; then
    exit 0
fi
count=$(echo "$files" | wc -l)

# Iterate over each file, replacing any 'bad' characters with underscores
changed=0
while IFS= read -r path; do
    # Split into directory and file name
    file=$(basename "$path")
    dir=$(dirname "$path")

    # Rename file
    new=$(echo $file | perl -pe 's/[^[:ascii:]]/_/g')
    if [ "$file" != "$new" ]; then
        mv "$dir/$file" "$dir/$new"
        echo "$1/{$file ==> $new}"
        changed=$((changed + 1))
    fi
done <<< "$files"

# Print results
echo "Done! Renamed $changed/$count files"