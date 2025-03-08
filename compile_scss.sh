#!/bin/bash
COMPONENTS=("header" "control-panel" "meters") # CSS Files to compile

if ! command -v sass &> /dev/null; then
    echo "Sass not found, installing..."
    npm install -g sass
fi

mkdir -p src/resources/scss
mkdir -p src/resources/css

# Process each component
for component in "${COMPONENTS[@]}"; do
    # Check if SCSS exists, create if not
    if [ ! -f "src/resources/scss/${component}.scss" ]; then
        echo "Creating empty ${component}.scss file..."
        touch "src/resources/scss/${component}.scss"
    fi
    
    # Compile SCSS to CSS
    sass "src/resources/scss/${component}.scss" "src/resources/css/${component}.css" --style compressed
done

# Check if all files were compiled successfully
all_compiled=true
for component in "${COMPONENTS[@]}"; do
    if [ ! -f "src/resources/css/${component}.css" ]; then
        all_compiled=false
        echo "ERROR: Failed to compile ${component}.css"
    fi
done

if [ "$all_compiled" = true ]; then
    echo "Successfully compiled all CSS files"
else
    echo "ERROR: Failed to compile some CSS files"
    exit 1
fi