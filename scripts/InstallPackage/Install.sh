#!/bin/sh
appDir="/boot/apps/Einsteinium"
binDir="/boot/common/bin"
launchDir="/boot/home/config/boot/launch"
beAppsDir="/boot/home/config/be/Applications"
prefsDir="/boot/home/config/be/Preferences"
currDir="$(dirname "$0")"

cancelInstall()
{
    alert --stop --modal "Einsteinium install canceled."
    exit 0
}

# Give a chance to cancel install
alert --modal "Einsteinium Installer: This install script will install and configure Einsteinium." "Cancel" "Next"
if [ $? -eq 0 ]; then
    cancelInstall
fi

# Create Einsteinium app folder, copy package into it and create symlinks
if [ ! -d $appDir ]; then
    mkdir $appDir
fi
unzip -u $currDir/Einsteinium.pkg -d $appDir
if [ "$?" -ne "0" ]; then
    alert --stop --modal "There was an error extracting the Einsteinium package.  Installation cannot continue."
    exit 1
fi
ln -s -f $appDir/ein* -t $binDir
ln -s -f $appDir/Einsteinium_Launcher "$beAppsDir/Einsteinium Launcher"
ln -s -f $appDir/Einsteinium_Preferences $prefsDir/Einsteinium

# Automatic startup option
alert --modal "Do you want Einsteinium to start automatically when Haiku boots up?  Symlinks to Einsteinium will be created in the \"$launchDir\" folder and can be removed later to prevent automatic startup." "No" "Yes"
if [ $? -eq 1 ]; then
    #Create boot launch links for Einsteinium
    ln -s -f $appDir/einsteinium_engine -t $launchDir
    ln -s -f $appDir/einsteinium_daemon -t $launchDir
fi

# Start Einsteinium now
$appDir/einsteinium_engine &
sleep 1
$appDir/einsteinium_daemon &
open $appDir
open $appDir/ReadMe

# Notification to user
alert --modal "Einsteinium has been successfully installed.  You may uninstall Einsteinium by running the Uninstall.sh script." "OK"

exit 0
