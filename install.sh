#!/bin/sh
set -e

REPO_URL="https://github.com/Subham1100/Cognex.git"
TMP_DIR="/tmp/cognex-install"

# Use current directory as install prefix
INSTALL_PREFIX="$(pwd)"

echo "Installing Cognex into $INSTALL_PREFIX ..."

# Clean temp workspace
rm -rf "$TMP_DIR"
git clone "$REPO_URL" "$TMP_DIR"
cd "$TMP_DIR"

# Configure & build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install into current directory
cmake --install build --prefix "$INSTALL_PREFIX"

echo "Cognex installed successfully!"
echo "Run with: .bin/cognex"



