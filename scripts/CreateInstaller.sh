#!/bin/sh

# One arguement required
if [ $# -ne 1 ]; then
    echo "Usage: CreateInstaller.sh name"
    echo "where name is the package name"
    exit 0
fi

# Find all directories
currDir=`pwd`
parentDir=${currDir%\/*}
srcDir=$parentDir/src
docsDir=$parentDir/docs
scriptsDir=$parentDir/scripts
releasesDir=$parentDir/releases

# create package contents
mkdir $releasesDir/$1
cp -u $scriptsDir/InstallPackage/Install.sh $releasesDir/$1
cp -u $docsDir/InstallerReadMe.pdf $releasesDir/$1/ReadMe
zip -j -o -u $releasesDir/$1/Einsteinium.pkg $docsDir/ReadMe $docsDir/License $srcDir/Daemon/einsteinium_daemon $srcDir/Engine/einsteinium_engine $srcDir/Launcher/Einsteinium_Launcher $srcDir/Preferences/Einsteinium_Preferences $scriptsDir/InstallPackage/Uninstall.sh
mkdir $releasesDir/$1/EngineSubscriberKit
cp $srcDir/Engine/SubscriberExample/makefile $srcDir/Engine/SubscriberExample/subscriber_example $srcDir/Engine/SubscriberExample/subscriber_example.rsrc $srcDir/Engine/SubscriberExample/SubscriberExample.cpp "$docsDir/Engine Subscriber's Development Guide" $srcDir/Engine/EngineSubscriber.* $releasesDir/$1/EngineSubscriberKit
cd $releasesDir/$1
zip -o -r -u Einsteinium.pkg EngineSubscriberKit
rm -r EngineSubscriberKit

# create final zip file for distribution and cleanup
cd $releasesDir
zip -o -r -u $1.zip $1
rm -r $1

exit 0
