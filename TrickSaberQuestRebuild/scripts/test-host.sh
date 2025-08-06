#!/bin/bash

# Redirect to the isolated test script
exec "$(dirname "$0")/test-host-isolated.sh" "$@"