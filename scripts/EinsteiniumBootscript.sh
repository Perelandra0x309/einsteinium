#!/bin/sh

#search for installed Einsteinium package
sysAppsDir=`finddir B_SYSTEM_APPS_DIRECTORY`
userAppsDir=`finddir B_USER_APPS_DIRECTORY`
userSettingsDir=`finddir B_USER_SETTINGS_DIRECTORY`
if [  ! -d $sysAppsDir/Einsteinium  -a  ! -d $userAppsDir/Einsteinium  ]
then
	#Einsteinium not installed, remove this script
	rm -f $userSettingsDir/boot/launch/EinsteiniumBootscript.sh
	exit 0
fi

#get daemon and engine boot settings
launchD='True'
launchE='True'
launchSettings=$userSettingsDir/Einsteinium/boot
attrResult=`catattr -d 'launch_daemon' $launchSettings`
if [ $? -eq 0 ]
then
	launchD=$attrResult
fi
attrResult=`catattr -d 'launch_engine' $launchSettings`
if [ $? -eq 0 ]
then
	launchE=$attrResult
fi

#find bin paths
sysBinDir=`finddir B_SYSTEM_BIN_DIRECTORY`
userBinDir=`finddir B_USER_BIN_DIRECTORY`

#launch the daemon
if [ -f $sysBinDir/einsteinium_daemon -a $launchD == 'True' ]
then
	open $sysBinDir/einsteinium_daemon
	waitfor -m application/x-vnd.Einsteinium_Daemon
elif [ -f $userBinDir/einsteinium_daemon -a $launchD == 'True' ]
then
	open $userBinDir/einsteinium_daemon
	waitfor -m application/x-vnd.Einsteinium_Daemon
fi

#launch the engine
if [ -f $sysBinDir/einsteinium_engine -a $launchE == 'True' ]
then
	open $sysBinDir/einsteinium_engine
elif [ -f $userBinDir/einsteinium_engine -a $launchE == 'True' ]
then
	open $userBinDir/einsteinium_engine
fi

exit 0
