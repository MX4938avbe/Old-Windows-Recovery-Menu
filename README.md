<img width="1024" height="768" alt="image" src="https://github.com/user-attachments/assets/6b91ce5e-a9a1-47be-88f2-2742bd638c2a" />


# Experiencing Old Look of Windows Recovery Screen for Windows 11

This program aims to provides classical look of Windows Recovery Menu before Windows 7.

## Compatibility

Tested on Windows 11 24H2 and 25H2 only. You should use this application **carefully** in Windows 8 and Windows 10.

## How to use

### For standalone users:
You may directly open this file by any methods in Windows PE, as this application uses multi-threaded runtime library.
Apart from that, you can freely create a winpeshl.ini file to open automatically upon booting.

### For users using installer: (Only applies to version 1.1.0 !!!)
Using the typical way to install is fine.
To uninstall this software, you need to perform these procedures:

1. Go to control panel and perform an uninstallation.
2. Find a Windows ISO image that matches your current version, then use dism command to extract system files.
3. Copy the Winre.wim file in (Your extracted folder)\Windows\System32\Recovery folder. This file should have a size around 500~1000 MB.
4. Paste and overwrite the original Winre.wim file in C:\Windows\System32\Recovery.
5. Open a commaand prompt as administrator and type reagentc /disable, then reagentc /enable.

A fully functional uninstaller will provisionally added in version 1.1.1 to ease these redundant steps.


## Functions lacking compared to Metro-UI style recovery screen
* Use a device
* Load Drivers

## List of Updates
Version 1.1.0: Installer available; Fixed system restore no target OS & window centering issues.

Version 1.0.2: Added language/keyboard picker and OS options menu.

Version 1.0.1: Added post-Windows 8 recovery options.

Version 1.0.0: Replica of Windows 7 recovery menu.
