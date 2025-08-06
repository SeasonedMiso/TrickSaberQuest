#!/bin/bash

# Parse arguments
QMOD_NAME=""
HELP=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --qmodName)
            QMOD_NAME="$2"
            shift 2
            ;;
        --help)
            HELP=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

if [ "$HELP" = true ]; then
    echo "\"createqmod\" - Creates a .qmod file with your compiled libraries and mod.json."
    echo ""
    echo "-- Arguments --"
    echo ""
    echo "--qmodName    The file name of your qmod"
    exit 0
fi

MOD_JSON="./mod.json"

# Validate mod.json
if [ ! -f "$MOD_JSON" ]; then
    echo "Error: mod.json not found"
    exit 1
fi

# Extract values from mod.json
MOD_NAME=$(grep -o '"name"[[:space:]]*:[[:space:]]*"[^"]*"' "$MOD_JSON" | cut -d'"' -f4)
VERSION=$(grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' "$MOD_JSON" | cut -d'"' -f4)

if [ -z "$QMOD_NAME" ]; then
    QMOD_NAME="$MOD_NAME"
fi

# Create file list
FILELIST=("$MOD_JSON")

# Add mod files
MOD_FILES=$(grep -A 10 '"modFiles"' "$MOD_JSON" | grep -o '"[^"]*\.so"' | tr -d '"')
for mod in $MOD_FILES; do
    path="./build/$mod"
    if [ ! -f "$path" ]; then
        path="./extern/libs/$mod"
    fi
    if [ ! -f "$path" ]; then
        echo "Error: could not find dependency: $path"
        exit 1
    fi
    FILELIST+=("$path")
done

# Add late mod files
LATE_MOD_FILES=$(grep -A 10 '"lateModFiles"' "$MOD_JSON" | grep -o '"[^"]*\.so"' | tr -d '"')
for mod in $LATE_MOD_FILES; do
    path="./build/$mod"
    if [ ! -f "$path" ]; then
        path="./extern/libs/$mod"
    fi
    if [ ! -f "$path" ]; then
        echo "Error: could not find dependency: $path"
        exit 1
    fi
    FILELIST+=("$path")
done

# Add library files
LIB_FILES=$(grep -A 10 '"libraryFiles"' "$MOD_JSON" | grep -o '"[^"]*\.so"' | tr -d '"')
for lib in $LIB_FILES; do
    path="./build/$lib"
    if [ ! -f "$path" ]; then
        path="./extern/libs/$lib"
    fi
    if [ ! -f "$path" ]; then
        echo "Error: could not find dependency: $path"
        exit 1
    fi
    FILELIST+=("$path")
done

# Ensure binaries directory exists
mkdir -p binaries

# Create both debug and release versions with version numbers
DEBUG_QMOD="binaries/${QMOD_NAME}_debug_v${VERSION}.qmod"
RELEASE_QMOD="binaries/${QMOD_NAME}_v${VERSION}.qmod"

# Remove existing qmods if they exist
rm -f "$DEBUG_QMOD" "$RELEASE_QMOD"

# Create debug version (unstripped)
zip -q "$DEBUG_QMOD" "${FILELIST[@]}"

# Create stripped version for release
STRIPPED_FILES=()
for file in "${FILELIST[@]}"; do
    if [[ "$file" == *build/*.so ]]; then
        stripped_file="${file%.so}_stripped.so"
        if [ -f "$stripped_file" ]; then
            STRIPPED_FILES+=("$stripped_file")
        else
            STRIPPED_FILES+=("$file")
        fi
    else
        STRIPPED_FILES+=("$file")
    fi
done

zip -q "$RELEASE_QMOD" "${STRIPPED_FILES[@]}"

echo "Created debug version: $DEBUG_QMOD"
echo "Created release version: $RELEASE_QMOD"
echo "Contents:"
for file in "${FILELIST[@]}"; do
    echo "  - $(basename "$file")"
done