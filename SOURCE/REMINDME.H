////////////////////////////////////////////////////////////////////////////////
//
//                              RemindMe for OS/2
//
//                           Written By Eric A. Wolf
//                  Copyright (C) 1995 - All Rights Reserved
//
// This source code may be used for reference ONLY!  It is provided AS-IS and no
// guarantees are made as to its utility, functionality or correctness.  It is
// provided solely as a guide to aid aspiring OS/2 2.x Presentation Manager
// programmers in developing their own PM applications.  No modifications are
// to be made to this code for re-release as a same or different product.  This
// code must be distributed (in its original entirety) with the executable
// portion of this product.
//
//          -- Please register this shareware product for $10 today --
//                          See documentation for details
//
// Written using Borland C++ for OS/2, version 1.0
//
////////////////////////////////////////////////////////////////////////////////
/* ALIGNMENT CODES FOR CALENDAR DATE */
#define LEFT            1
#define CENTER          2
#define RIGHT           3

/* CODES FOR REMINDER TYPE */
#define VISUALLY	4
#define AUDIBLY	        5
#define BOTH	        6

/* RESOURCE ID CODES */
#define MENUBAR         2
#define CALDRAWOPTIONS	3
#define ABOUT1          4
#define PICKANEVENT	5
#define TODOLISTOPTIONS	6
#define EVENTDLG	7
#define TODOLISTICON    8
#define CHOOSECOLOR	9
#define REMINDER_DIALOG	10
#define CALCOLORDLG	11
#define REMINDOPTIONS	12
#define JUMPTODATE      13
#define PRINTDIALOG	14

/* DIALOG CONTROL ID CODES */
#define LEFTALIGN	200
#define CENTERALIGN	201
#define RIGHTALIGN	202
#define USE3DEFFECT	203
#define LINESBETWEEN	204
#define QUICKREMINDER	205
#define DRAWDAYBORDER	206
#define SAMPLECAL	207
#define DAYSOFWEEK	208
#define ABBREVIATEMONTH	209
#define HIGHLIGHTTODAY	210
#define ABBREVIATEDAYS	211
#define CURITEMLIST	212
#define REMINDOLD       213
#define HOUR	        214
#define MIN	        215
#define AM	        216
#define PM	        217
#define EVENTTITLE	218
#define DAY	        219
#define MONTH	        220
#define YEAR	        221
#define NEWEVENT	222
#define DELETEEVENT	223
#define MODIFYEVENT	224
#define FLASHREMINDER	225
#define BUTTON1	        226
#define BUTTON2	        227
#define MINBEFORE	228
#define DAYSBEFORE	229
#define MINBEFORELBL	230
#define DAYSBEFORELBL	231
#define BCOLOR	        232
#define FCOLOR	        233
#define SHOWTODOLST	234
#define ITEMDONE	235
#define SHOWASTODO	236
#define DAYSADVANCE	237
#define MONTHLABEL	238
#define YEARLABEL	239
#define EVERYMONTH	240
#define EVERYYEAR	241
#define SELECTSOUNDFILE	242
#define COLORBOX	243
#define SOUNDFILE	244
#define MARKDONE	245
#define REVISEEVENT	246
#define ERASEEVENT	247
#define SNOOZE	        248
#define REMINDERTEXT	249
#define QUICKNOTDONE	250
#define QUICKDONE	251
#define INCOMPLETETXT	252
#define INCOMPLETEBKG	253
#define COMPLETETXT	254
#define COMPLETEBKG	255
#define TEXTCOLOR	256
#define LIGHTCOLOR	257
#define MIDCOLOR	258
#define DARKCOLOR	259
#define CHANGECOLORS	260
#define SYNCCOLORS      261
#define NUMSNOOZE	262
#define NUMREPLAY	263
#define NUMADVANCE	264
#define SNOOZE2	        265
#define LAUNCHRECORDER	266
#define DATETEXT	267
#define ASKBEFOREDELETE	268
#define NOPREFIX	269
#define DATEOFEVENT	270
#define TIMEOFEVENT	271
#define DATEANDTIMEOFEVENT	272
#define DISPLAYLASTNEXT	273
#define OLDTEXTCOLOR	274
#define OLDBACKCOLOR	275
#define SHOWCLOCK	276
#define RESCHEDTIME	277
#define PRTTODAY        278
#define PRTMONTH	279
#define PRINTTODO	280
#define PRTWEEK	        281
#define WIDTH	        282
#define PRINTCALENDAR	283
#define INCLUDEDONE	284

