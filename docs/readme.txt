Einsteinium Developer's Release 2

Thank you for downloading the developer's release of Einsteinium.  We hope you will try these programs and realize the benefits Einsteinium can provide for everyday Haiku usage.  This release is a functional proof of concept.  Many of the ideas we proposed to impliment using Einsteinium have been completed and can be tested by developers and users.

This mini-manual will attempt to describe in overview what Einsteinium is currently capable of, and what is planned for the near future.  For an in depth description of the project, go to the Einsteinium webpage using the provided bookmark.


What is Einsteinium?

Einsteinium provides smarter monitoring of applications and system services for Haiku.  Currently the two major functions implimented are automatically restarting applications and system services that quit or crash, and gathering statistics on application usage to provide customizable ranked lists of applications.

Currently Einsteinium is divided into three seperate applications:

The Einsteinium Daemon is a background app that watches all the apps which are running.  The daemon can automatically restart an app which quits or crashes, provide a prompt to ask whether the app should be restarted, or allow the app to quit without restarting it.  The Einsteinium Daemon determines the appropriate action to take for each app by the settings you specify.  This is useful for automatically restarting the Deskbar or Tracker if they crash, or restarting apps you like to have always running (such as ICQ, Terminal, or Mail).  It can also monitor system servers to automatically restart any that crash.

The Einsteinium Engine is a second background app which keeps statistics on every applications that runs.  These statistics are available for anyone to use, and can be used to create ranked lists of applications based on certain criteria.  For example, a list of the most recently run apps (the typical Recent Applications list), a list of apps with the longest total running time, a list of apps with the highest number of launches, and more are possible using the statictics gathered by the Einsteinium Engine.

The Einsteinium Preferences app allows you to adjust settings for the both the engine and daemon.  You can also start and stop many system services (servers, Tracker, and Deskbar) from the preferences app.


How to run Einsteinium

For the engine and daemon, open a terminal and type "einsteinium_engine &" and "einsteinium_daemon &".  You may need to cd to the proper directories, and you may wish to run each command in a seperate terminal window if you want to look at the apps' terminal output.  To quit these apps, use the "-q" option (type "einsteinium_engine -q").  If you wish to have these apps automatically start on startup, put the two commands to run the apps in your ~/config/boot/UserBootscript file.
The Einsteinium preferences app is run by double-clicking the file.  Quit the app when you are finished tweaking the Einsteinium settings.  A symlink to the preferences app may be added to /boot/home/config/be/Preferences for easier access.


Einsteinium Engine

The engine maintains statistics on all apps.  The engine watches the be_roster, and receives a message whenever an app is started or quit.  Each app has an associated file located in ~/config/settings/Einsteinium/Applications which has the same name as the application file.  Each statistic is saved as an attribute of this file.  The attributes are updated every time the application is started or quit.  Here is the list of attributes currently used, with their type and description.

EIN:LAUNCHES, B_INT32_TYPE, The number of times the app has been launched
EIN:RANK, B_INT32_TYPE, The overall rank of the app, as calculated from the other statistics
EIN:FIRST_LAUNCH, B_INT32_TYPE, date of the very first launch (as a time_t)
EIN:LAST_LAUNCH, B_INT32_TYPE, date of most recent launch (as a time_t)
EIN:LAST_INTERVAL, B_INT32_TYPE, interval between the previous two launches (in seconds)
EIN:TOTAL_RUN_TIME, B_INT32_TYPE, the total time in seconds that the application has run since the first launch
EIN:APP_SIG, B_STRING_TYPE, the application signature
EIN:APP_FILENAME, B_STRING_TYPE, the name of the application file
EIN:APP_PATH, B_STRING_TYPE, the path where the application is located
EIN:SESSION, B_INT32_TYPE, the session number of the Einsteinium engine instance that last updated the application's start or quit time

