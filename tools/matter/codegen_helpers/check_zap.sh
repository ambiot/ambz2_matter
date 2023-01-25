#!/bin/bash

DIR=$(dirname "$0")
ZIPFILE=$DIR/zap-linux.zip
ZAPDIR=$DIR/zap

echo "Downloading ZAP"

if [ ! -f "$ZIPFILE" ]; then
    curl -L https://github.com/project-chip/zap/releases/download/v2023.01.18-nightly/zap-linux.zip -o "$ZIPFILE"
else
    echo "$ZIPFILE already exists"
fi

echo "Unzipping..."
if [ ! -d "$ZAPDIR" ]; then
    unzip "$ZIPFILE" -d "$ZAPDIR"
else
    echo "$ZAPDIR already exists"
fi
