#!/bin/sh
# This script makes the catkeys needed before compiling the binaries

for f in Daemon Launcher Preferences; do
	pushd $f
	make catkeys
	pushd locales
	sort_catkeys.sh en.catkeys
	popd
	popd
done
