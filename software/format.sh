#!/bin/sh

LICENSE_FILE="software/license-header"

FORMAT_ROOT="$(pwd)/$(dirname '$0')"
# Move to root of repo
cd $(git rev-parse --git-path ..)

FORMAT_FILE="$FORMAT_ROOT/.clang-format"
TIDY_FILE="$FORMAT_ROOT/.clang-tidy"

if [ ! -f $LICENSE_FILE ]; then
    echo "No license header found"
    exit -1
fi

FILES="$(echo **/Common/Src/**.c** **/CM?/Core/Src/**.c* **/Common/Inc/**.h* **/CM?/Core/Inc/**.h* | tr ' ' '\n')"
echo Processing files: $FILES

LICENSE_LENGTH=$(wc -l "$LICENSE_FILE" | awk '{print $1}') 
LICENSE_TEXT=$(head -n "$LICENSE_LENGTH" "$LICENSE_FILE" 2> /dev/null)
while read -r FILE; do
    # CHECK HEADER
    FILE_HEADER=$(head -n $LICENSE_LENGTH "$FILE" 2> /dev/null | dos2unix)
    if [ "$LICENSE_TEXT" != "$FILE_HEADER" ]; then
        TEMP_FILE=$(mktemp)
        cat "$LICENSE_FILE" > "$TEMP_FILE"
        cat "$FILE" >> "$TEMP_FILE"
        mv "$TEMP_FILE" "$FILE"
    fi

    # CHECK TRAILING NEWLINES
    if [ "$(tail -c 1 "$FILE" | wc -l)" -eq "0" ]; then
        echo "" >> "$FILE"
    else
        while [ "$(tail -c 2 "$FILE" | wc -l)" -gt "1" ]; do
            truncate -s -1 "$FILE"
        done
    fi
done <<< "$FILES"

# TODO: Run Clang Tidy
echo $FILES | xargs clang-format -i -style="file:$FORMAT_FILE"

