#!/bin/sh
# This script makes the catkeys needed before compiling the binaries

make catkeys
cd locales
sort_catkeys.sh en.catkeys
cd ..