/* MENU BAR ID CODES */
#define EXIT            109
#define FILEMENU        110
#define CALOPTIONS      112
#define SHOWREMINDERS   113
#define SAVECAL         114
#define SAVECALAS       115
#define NEWCAL          116
#define OPENCAL         117
#define TODOOPTIONS     118
#define REMINDEROPTIONS 119
#define JUMPTO          120
#define TESTFUNCTION    121
#define PRINT           122

/* TEXT CODES */
#define FILENAME      "DEFAULT.CAL"
#define ClassName     "RemindMe"
#define ToDoClassName "ToDoListClass"

/* CONFIGURATION FILE TEXT FIELDS */
#define NUMREMINDERS     "NumReminders"
#define DAYALIGNMENT     "DayAlign"
#define CALBORDERLINES   "DrwCalBdrLines"
#define CALDAYBORDER     "DrwBdrAroundDay"
#define CALBUTTONSHADOWS "Drw3DCal"
#define SHOWTODOLIST     "ShwToDo"
#define USEQUICKREMINDER "UseQckReminder"
#define CALSHOWCONTROLS  "ShwCalCtrls"
#define TODOSHOWCONTROLS "ShwToDoCtrls"
#define LABELDAYSOFWEEK  "LabelDays"
#define ABBREVWEEKDAYS   "AbbrvWeekDays"
#define ABBREVMONTH      "AbbrvMonth"
#define HIGHLIGHTTDAY    "HighlightToday"
#define REMINDER         "Event"
#define TODOLISTPOS      "ToDoWindow"
#define CALPOS           "CalWindow"
#define TODOFCOLOR       "ToDoFColor"
#define TODOBCOLOR       "ToDoBColor"
#define TODOFONT         "ToDoFont"
#define CALFONT          "CalFont"
#define CALHIGHTEXT      "Clr_HighTxt"
#define CALDAYCOLOR      "Clr_DayColor"
#define CALHIGHLIGHT     "Clr_Highlight"
#define CALDONEHIGHLIGHT "Clr_DoneHighlight"
#define CALDONETEXT      "Clr_DoneText"
#define CALTEXT          "Clr_Text"
#define CALREMINDDONE    "Clr_RemindDone"
#define CALBUTTONLIGHT   "Clr_Light"
#define CALBUTTONMID     "Clr_Middle"
#define CALBUTTONDARK    "Clr_Dark"
#define REMINDIFDONE     "RemindIfDone"
#define ASKBEFOREDEL     "AskBeforeDelete"
#define SOUNDREPLAY      "ReplaySound"
#define SNOOZETIME       "SnoozeTime"
#define DAYSINADVANCE    "DaysInAdvance"
#define TODOMINSTATE     "CalMinimize"
#define CALMINSTATE      "ToDoMinimize"
#define TODODISPLAY      "ToDoDisplayPrefix"
#define SHOWALLDAYS      "ShowAllDays"
#define OLDMONTHTEXT     "OldMonthText"
#define OLDMONTHBACK     "OldMonthBack"
#define SHOWTHECLOCK     "ShowClock"
#define PRINTWIDTH       "PrintPageWidth"
#define PRINTTODOL       "PrintToDo"
#define PRINTCALEN       "PrintCal"

/******************************************************************************/
/*                      LANGUAGE INDEPENDENT TEXT STRINGS                     */
/******************************************************************************/
#define MSG_NOFONT    "none"
#define NO_ITEM       "----"
#define CHIMEWAV      "DEFAULT.WAV"
#define BACKUPFILE    "BACKUP.CAL"
#define LAUNCHFILE    "STARTPRG.CMD"


