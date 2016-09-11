#!/bin/sh

# Start Einsteinium now
open application/x-vnd.Einsteinium_Engine &
sleep 1
open application/x-vnd.Einsteinium_Daemon &
desklink "cmd=Remove replicant:desklink --remove=Einsteinium_Launcher" application/x-vnd.Einsteinium_Launcher

# Notification to user
notify --group Einsteinium --onClickApp application/x-vnd.Einsteinium_Launcher --timeout 60 --icon \
"/boot/home/config/settings/Einsteinium/notify_icon" \
"Thank you for installing Einsteinium.  Click on this message to open the Launcher application."

# Open ReadMe file
if test -f /boot/system/apps/Einsteinium/ReadMe
then
	open /boot/system/apps/Einsteinium/ReadMe
#	notify --group Einsteinium --onClickFile /boot/system/apps/Einsteinium/ReadMe --timeout 60 --icon \
#"/boot/home/config/settings/Einsteinium/notify_icon" \
#"Click on this message to open the ReadMe file."
elif test -f /boot/home/config/apps/Einsteinium/ReadMe
then
	open /boot/home/config/apps/Einsteinium/ReadMe
#	notify --group Einsteinium --onClickFile "/boot/home/config/apps/Einsteinium/ReadMe" --timeout 60 --icon \
#"/boot/home/config/settings/Einsteinium/notify_icon" \
#"Click on this message to open the ReadMe file."
fi

exit 0
