#!/bin/bash

CURRENT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

# Run `./build.sh --build-release`
"$CURRENT_DIR/build.sh" --build-release

# Create a lib directory
mkdir -p lib

# Copy the prebuilt libraries to the lib directory
cp -r ./prebuilds/ ./lib/

# Copy the package.json, README.md, LICENSE and binding.gyp to the lib directory
cp package.json ./lib
cp README.md ./lib
cp LICENSE ./lib
cp binding.gyp ./lib

# Remove any line with `file:/` in the package.json
sed -i '/file:/d' ./lib/package.json

# Add `"install": "node ./scripts/install.js",` to the scripts attribute in the package.json
sed -i 's#"scripts": {#"scripts": {\n    "install": "node ./scripts/install.js",#' ./lib/package.json
