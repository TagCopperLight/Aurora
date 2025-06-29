#!/bin/bash

# Create txt/ directory if it doesn't exist
mkdir -p txt

# Process both src and include recursively
for dir in aurora_app aurora_engine debug_example; do
  find "$dir" -type f | while read -r file; do
    # Get the relative path and replace slashes with underscores
    relpath="${file}"
    relpath_underscored="${relpath//\//_}"
    # Copy to txt/ with .txt appended
    cp "$file" "txt/${relpath_underscored}.txt"
  done
done
