# KD

Debugger extension for the Debugging Tools for Windows (WinDbg, KD, CDB, NTSD).

## Commands

* st   - Displays system service table
* idt  - Displays interrupt descriptor table

## How to Debug

1. Open the project in Visual Studio.
2. In Solution Explorer, open the shortcut menu for your project and then choose Properties.
3. In the Property Pages dialog box, open the Configuration drop-down list and then select Debug.
4. In the Property Pages dialog box, open the Platform drop-down list and then select Win32 or x64.
5. In the left pane of the dialog box, expand Configuration Properties and then select Debugging.
6. In the right pane, select Command and then set the path to the WinDbg.exe:
C:\Program Files (x86)\Windows Kits\8.1\Debuggers\x64\windbg.exe or
C:\Program Files (x86)\Windows Kits\8.1\Debuggers\x86\windbg.exe
7. In the right pane, select Command Arguments and then set the command line arguments:
-c ".prefer_dml 1;.load kd" -z "D:\CrashDumps\livekd.dmp"
8. In the right pane, select Environment and then set the PATH environment variable: PATH=$(OutDir);%PATH%
9. Choose the OK button.
10. Press F5 or use Debug > Start Debugging to debug.