/******************************************************************************/
/******************************************************************************/
/*                       LANGUAGE DEPENDENT TEXT STRINGS                      */
/******************************************************************************/
/******************************************************************************/
#define VERSION       "Version 1.02"
#define MSG_TODOTTL   "To Do List"
#define MSG_OK        "OK"
#define MSG_CANCEL    "Cancel"
#define MSG_DONE      "Done"
#define MSG_ALL       "All"
#define NEWITEMTEXT   "New item"
#define MSG_REGISTER  "Register RemindMe for OS/2 (Shareware!)  See documentation (REMINDME.TXT) for complete details."
#define MSG_CHRISTMAS "Christmas"
#define MSG_NEWYEAR   "New Years"
#define MSG_ERICBDAY  "Eric's Birthday"

#define CURDATESTR1   "%B %d on every year"
#define CURDATESTR2   "Day #%d of every month in %Y"
#define CURDATESTR3   "Day #%d of every month of every year"

#define MSG001        "Too many run-time parameters specified!  You may only include a single calendar file to open."
#define MSG002        "Error opening specified calendar file!  Using local default calendar file."
#define MSG003        "Error opening default calendar file!\n\nThis may be because you are running this application for the first time.  The built-in defaults will be used."
#define MSG004        "RemindMe for OS/2 - Error!"
#define MSG005        "Error in calendar header file.  An incorrect version number (%g) was found.  That calendar file will be ignored and built-in defaults will be used instead."
#define MSG006        "Error writing calendar file!  Your data was NOT saved!  Please address this problem and try again."
#define MSG007        "RemindMe for OS/2"
#define MSG008        "Your calendar file has been saved."
#define MSG009        "Written By Eric A. Wolf"
#define MSG010        "Copyright (C) 1995 - All Rights Reserved"
#define MSG011        "Error in line %d of the calendar file.  That line will be ignored."
#define MSG012        "The number of actual reminders in your calendar file does not match the number specified in the %s= statement."
#define MSG013        "Delete the Current Event"
#define MSG014        "Once deleted, there will be no way to recover this item.  Are you sure you want to delete this event?"
#define MSG015        "Are you sure you want to clear all current dates and start a new calendar?  This operation can not be reversed!"
#define MSG016        "Open a new calendar file..."
#define MSG017        "Save a calendar file as..."
#define MSG018        "Your calendar file has been saved as %s."
#define MSG019        "There was an error saving your calendar file as %s!\n\nPlease try again once the situation has been resolved or specify a new filename using Save As..."
#define MSG020        "Select a Sound File for Audible Reminder"
#define MSG021        "To Do List Background Color"
#define MSG022        "To Do List Text Color"
#define MSG023        "Calendar Dark Color"
#define MSG024        "Calendar Middle Color"
#define MSG025        "Calendar Light Color"
#define MSG026        "Calendar Text Color"
#define MSG027        "Quick Reminder Event Text (Not Completed)"
#define MSG028        "Quick Reminder Event Text (Completed)"
#define MSG029        "Day Highlight Text (Events incomplete)"
#define MSG030        "Day Highlight Background (Events incomplete)"
#define MSG031        "Day Highlight Text (All events complete)"
#define MSG032        "Day Highlight Background (All events complete)"
#define MSG033        "Using the Save... option will overwrite the previous calendar you were using.  Remember to use Save As... if you wish to start a new calendar file."
#define MSG034        "All Current Calendar Entries..."
#define MSG035        "Sorry!  There is no more room for an additional calendar entry.  Please delete old entries and try again."
#define MSG036        "Defer %d minutes"
#define MSG037        "Defer until tomorrow"
#define MSG038        "~File"
#define MSG039        "~New"
#define MSG040        "~Open..."
#define MSG041        "~Save\tF2"
#define MSG042        "~Save As..."
#define MSG043        "~About..."
#define MSG044        "~Exit\tAlt+F4"
#define MSG045        "~Options"
#define MSG046        "~Calendar..."
#define MSG047        "~To Do List..."
#define MSG048        "~Reminders..."
#define MSG049        "Calendar Options..."
#define MSG050        "Date Placement"
#define MSG051        "Draw 3D Calendar"
#define MSG052        "Draw Lines Between Days"
#define MSG053        "Draw Border Around Day #"
#define MSG054        "Use Quick Reminder"
#define MSG055        "Label the Days of the Week"
#define MSG056        "Abbreviate Days of Week"
#define MSG057        "Highlight current date"
#define MSG058        "Abbreviate Month Name"
#define MSG059        "Remind for completed events"
#define MSG060        "Current Calendar:"
#define MSG061        "Change Colors"
#define MSG062        "About..."
#define MSG063        "Please select an event..."
#define MSG064        "New Event"
#define MSG065        "Delete Event"
#define MSG066        "Modify Event"
#define MSG067        "To Do List Options..."
#define MSG068        "Show To Do List"
#define MSG069        "Background Color"
#define MSG070        "Text Color"
#define MSG071        "Enter/Modify Event..."
#define MSG072        "Display this event every month for the year specified"
#define MSG073        "Display this event for every year"
#define MSG074        "Show event on to-do list"
#define MSG075        "Show # of days in advance:"
#define MSG076        "Event reminder"
#define MSG077        "Reminder At Time of Event"
#define MSG078        "minutes before"
#define MSG079        "days before"
#define MSG080        "Visually"
#define MSG081        "Audibly"
#define MSG082        "Both"
#define MSG083        "Reminder Sound"
#define MSG084        "Event Name:"
#define MSG085        "Date "
#define MSG086        "day"
#define MSG087        "month"
#define MSG088        "year"
#define MSG089        "hour"
#define MSG090        "min"
#define MSG091        "To-Do List"
#define MSG092        "Repititon Options"
#define MSG093        "of Event:"
#define MSG094        "Time "
#define MSG095        "of Event:"
#define MSG096        "Mark item as completed"
#define MSG097        "Please select your new color:"
#define MSG098        "Calendar Color Options"
#define MSG099        "Background Light"
#define MSG100        "Background Mid"
#define MSG101        "Background Dark"
#define MSG102        "Calendar Text"
#define MSG103        "Quick Reminder Text"
#define MSG104        "Completed Events"
#define MSG105        "Completed Text"
#define MSG106        "Calendar"
#define MSG107        "Daily Event Highlighting"
#define MSG108        "Uncompleted Bkgnd"
#define MSG109        "Uncompleted Text"
#define MSG110        "Completed Bkgnd"
#define MSG111        "Today's Date Highlight"
#define MSG112        "Sample Calendar"
#define MSG113        "RemindMe Reminder..."
#define MSG114        "Mark As Done"
#define MSG115        "Revise Event"
#define MSG116        "Erase"
#define MSG117        "Reminder Options..."
#define MSG118        "Number of minutes a reminder should snooze"
#define MSG119        "Number of minutes between playing reminder"
#define MSG120        "Default number of days in advance to"
#define MSG121        "sound due to inactivity (0 = don't replay)"
#define MSG122        "display a todo item on the todo list"
#define MSG123        "Launch OS/2 Sound Recorder"
#define MSG124        "OS/2 Sound Recorder"
#define MSG125        "Display previous/next month days on Calendar"
#define MSG126        "To Do List Entry Prefix Text:"
#define MSG127        "None"
#define MSG128        "Date of Event"
#define MSG129        "Time of Event"
#define MSG130        "Date and Time of Event"
#define MSG131        "Ask before deleting an event"
#define MSG132        "Non-current month background color"
#define MSG133        "Non-current Month Days:"
#define MSG134        "Background Color"
#define MSG135        "Text Color"
#define MSG136        "Non-current month text color"
#define MSG137        "Interactive scheduling,"
#define MSG138        "appointment reminders,"
#define MSG139        "intelligent to-do list,"
#define MSG140        "and fully customizable!"
#define MSG141        "Shareware Version"
#define MSG142        "Jump to Date...\tF7"
#define MSG143        "Could not open a temporary file for printing!"
#define MSG144        "Print...\tF6"
#define MSG145        "To Do List through "
#define MSG146        "Show Clock"
#define MSG147        "# of days later to reschedule event after marked done"
#define MSG148        "Jump to Date..."
#define MSG149        "Print..."
#define MSG150        "Print month's calendar with list"
#define MSG151        "Print To-Do List Through:"
#define MSG152        "Today"
#define MSG153        "This week"
#define MSG154        "This month"
#define MSG155        "Width of page (in characters):"
#define MSG156        "Print to do list"

