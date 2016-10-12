#!/bin/sh

#search for installed Einsteinium package
sysAppsDir=`finddir B_SYSTEM_APPS_DIRECTORY`
userAppsDir=`finddir B_USER_APPS_DIRECTORY`
if [  ! -d $sysAppsDir/Einsteinium  -a  ! -d $userAppsDir/Einsteinium  ]
then
	#Einsteinium not installed, remove this script
	rm -f /boot/home/config/settings/boot/launch/EinsteiniumBootscript.sh
	exit 0
fi

#find bin paths
sysBinDir=`finddir B_SYSTEM_BIN_DIRECTORY`
userBinDir=`finddir B_USER_BIN_DIRECTORY`

#launch the daemon
if [ -f $sysBinDir/einsteinium_daemon ]
then
	open $sysBinDir/einsteinium_daemon
	waitfor -m application/x-vnd.Einsteinium_Daemon
elif [ -f $userBinDir/einsteinium_daemon ]
then
	open $userBinDir/einsteinium_daemon
	waitfor -m application/x-vnd.Einsteinium_Daemon
fi

#launch the engine
if [ -f $sysBinDir/einsteinium_engine ]
then
	open $sysBinDir/einsteinium_engine
elif [ -f $userBinDir/einsteinium_engine ]
then
	open $userBinDir/einsteinium_engine
fi

#create Launcher Deskbar menu
if [ -d $sysAppsDir/Einsteinium ]
then
	$sysAppsDir/Einsteinium/Einsteinium\ Launcher --prepdeskbar
elif [ -d $userAppsDir/Einsteinium ]
then
	$userAppsDir/Einsteinium/Einsteinium\ Launcher --prepdeskbar
fi

exit 0
