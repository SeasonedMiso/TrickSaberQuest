#!/bin/bash

# Create .qmod file with compiled libraries and mod.json

MOD_JSON="./mod.json"

# Validate mod.json exists
if [ ! -f "$MOD_JSON" ]; then
    echo "Error: mod.json not found"
    exit 1
fi

# Extract mod name from mod.json
MOD_NAME=$(grep -o '"name"[[:space:]]*:[[:space:]]*"[^"]*"' "$MOD_JSON" | sed 's/.*"name"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/')

if [ -z "$MOD_NAME" ]; then
    MOD_NAME="TrickSaber"
fi

echo "Creating .qmod for: $MOD_NAME"

# Create temporary directory for qmod contents
TEMP_DIR="temp_qmod"
rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR"

# Copy mod.json
cp "$MOD_JSON" "$TEMP_DIR/"

# Copy mod files (libtricksaber.so)
if [ -f "./build/libtricksaber.so" ]; then
    cp "./build/libtricksaber.so" "$TEMP_DIR/libtricksaber.so"
    echo "Added: libtricksaber.so"
else
    echo "Error: libtricksaber.so not found in ./build/"
    exit 1
fi

# Copy library files
for lib in libbeatsaber-hook_5_1_9.so libcustom-types.so libbsml.so libpaperlog.so libsl2.so; do
    if [ -f "./extern/libs/$lib" ]; then
        cp "./extern/libs/$lib" "$TEMP_DIR/$lib"
        echo "Added: $lib"
    else
        echo "Warning: $lib not found in ./extern/libs/"
    fi
done

# Create the .qmod file (which is just a zip)
QMOD_FILE="binaries/${MOD_NAME}.qmod"
rm -f "$QMOD_FILE"
cd "$TEMP_DIR"
zip -r "../$QMOD_FILE" *
cd ..

# Clean up
rm -rf "$TEMP_DIR"

echo "Created: $QMOD_FILE"

# Verify contents
echo "Contents of $QMOD_FILE:"
unzip -l "$QMOD_FILE"