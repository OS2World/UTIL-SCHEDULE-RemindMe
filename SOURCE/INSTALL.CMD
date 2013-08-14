/* REXX installation script for RemindMe */

call RxFuncAdd 'SysCls', 'RexxUtil', 'SysCls'
call RxFuncAdd 'SysCreateObject', 'RexxUtil', 'SysCreateObject'

currentDirectory = directory()
programName =  "\REMINDME.EXE"
programName2 = "\INTERNTL.EXE"

programPath = insert( currentDirectory, programName )
executable = insert( "EXENAME=", programPath )
programPath2 = insert( currentDirectory, programName2 )
executable2 = insert( "EXENAME=", programPath2 )

SysCreateObject("WPProgram","RemindMe - Int'l Version","<WP_DESKTOP>",,
                executable2,"REPLACE" )
SysCreateObject("WPProgram","RemindMe - Version 1.02","<WP_DESKTOP>",,
                executable,"REPLACE" )

call SysCls
say ""
say ""
say "----------------------------------------------------------------"
say " RemindMe - an OS/2 Calendar and Scheduling Application         "
say " Written By Eric A. Wolf - Copr. (C) 1995 - All Rights Reserved "
say "----------------------------------------------------------------"
say ""
say " RemindMe objects successfully created on desktop..."
say ""
say " Start RemindMe by double-clicking  on the program object.  And,"
say " for  future  OS/2 sessions, you  may drag  this  object to your"
say " startup  folder  (found  in the OS/2  System  Folder)  so  that"
say " RemindMe will be executed every time you boot OS/2."
say ""
say "****************************************************************"
say "* This program is distributed as Shareware!  You MUST register *"
say "* this  product if you  continue  to  use it.   Please see the *"
say "* documentation for the complete details.                      *"
say "****************************************************************"
say ""
