# CNC_CURRENT_TRACKER Service

The source for the cnc_current tracking service that runs on HPNEW1. It logs the network call from the NCD modem to the Microsoft SQL Server.

It is built using cmake and requires ODBC to be installed and a DSN to be setup in the revelant 64 or 32 bit version. It also supports logging to a MariaDB sql database, using the MariaDB C++ Connector.

## Prerun Requirements

The applications requires a 'settings.ini' file, it must be layed out as follows;

```
[Database]
Host=""
Port=
Username=""
Password=""
Schema=""
DNS=""
```

Host is the Databases host address, 
Port is the port the database is running on, 
Schema is the name of the target database. 
Username and Password are the credentials needed to access target database, 
and finally DNS is the defined namespace used by ODBC.

## Running the app

The .exe needs to be run in a terminal with either the 'install' or 'remove' arguments.

```
CNC_Current_Tracker.exe install
```

This initialises and installs the service, as well as starts it if the autostart fails.

```
CNC_Current_Tracker.exe remove
```

This uninstalls the service and prevents it from starting back up. 
Restarting the device may be required to complete the process, alternatively killing the cnc_current_tracker process will acomplish the same thing.