#!/bin/sh

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
else
	notify --group EinsteiniumBootscript.sh --type error --timeout 60 "Could not find the einsteinium_daemon binary.  \
Please verify Einsteinium is installed in /boot/system/packages.  If you have removed Einsteinium please \
delete EinsteiniumBootscript.sh in ~/config/settings/boot/launch"
fi

#launch the engine
if [ -f $sysBinDir/einsteinium_engine ]
then
	open $sysBinDir/einsteinium_engine
elif [ -f $userBinDir/einsteinium_engine ]
then
	open $userBinDir/einsteinium_engine
else
	notify --group EinsteiniumBootscript.sh --type error --timeout 60 "Could not find the einsteinium_engine binary.  \
Please verify Einsteinium is installed in /boot/system/packages.  If you have removed Einsteinium please \
delete EinsteiniumBootscript.sh in ~/config/settings/boot/launch"
fi

exit 0
