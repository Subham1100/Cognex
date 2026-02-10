#!/bin/bash
set -e

REPO_URL="https://github.com/Subham1100/Cognex.git"
TMP_DIR="/tmp/cognex-install"

echo "Installing Cognex..."

rm -rf "$TMP_DIR"
git clone "$REPO_URL" "$TMP_DIR"
cd "$TMP_DIR"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build

echo "Cognex installed"
