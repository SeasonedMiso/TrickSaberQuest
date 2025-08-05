#!/bin/bash

set -e

CLEAN=false
LOG=false
USE_DEBUG=false
HELP=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --log)
            LOG=true
            shift
            ;;
        --use-debug)
            USE_DEBUG=true
            shift
            ;;
        --help)
            HELP=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--clean] [--log] [--use-debug] [--help]"
            exit 1
            ;;
    esac
done

if [ "$HELP" = true ]; then
    echo "\"Copy\" - Builds and copies your mod to your quest, and also starts Beat Saber with optional logging"
    echo ""
    echo "-- Arguments --"
    echo ""
    echo "--clean        Performs a clean build (equivalent to running \"build --clean\")"
    echo "--use-debug    Copies the debug version of the mod to your quest"
    echo "--log          Logs Beat Saber using the \"start-logging\" command"
    exit 0
fi

# Build first
if [ "$CLEAN" = true ]; then
    ./scripts/build.sh --clean
else
    ./scripts/build.sh
fi

# Validate mod.json
if [ ! -f "mod.json" ]; then
    echo "mod.json not found!"
    exit 1
fi

# Parse mod.json to get mod files
MOD_FILES=$(python3 -c "
import json
with open('mod.json', 'r') as f:
    data = json.load(f)
    for file in data.get('modFiles', []):
        print(file)
")

# Copy mod files
for fileName in $MOD_FILES; do
    if [ "$USE_DEBUG" = true ]; then
        adb push "build/debug/$fileName" "/sdcard/ModData/com.beatgames.beatsaber/Modloader/early_mods/$fileName"
    else
        adb push "build/$fileName" "/sdcard/ModData/com.beatgames.beatsaber/Modloader/early_mods/$fileName"
    fi
done

# Copy late mod files if they exist
LATE_MOD_FILES=$(python3 -c "
import json
with open('mod.json', 'r') as f:
    data = json.load(f)
    for file in data.get('lateModFiles', []):
        print(file)
" 2>/dev/null || true)

for fileName in $LATE_MOD_FILES; do
    if [ "$USE_DEBUG" = true ]; then
        adb push "build/debug/$fileName" "/sdcard/ModData/com.beatgames.beatsaber/Modloader/mods/$fileName"
    else
        adb push "build/$fileName" "/sdcard/ModData/com.beatgames.beatsaber/Modloader/mods/$fileName"
    fi
done

# Restart game
adb shell am force-stop com.beatgames.beatsaber
adb shell am start com.beatgames.beatsaber/com.unity3d.player.UnityPlayerActivity

if [ "$LOG" = true ]; then
    adb logcat -c
    adb logcat QuestHook[TrickSaber*]:* AndroidRuntime:E *:S
fi