#!/bin/bash

if ! command -v sass &> /dev/null; then
    echo "Sass not found, installing..."
    npm install -g sass
fi

mkdir -p src/resources/scss

# Compile the single layout SCSS file
if [ -f "src/resources/layout.scss" ]; then
    echo "Compiling layout.scss..."
    sass "src/resources/layout.scss" "src/resources/layout.css" --style compressed
else
    echo "ERROR: src/resources/layout.scss not found!"
    exit 1
fi

# Check if compilation was successful
if [ -f "src/resources/layout.css" ]; then
    echo "Successfully compiled layout CSS file"
else
    echo "ERROR: Failed to compile layout.css"
    exit 1
fi