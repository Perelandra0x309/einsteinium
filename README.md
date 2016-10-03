# einsteinium
Einsteinium for Haiku

What is Einsteinium?
Einsteinium provides smarter monitoring of applications and system services for Haiku. The two
major functions provided are:
• Restarting applications and system services that quit or crash
• Gathering data on application usage to provide customizable ranked lists of applications

Einsteinium is divided into four applications:

Einsteinium Daemon:
This is a background application (daemon) that watches all the applications which are running. The
daemon detects when an application quits or crashes, and can perform one of three options;
automatically restart the app, provide a prompt to ask you whether the app should be restarted, or allow
the app to quit and do nothing. The Einsteinium Daemon determines the appropriate action to take for
each app by the settings you specify. This is useful for automatically restarting the Deskbar or Tracker
if they crash, or restarting apps you like to have always running (such as a chat client, Terminal, or
Mail). It can also monitor system servers to automatically restart any that crash.

Einsteinium Engine:
The Engine is another background application that captures data on every application that runs. Using
this data, the Engine will create statistics on application usage (for example number of times an
application has been launched, total running time, etc). These statistics are used to create ranked lists
of applications based on certain criteria. For example, the Engine can create a list of the most recently
run apps (the typical Recent Applications list), a list of apps with the longest total running time, a list of
apps with the highest number of launches, and more are possible using the statistics gathered by the
Einsteinium Engine.

Einsteinium Launcher:
The Launcher is a simple application and file launcher, and is a working example of how to use the
Engine to create application lists. The Launcher displays a list of applications, files, folders and
queries which have been recently opened. The Launcher queries the Engine to create the list of
applications ranked by specified criteria that you define. This allows you to have an application
launcher with applications ordered the way you want them.

Einsteinium Preferences:
This is the preferences application that allows you to adjust settings for the Engine, Daemon and
Launcher.

-------------------------------------------------
Einsteinium was first envisioned and started development in 1999 for BeOS 4.5 by Kris Finkenbinder,
Mikeal Eiman and myself. We released a simple development version that demonstrated the Daemon
restarting applications from a user defined list, and the Engine tracking the number of times an
application had been launched. Since then BeOS has polymorphed several times, and development on
Einsteinium stalled as the future of BeOS was uncertain. Then Haiku became the beacon of hope for
the continuation BeOS unrestrained by commercial interests. Development of Einsteinium was
resumed by myself and has progressed as time and life allowed. Now I am happy to release a full
working version of Einsteinium that fulfills our vision from so many years ago.