Each attribute file is saved as type "document/x-E-app-attr", so you can create an entry for these files in FileTypes preferences, and add the above attributes as "Extra Attributes" to display when viewing /boot/home/config/settings/Einsteinium/Applications in a Tracker window.

The rank is a weighted combination of all of the other numerical statistics.  The weights of each statistic used to calculate the rank is changable by the user using the Einsteinium preferences application.

Obviously, one result of these statistics will be to create a truly intelligent system for calculating the most frequently, most recently, most constantly, most whatever apps.  This system is much more desirable than Haiku's current implimentation of only the most recent apps.  Also, all the statistic for each file are saved as attributes, so are freely available to anyone who wishes to use them.  At this point, there is only one "rank" calculated by the engine, but this may change with time.

The engine has some functions for returning a list of the highest ranking apps.   The first function will simply print the list out to the terminal.  Use the option --apps in a terminal to get the printed list.  You can also request the list from other applications via BMessages.  To do this create a BMessage with the cmmand E_REPLY_RECENT_APPS, as defined in the engine_constants.h file.  If you wish to receive a certain number of apps, add an INT8 called "count" to the message.  If "count" is not  specified you will the entire list of apps.  Then just send this message to the engine.  When the message is received, a list of the "count" highest ranking apps will be added to a reply message.  For each app the message will contain the data "app_signature" (String), "app_path" (String), and "rank" (Int32).  The data for the highest ranking app will be the first element in each array (index 0), and each next highest ranking app will be at the next index, down to the lowest ranking app for as many apps as you specified in "count".  This message will be sent back to your app as a reply with the command E_RECENT_APPS_REPLY, again as defined in engine_constants.h.

As more statistics are added to each files' attributes, and the rank calculation algorithms are improved, the engine will be able to provide lists ordered to the needs of whatever type of list you want.  Filters will also be added so you can exclude certain programs from appearing in the lists (ie. background apps).  Also, the engine may eventually be able to track files, folders, URLs, etc, for even more everyday usage stats.


Einsteinium Daemon

The daemon keeps track of the currently running apps, and makes sure that everything you want to stay running does (such as servers, Deskbar, Tracker, mail client, etc).  Using the Einsteinium preferences app you can specify a list of apps you want the daemon to keep track of.  You can setup each app independantly to either automatically restart, or prompt you first before restarting.

This app doesn't have many features yet, but I already find it very useful.  There is also lots that can be done with this program.  One idea I have is to use the Preferences app as an interface to the daemon for restarting the individual servers on demand.  Also the Deskbar and Tracker can be restarted in a similar manner.  Features from the already available DeskAttendant by Kris Finkenbinder can also be incorperated to keep all of those deskbar tray objects in place after a deskbar restart/crash.


Einsteinium Preferences

The Einsteinium peferences app allows the user to change settings which effect the way the engine and daemon monitor the system.  Currently the user can modify a list of apps for the daemon to monitor, under the "App Relaunch" secction.  The "System Status" section allows starting and stoping of several services, including servers, Tracker and Deskbar.  Under the "App Rankings" section a system for adjusting the weights for statistics used in calculating each application rank has been implimented.  All settings are saved into seperate files for the engine and daemon in a standard XML format.  The settings in these files are read using the libxml2 library.

As more features of the engine and daemon are added, any possible adjustable settings will be added to the preferences app.


Final Notes

Please understand that this is a work in progress.  Major restructuring of the apps is always a possibility.  I try my best to program things right the first time, but it's not always easy or even possible.

We are looking for some good help.  This is a fairly large project, too much for one person to finish in reasonable time.  If you would like to help out, please contact Kris Finkenbinder.  We have an account at SourceForge, so we can start using CVS at any time.  If you have any ideas that you would like to see added to Einsteinium, please let us know.  We want this project to be something people find useful.

Thanks,
-The Einsteinium Project Members (Kris Finkenbinder, Brian Hill, Mikael Eiman)