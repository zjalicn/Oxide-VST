#!/bin/bash
if ! command -v sass &> /dev/null; then
    echo "Sass not found, installing..."
    npm install -g sass
fi

# Create directory & file if they don't exist
mkdir -p src/resources/scss &&

if [ ! -f src/resources/scss/global.scss ]; then
    echo "Creating empty global.scss file..."
    touch src/resources/scss/global.scss
fi

# Compile
sass src/resources/scss/global.scss src/resources/global.css --style compressed

# Check
if [ -f "src/resources/global.css" ]; then
    echo "Successfully created global.css"
else
    echo "ERROR: Failed to create global.css"
    exit 1
fi