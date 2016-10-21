#!/bin/sh

# Start Einsteinium now
open application/x-vnd.Einsteinium_Engine &
waitfor -m application/x-vnd.Einsteinium_Engine
open application/x-vnd.Einsteinium_Daemon &

#find app and data paths
appDir=""
dataDir=""
sysAppsDir=`finddir B_SYSTEM_APPS_DIRECTORY`
userAppsDir=`finddir B_USER_APPS_DIRECTORY`

if [ -d $sysAppsDir/Einsteinium ]
then
	appDir="$sysAppsDir/Einsteinium"
	dataDir=`finddir B_SYSTEM_DATA_DIRECTORY`
elif [ -d $userAppsDir/Einsteinium ]
then
	appDir="$userAppsDir/Einsteinium"
	dataDir=`finddir B_USER_DATA_DIRECTORY`
else
	notify --group Einsteinium --type error --timeout 60 "There may have been a problem installing Einsteinium.  \
The Launcher application could not be found."
	exit 0
fi

# Copy bootscript to launch directory
userBootDir=`finddir B_USER_BOOT_DIRECTORY`
cp -af $dataDir/Einsteinium/EinsteiniumBootscript.sh $userBootDir/launch

# Notification to user
notify --group Einsteinium --onClickApp application/x-vnd.Einsteinium_Launcher --timeout 60 --icon "$dataDir/Einsteinium/notify_icon" \
"Thank you for installing Einsteinium.  Click on this message to open the Launcher application."

# Open ReadMe file
if [ -f $appDir/ReadMe.pdf ]
then
	open $appDir/ReadMe.pdf
fi

exit 0
