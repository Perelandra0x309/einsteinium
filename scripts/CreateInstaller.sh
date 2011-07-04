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
cp -u $docsDir/InstallerReadMe.txt $releasesDir/$1/ReadMe.txt
zip -j -o -u $releasesDir/$1/Einsteinium.pkg $docsDir/ReadMe.txt $docsDir/License.txt $srcDir/Daemon/einsteinium_daemon $srcDir/Engine/einsteinium_engine $srcDir/Launcher/Einsteinium_Launcher $srcDir/Preferences/Einsteinium_Preferences $scriptsDir/InstallPackage/Uninstall.sh

# create final zip file for distribution and cleanup
cd $releasesDir
zip -o -r -u $1.zip $1
rm -r $1

exit 0
