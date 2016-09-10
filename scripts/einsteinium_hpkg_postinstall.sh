#!/bin/sh
appDir="/boot/system/apps/Einsteinium"
binDir="/boot/system/bin"

# Start Einsteinium now
$binDir/einsteinium_engine &
sleep 1
$binDir/einsteinium_daemon &
open $appDir
open $appDir/ReadMe

# Notification to user
alert --modal "Einsteinium has been successfully installed.  You may uninstall Einsteinium by removing the einsteinium hpkg file from /boot/system/packages." "OK"

exit 0
