#!/bin/sh
# This script builds the binaries and binds the catkeys

for f in Engine; do
	pushd $f
	make $jobArgs OBJ_DIR=_${f:0:1}objects
	popd
done
for f in Daemon Launcher Preferences; do
	pushd $f
	make $jobArgs OBJ_DIR=_${f:0:1}objects
	make bindcatalogs OBJ_DIR=_${f:0:1}objects
	popd
done
