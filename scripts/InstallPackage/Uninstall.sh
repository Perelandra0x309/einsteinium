#!/bin/sh
appDir="/boot/apps/Einsteinium"
binDir="/boot/common/bin"
launchDir="/boot/home/config/boot/launch"
beAppsDir="/boot/home/config/be/Applications"
prefsDir="/boot/home/config/be/Preferences"
settingsDir="/boot/home/config/settings/Einsteinium"
currDir="$(dirname "$0")"

cancelUninstall()
{
    alert --stop --modal "Einsteinium uninstall canceled."
    exit 0
}

# Give a chance to cancel uninstall
alert --modal "Einsteinium Uninstaller: This script will uninstall Einsteinium." "Cancel" "Next"
if [ $? -eq 0 ]; then
    cancelUninstall
fi

# Quit any running instances of Einsteinium
# $currDir/Einsteinium_Launcher -q
einsteinium_daemon -q
einsteinium_engine -q

# Remove files
rm -f $launchDir/einsteinium_daemon
rm -f $launchDir/einsteinium_engine
rm -f $launchDir/Einsteinium_Launcher
rm -f $binDir/einsteinium_daemon
rm -f $binDir/einsteinium_engine
rm -f "$beAppsDir/Einsteinium Launcher"
rm -f $prefsDir/Einsteinium
rm -f -r $currDir

# Ask to delete settings
alert --modal "Do you want to delete all Einsteinium settings files and databases?" "Yes" "No"
if [ $? -eq 0 ]; then
    rm -f -r $settingsDir
fi

# Notification to user
alert --modal "Einsteinium has been successfully uninstalled." "OK"

exit 0
