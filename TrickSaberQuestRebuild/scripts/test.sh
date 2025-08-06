#!/bin/bash

# Legacy test script - redirects to host-native tests
echo -e "\033[33mNote: Using host-native tests (Android cross-compilation tests not supported)\033[0m"
exec "$(dirname "$0")/test-host.sh" "$@"