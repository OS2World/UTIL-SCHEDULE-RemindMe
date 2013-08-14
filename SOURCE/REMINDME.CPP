// To-Do List for future work
// ==========================
// when drawing days, put event text for each day in calendar day boxes
// add Advanced Power Management features to reminders??? -- I need apm.h and apm.lib
// on-line help
// make reminders appear over a FS windows session
   // if possible without writing a windows front end, too
// TEST AND BUG FIX, TEST AND BUG FIX, and TEST AND BUG FIX

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
//
// Project Start Date:      December 18, 1994
// Project Completion Date: January  12, 1995
//
// File Last Modified:      January  30, 1995
//
////////////////////////////////////////////////////////////////////////////////
#include "remindme.h"        // include resource+misc definitions
#include "rmndclas.h"        // include definiton for reminder class


////////////////////////////////////////////////////////////////////////////////
// define switches for all the OS/2 stuff we need
////////////////////////////////////////////////////////////////////////////////
#define INCL_GPI
#define INCL_WINSYS
#define INCL_WININPUT
#define INCL_WINMENUS
#define INCL_WINBUTTONS
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WINTRACKRECT
#define INCL_WINPOINTERS
#define INCL_WINWINDOWMGR
#define INCL_WINLISTBOXES
#define INCL_WINSTDSPIN
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDFILE
#define INCL_WINMLE
#define INCL_WINSTDVALSET
#define INCL_WINPROGRAMLIST
#define INCL_WINTIMER

#include <os2.h>
#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


////////////////////////////////////////////////////////////////////////////////
// include library code to play .WAV file
////////////////////////////////////////////////////////////////////////////////
extern "C" {
    #define INCL_REXXSAA
    #include "rexxsaa.h"
    RexxStart rexxStart;
    }


////////////////////////////////////////////////////////////////////////////////
// misc. defines
////////////////////////////////////////////////////////////////////////////////
#define ENGLISH                   // signal we want ENGLISH version
#define NONE          0
#define TIMEONLY      1
#define DATEONLY      2
#define DATEANDTIME   3
#define STACKSIZE     8
#define MAXEVENTS     300         // maximum # of events calendar can store
#define MAXITEMSIZE   256         // max # of chars in an event title
#define CURRENTVER    1.02        // current version # of software
#define LOWESTVER     1.00        // lowest supported version of software
#define ALLYEARS      1           // value to signal event present on all years
#define ALLMONTHS     13          // value to signal event present on all months
#define TIMERID       2000        // unique ID given for our 1st timer
#define TIMERID2      2001        // unique ID given for our 2nd timer
#define TODOMONTH     10
#define TODOWEEK      11
#define TODOTODAY     12
#define TICKSPERSEC   1000        // # of millisecs per second (and minute below)
#if MAXEVENTS<255                 // correctly determine size of the reminder index
    #define REMINDERINDEX CHAR
#else
    #define REMINDERINDEX SHORT
#endif
#define MAXREMINDERS               MAXEVENTS+1
#define TICKSPERMIN                60*TICKSPERSEC
#define stopMinuteTimer()          WinStopTimer( hab, calendar.client, minuteTimer )
#define startMinuteTimer(TICKS)    minuteTimer = WinStartTimer( hab, calendar.client, TIMERID, TICKS )
#define popStack()                 stack[--stackPos]
#define pushStack(X)               stack[stackPos++] = (CHAR)X


////////////////////////////////////////////////////////////////////////////////
//  Macro constants
////////////////////////////////////////////////////////////////////////////////
#ifdef ENGLISH
    #define MONTHSTRING  "%B 19%y"
    #define MONTHSTRING2 "%b 19%y"
    #define CURDATESTR4  "%A, %B %d, %Y"
    #define MONTHDAY     "%s/%2d "
    #define HOURMIN      "%s:%s "
    #define MONTHANDHOUR "%s/%2d %s:%s "
#else
    #define MONTHSTRING  "19%y %B"
    #define MONTHSTRING2 "19%y %b"
    #define CURDATESTR4  "%A - %d %B %Y"
    #define MONTHDAY     "%2d/%s "
    #define HOURMIN      "%s:%s "
    #define MONTHANDHOUR "%2d/%s %s:%s "
#endif


////////////////////////////////////////////////////////////////////////////////
// define necessary data for each window we open
////////////////////////////////////////////////////////////////////////////////
struct APPWINDOW {
    HWND frame;
    HWND client;
    HWND menubar;
    HWND sysMenu;
    HWND minMax;
    HWND titleBar;
    HWND childPane;
};


////////////////////////////////////////////////////////////////////////////////
// function prototypes
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY printProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY calColorProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY ClientWndProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY ToDoClientWndProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY calDrawOptionsProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY sampleCalProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY genericProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY reminderProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY pickAnEventProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY toDoListProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY modifyEventProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY ChangeColorProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY toDoListOptionsProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY reminderOptionsProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY jumpToProc( HWND, ULONG, MPARAM, MPARAM );
VOID print( CHAR, CHAR, CHAR );
VOID setSnooze( HWND, REMINDERINDEX );
VOID centerDialog( HWND hWnd );
VOID initCalendar( VOID );
VOID readOptionFile( VOID );
VOID drawCalendar( HPS, HWND, RECTL );
VOID ShowControls( CHAR, APPWINDOW );
VOID sortTimeEntries( VOID );
VOID resetFileDialog( VOID );
VOID fillToDoList( VOID );
CHAR numberOfDaysInMonth( CHAR, SHORT );
CHAR writeOptionFile( VOID );
CHAR makeBackupFile( VOID );
VOID setItem( CHAR **, CHAR * );
CHAR modifyEvent( HWND, REMINDERINDEX );
VOID restartTimer( VOID );
VOID playSound( CHAR* );
VOID launchRecorder( VOID );
VOID drawClock( HPS, HWND, CHAR, CHAR, CHAR );
VOID moveToDate( REMINDERINDEX, CHAR, CHAR, short, CHAR, CHAR );


////////////////////////////////////////////////////////////////////////////////
// Initialized Variables
////////////////////////////////////////////////////////////////////////////////
REMINDERINDEX numReminders = 0;
REMINDERINDEX  toDoCount = 0;
CHAR  *calendarFile = NULL;
ULONG calDaysAlignment = RIGHT;
float calBottom = 1;
float calWidth = 1;
float calLength = 1;
float calHeight = 1;
COLOR toDoBackColor = CLR_YELLOW;
COLOR toDoForeColor = CLR_BLACK;
COLOR CalHighText = CLR_RED;
COLOR CalDayColor = CLR_RED;
COLOR CalHighlight = CLR_YELLOW;
COLOR CalDoneHighlight = CLR_CYAN;
COLOR CalDoneText = CLR_BLACK;
COLOR CalText = CLR_BLACK;
COLOR CalRemindDone = CLR_DARKGRAY;
COLOR CalButtonLight = CLR_WHITE;
COLOR CalButtonMiddle = CLR_PALEGRAY;
COLOR CalButtonDark = CLR_DARKGRAY;
COLOR oldMonthBack = CLR_PALEGRAY;
COLOR oldMonthText = CLR_DARKGRAY;
LONG  printWidth = 80;
CHAR  toDoDisplay = DATEONLY;
CHAR  drawCalBorderLines = TRUE;
CHAR  calDrawDayBorder = TRUE;
CHAR  calButtonShadows = TRUE;
CHAR  showAllDays = TRUE;
CHAR  showToDoList = TRUE;
CHAR  calQuickRemind = TRUE;
CHAR  calShowControls = TRUE;
CHAR  todoShowControls = TRUE;
CHAR  labelDaysOfWeek = FALSE;
CHAR  abbreviateWeekDays = TRUE;
CHAR  abbreviateMonth = FALSE;
CHAR  highlightToday = TRUE;
CHAR  showClock = TRUE;
CHAR  remindEvenIfDone = TRUE;
CHAR  askBeforeDeleteItem = TRUE;
CHAR  daysInAdvance = 7;
CHAR  snoozeTime = 9;
CHAR  soundReplay = 5;
CHAR  snoozeOn = FALSE;
CHAR  prevDay = 0;
CHAR  stackPos = 0;
CHAR  printToDo = TRUE;
CHAR  printCal = TRUE;

////////////////////////////////////////////////////////////////////////////////
// Uninitialized Variables
////////////////////////////////////////////////////////////////////////////////
CHAR           *hourArray[24];
ULONG          minuteTimer;
ULONG          soundTimer;
SWP            DesktopPos;
HAB            hab;
FILEDLG        fileDlgInfo;
APPWINDOW      toDoList;
APPWINDOW      calendar;
RECTL          calendarRect;
RECTL          reminderRect;
HWND           popUpMenu;
struct date    todaysDate;
struct tm      *time_now;
FNWP           *oldSpinProc;
FNWP           *oldSampleProc;
FNWP           *oldToDoList;
CHAR           *messagePointer;
COLOR          colorToChange;
SHORT          curYear;
CHAR           totalDaysInMonth;
CHAR           curDay;
CHAR           curMonth;
CHAR           calStartIndex;
CHAR           resyncTimer;
CHAR           minimizeToDo;
CHAR           minimizeCal;
unsigned CHAR  stack[STACKSIZE];
REMINDERINDEX  eventToEdit;
REMINDERINDEX  eventReminder;
REMINDERINDEX  toDoListArray[MAXREMINDERS];
REMINDERINDEX  offsetTable[MAXREMINDERS];
ReminderItem   *reminders[MAXREMINDERS];


////////////////////////////////////////////////////////////////////////////////
// main function - application entry point
////////////////////////////////////////////////////////////////////////////////
main( int argc, char* argv[] )
{
    HMQ hmq;                   // a message queue for our application
    QMSG qmsg;                 // a message packet as retrieved from the queue
    unsigned LONG FrameFlags;  // flags used to build windows

    //--------------------------------------------------------------------------
    // Get today's date as returned from the operating system
    //--------------------------------------------------------------------------
    getdate( &todaysDate );
    curMonth = todaysDate.da_mon;
    curYear = (SHORT)todaysDate.da_year;

    //--------------------------------------------------------------------------
    // install some default dates in our calendar - these are overwritten if
    // a configuration file exists
    //--------------------------------------------------------------------------
    makeBackupFile();
    initCalendar();

    //--------------------------------------------------------------------------
    // Initialize some window intrinsics
    //--------------------------------------------------------------------------
    hab = WinInitialize( 0 );                 // handle: anchor block
    hmq = WinCreateMsgQueue( hab, 0 );        // handle: message queue
    WinQueryWindowPos( HWND_DESKTOP, &DesktopPos );

    //--------------------------------------------------------------------------
    // register the window classes for both the main app and the to do list
    //--------------------------------------------------------------------------
    WinRegisterClass(hab,                // Main Application Frame Window Class
                     ClassName,          // - name of class being registered
                     ClientWndProc,      // - window procedure for class
                     CS_SIZEREDRAW,      // - class style
                     0);
    WinRegisterClass(hab,                // To-Do List Window Class
                     ToDoClassName,      // - name of class being registered
                     ToDoClientWndProc,  // - window procedure for class
                     CS_SIZEREDRAW,      // - class style
                     0);

    //--------------------------------------------------------------------------
    // create calendar window
    //--------------------------------------------------------------------------
    FrameFlags = FCF_TITLEBAR|FCF_SYSMENU|FCF_AUTOICON|FCF_TASKLIST|FCF_BORDER|\
                 FCF_MINBUTTON|FCF_MAXBUTTON|FCF_SIZEBORDER|FCF_MENU|FCF_ICON|
                 FCF_ACCELTABLE;

    calendar.frame = WinCreateStdWindow(HWND_DESKTOP,  // parent
                                        0,             // style
                                        &FrameFlags,   // control data
                                        ClassName,     // client name
                                        ClassName,     // title bar text
                                        0,             // client style
                                        NULLHANDLE,    // resource handle
                                        MENUBAR,       // menu resource id
                                        &calendar.client);  // client pointer
    calendar.menubar = WinWindowFromID( calendar.frame, FID_MENU );
    calendar.sysMenu = WinWindowFromID( calendar.frame, FID_SYSMENU );
    calendar.titleBar = WinWindowFromID( calendar.frame, FID_TITLEBAR );
    calendar.minMax = WinWindowFromID( calendar.frame, FID_MINMAX );
    calendar.childPane = 0;
    WinSetWindowPos( calendar.frame, HWND_TOP, DesktopPos.cx*.20,
                     DesktopPos.cy*.20, DesktopPos.cx*.75, DesktopPos.cy*.75,
                     SWP_MOVE|SWP_SIZE);
    popUpMenu = WinLoadMenu( HWND_DESKTOP, NULLHANDLE, MENUBAR );

    //--------------------------------------------------------------------------
    // create to-do list window
    //--------------------------------------------------------------------------
    FrameFlags = FCF_TASKLIST|FCF_TITLEBAR|FCF_MINBUTTON|FCF_SIZEBORDER|FCF_AUTOICON|FCF_ICON;

    toDoList.frame = WinCreateStdWindow(HWND_DESKTOP,
                                        0,
                                        &FrameFlags,
                                        ToDoClassName,
                                        MSG_TODOTTL,
                                        0,
                                        (HMODULE)NULL,
                                        TODOLISTICON,
                                        &toDoList.client);
    toDoList.menubar = WinWindowFromID( toDoList.frame, FID_MENU );
    toDoList.sysMenu = WinWindowFromID( toDoList.frame, FID_SYSMENU );
    toDoList.titleBar = WinWindowFromID( toDoList.frame, FID_TITLEBAR );
    toDoList.minMax = WinWindowFromID( toDoList.frame, FID_MINMAX );
    // the list is actually a modified MLE control occupying entire window
    toDoList.childPane = WinCreateWindow( toDoList.client, WC_MLE, "",
                                          WS_VISIBLE, 0, 0, 0, 0,
                                          toDoList.frame, HWND_TOP, 0, 0, 0 );
    oldToDoList = WinSubclassWindow( toDoList.childPane, toDoListProc );
    WinSetWindowPos( toDoList.frame, HWND_TOP, DesktopPos.cx*.02,
                     DesktopPos.cy*.02, DesktopPos.cx*.25, DesktopPos.cy*.25,
                     SWP_MOVE|SWP_SIZE|SWP_ACTIVATE);
    WinSendMsg( toDoList.childPane, MLM_SETTEXTCOLOR, MPFROMLONG( toDoForeColor ), 0 );
    WinSendMsg( toDoList.childPane, MLM_SETBACKCOLOR, MPFROMLONG( toDoBackColor ), 0 );

    //--------------------------------------------------------------------------
    // read in the calendar option file
    //--------------------------------------------------------------------------
    memset( &fileDlgInfo, 0, sizeof(FILEDLG));
    fileDlgInfo.cbSize = sizeof(FILEDLG);
    resetFileDialog();

    if( argc > 2 )
        WinMessageBox( HWND_DESKTOP, calendar.frame, MSG001, MSG004, 0,
                       MB_ERROR|MB_OK );
    else if( argc == 2 )
        setItem( &calendarFile, argv[1] );

    //--------------------------------------------------------------------------
    // initialize hours of the day - differs between language versions
    //--------------------------------------------------------------------------
    for (int z=0; z<24; z++)
        hourArray[z] = new char[4];
    #ifdef ENGLISH
    for (z=0; z<24; z++)
        if (z==0)
            sprintf( hourArray[z], "12a" );
        else if (z==12)
            sprintf( hourArray[z], "12p" );
        else if (z<12)
            sprintf( hourArray[z], "%2da", z );
        else
            sprintf( hourArray[z], "%2dp", z-12 );
    #else
    for (z=0; z<24; z++)
        sprintf( hourArray[z], "%2d ", z );
    #endif

    //--------------------------------------------------------------------------
    // read options, redraw calendar, and update to do list
    //--------------------------------------------------------------------------
    readOptionFile();
    redrawCalendar();

    //--------------------------------------------------------------------------
    // initialize some initial window properties
    //--------------------------------------------------------------------------
    WinSetWindowPos( toDoList.frame, 0,0,0,0,0, showToDoList?SWP_SHOW:SWP_HIDE);
    WinSetWindowPos( calendar.frame, HWND_TOP, 0,0,0,0, SWP_SHOW);
    ShowControls( todoShowControls, toDoList );
    ShowControls( calShowControls, calendar );
    WinSetWindowPos( toDoList.frame, 0, 0, 0, 0, 0, (minimizeToDo?SWP_MINIMIZE:0) );
    WinSetWindowPos( calendar.frame, 0, 0, 0, 0, 0, (minimizeCal?SWP_MINIMIZE:0) );
    restartTimer();

    //--------------------------------------------------------------------------
    // create a message queue and then as long as there are messages to get,
    // get them and dispatch them to our message handlers
    //--------------------------------------------------------------------------
    while( WinGetMsg( hab, &qmsg, (HWND)0, 0, 0 ) != FALSE )
        WinDispatchMsg( hab, &qmsg );

    //--------------------------------------------------------------------------
    // write option file before quitting and destroy all windows, queues, etc
    //--------------------------------------------------------------------------
    stopMinuteTimer();
    writeOptionFile();
    WinDestroyWindow( toDoList.frame );
    WinDestroyWindow( calendar.frame );
    for (z=0; z<24; z++)
        delete hourArray[z];
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// centerDialog - center a given window on the desktop
////////////////////////////////////////////////////////////////////////////////
VOID centerDialog( HWND hWnd )
{
    SWP swp;
    WinQueryWindowPos( hWnd, (PSWP)&swp);
    WinSetWindowPos( hWnd, (HWND)0,
                     ((SHORT)((DesktopPos.cx-swp.cx)/2)),
                     ((SHORT)((DesktopPos.cy-swp.cy)/2)),
                     0, 0, SWP_MOVE);
}


////////////////////////////////////////////////////////////////////////////////
// launchRecorder - launch the OS/2 sound reocorder
////////////////////////////////////////////////////////////////////////////////
VOID launchRecorder( VOID )
{
    PROGDETAILS pDetails;

    memset( &pDetails, 0, sizeof(PROGDETAILS) );
    pDetails.Length                      = sizeof(PROGDETAILS);
    pDetails.progt.progc                 = PROG_PM;
    pDetails.progt.fbVisible             = SHE_VISIBLE;
    pDetails.pszTitle                    = MSG124;
    pDetails.pszExecutable               = "WEPM.EXE";
    pDetails.swpInitial.fl               = SWP_ACTIVATE;
    pDetails.swpInitial.hwndInsertBehind = HWND_TOP;
    pDetails.swpInitial.hwnd             = calendar.frame;
    WinStartApp( calendar.frame, &pDetails, NULL, NULL, SAF_STARTCHILDAPP );
}


////////////////////////////////////////////////////////////////////////////////
// resetFileDialog - reset file dialog control structure for its use next time
////////////////////////////////////////////////////////////////////////////////
VOID resetFileDialog( VOID )
{
    INT s = strlen( fileDlgInfo.szFullFile );
    while((fileDlgInfo.szFullFile[s]!='\\') && (s>=0))
        s--;

    if (fileDlgInfo.szFullFile[s]=='\\')
        fileDlgInfo.szFullFile[s+1]='\0';
    else
        fileDlgInfo.szFullFile[0]='\0';
}


////////////////////////////////////////////////////////////////////////////////
// sortTimeEntries - rearrange time entries into chronological order
////////////////////////////////////////////////////////////////////////////////
VOID sortTimeEntries( VOID )
{
    ReminderItem *tmp;

    for (REMINDERINDEX z1=0; z1<numReminders-1; z1++)
        for (REMINDERINDEX z2=0; z2<numReminders-1; z2++)
            if (reminders[z2]->getTimeOfOccurence() >
                reminders[z2+1]->getTimeOfOccurence() ) {
                tmp             = reminders[z2];             // swap two entries
                reminders[z2]   = reminders[z2+1];
                reminders[z2+1] = tmp;
                }
}


////////////////////////////////////////////////////////////////////////////////
// initCalendar - initialize some events in our calendar so it is not completely
//                empty when a new calendar is created
////////////////////////////////////////////////////////////////////////////////
VOID initCalendar( VOID )
{
    CHAR  tmp_month = curMonth;
    SHORT tmp_year = curYear;
    CHAR  tmp_day = todaysDate.da_day + 14;

    numReminders = 0;
    if ( tmp_day > numberOfDaysInMonth( tmp_month, tmp_year ) ) {
        tmp_day -= numberOfDaysInMonth( tmp_month, tmp_year );
        tmp_month++;
        if ( tmp_month > 12 ) {
            tmp_month = 1;
            tmp_year++;
            }
        }

    // default dates to be installed in each new calendar
    reminders[numReminders++] = new ReminderItem( 1, 0, 14, tmp_month, tmp_day, tmp_year, 12, 0, TRUE, BOTH, FALSE, 0, 3, CHIMEWAV, MSG_REGISTER, 7 );
    reminders[numReminders++] = new ReminderItem( 1, 0, 14, 12, 25, ALLYEARS, 0, 0, FALSE, BOTH, FALSE, 0, 14, CHIMEWAV, MSG_CHRISTMAS, 0 );
    reminders[numReminders++] = new ReminderItem( 1, 0, 6, 1, 1, ALLYEARS, 0, 0, FALSE, BOTH, FALSE, 0, 14, CHIMEWAV, MSG_NEWYEAR, 0 );
    reminders[numReminders++] = new ReminderItem( 1, 0, 7, 9, 12, ALLYEARS, 0, 0, FALSE, BOTH, FALSE, 0, 14, CHIMEWAV, MSG_ERICBDAY, 0 );
    sortTimeEntries();
}


////////////////////////////////////////////////////////////////////////////////
// numberOfDaysInMonth - return # of days in a month (even checking for leap
//                       year).   Month is between 1 and 12
////////////////////////////////////////////////////////////////////////////////
static CHAR daysInMonth[13] = { 31,28,31,30,31,30,31,31,30,31,30,31,31 };

CHAR numberOfDaysInMonth( CHAR month, SHORT year )
{
    if(month==2)
        if( ((year%4==0)&&(year%100!=0)) || ((year%100==0)&&(year%400==0)))
            return 29;                      //handle 29 days in Feb in leap year
    return daysInMonth[month-1];
}


////////////////////////////////////////////////////////////////////////////////
// Reports an error in the calendar configuration file to the user (gives line#)
////////////////////////////////////////////////////////////////////////////////
VOID reportErrorLine( INT lineNum )
{
    CHAR tmp[ 128 ];
    sprintf( tmp, MSG011, lineNum );
    WinMessageBox( HWND_DESKTOP, calendar.frame, tmp, MSG004, 0, MB_OK );
}


////////////////////////////////////////////////////////////////////////////////
// ShowControls - toggles display of window controls in the window specified
//                by appWin (show/hidden specified by boolean showControls)
////////////////////////////////////////////////////////////////////////////////
VOID ShowControls( CHAR showControls, APPWINDOW window )
{
    SWP Position;
    SWP OldPosition;
    RECTL Rectangle;
    HWND hwndFrame = window.frame;
    CHAR WasVisible = WinIsWindowVisible( hwndFrame );

    //--------------------------------------------------------------------------
    // Restore and hide the window
    //--------------------------------------------------------------------------
    WinQueryWindowPos( hwndFrame, &OldPosition );
    WinSetWindowPos( hwndFrame, NULLHANDLE, 0, 0, 0, 0, SWP_RESTORE|SWP_HIDE );

    //--------------------------------------------------------------------------
    // Determine client window and location
    //--------------------------------------------------------------------------
    WinQueryWindowPos( hwndFrame, &Position );
    Rectangle.xLeft = Position.x;
    Rectangle.xRight = Position.x + Position.cx;
    Rectangle.yBottom = Position.y;
    Rectangle.yTop = Position.y + Position.cy;
    WinCalcFrameRect( hwndFrame, &Rectangle, TRUE );

    //--------------------------------------------------------------------------
    // Hide or show the controls windows by changing their parent window handle
    //--------------------------------------------------------------------------
    WinSetParent( window.menubar, showControls?hwndFrame:HWND_OBJECT, showControls );
    WinSetParent( window.sysMenu, showControls?hwndFrame:HWND_OBJECT, showControls );
    WinSetParent( window.titleBar, showControls?hwndFrame:HWND_OBJECT, showControls );
    WinSetParent( window.minMax, showControls?hwndFrame:HWND_OBJECT, showControls );

    //--------------------------------------------------------------------------
    // Notify the frame that the environment has changed
    //--------------------------------------------------------------------------
    WinSendMsg( hwndFrame, WM_UPDATEFRAME, MPFROMSHORT( FCF_TITLEBAR|\
                FCF_SYSMENU|FCF_MINBUTTON), 0L );

    //--------------------------------------------------------------------------
    // Reposition the frame around the remaining client window
    //--------------------------------------------------------------------------
    WinCalcFrameRect( hwndFrame, &Rectangle, FALSE ) ;
    WinSetWindowPos( hwndFrame, NULLHANDLE, (SHORT)Rectangle.xLeft,
                     (SHORT)Rectangle.yBottom,
                     (SHORT)(Rectangle.xRight-Rectangle.xLeft),
                     (SHORT)(Rectangle.yTop-Rectangle.yBottom),
                     SWP_SIZE|SWP_MOVE ) ;

    //--------------------------------------------------------------------------
    // Restore window to how we found it
    //--------------------------------------------------------------------------
    if( OldPosition.fl & SWP_MAXIMIZE )
        WinSetWindowPos( hwndFrame, NULLHANDLE,
                          (SHORT) Rectangle.xLeft, (SHORT) Rectangle.yBottom,
                          (SHORT) (Rectangle.xRight-Rectangle.xLeft),
                          (SHORT) (Rectangle.yTop-Rectangle.yBottom),
                          SWP_SIZE|SWP_MOVE|( OldPosition.fl & SWP_MAXIMIZE ));
    if( WasVisible )
        WinShowWindow( hwndFrame, TRUE ) ;
}


////////////////////////////////////////////////////////////////////////////////
// Subclassed message handler for the calendar viewing area used in the
// calendar options and color options dialogs.  (Intercepts only paint commands)
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY sampleCalProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    //--------------------------------------------------------------------------
    // if the sample calendar requests repainting, call our routine to do it
    //--------------------------------------------------------------------------
    if( msg == WM_PAINT ) {
        RECTL rect;
        HPS hps = WinGetPS( hWnd );
        WinQueryWindowRect( hWnd, &rect );
        drawCalendar( hps, hWnd, rect );
        WinReleasePS( hps );
        }
    return oldSampleProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// Message handler for a generic text-display dialog box (About... box)
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY genericProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    if( msg == WM_INITDLG )
        centerDialog( hWnd );
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// modifyEvent - stub to call dialog to modify the passed calendar event
////////////////////////////////////////////////////////////////////////////////
CHAR modifyEvent( HWND hWnd, REMINDERINDEX numRemind )
{
    eventToEdit = numRemind;
    WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)modifyEventProc, 0, EVENTDLG,
              (PVOID)NULL);
    reminders[ numRemind ]->updateTimes();
    sortTimeEntries();
    redrawCalendar();
    return (eventToEdit==TRUE);
}


////////////////////////////////////////////////////////////////////////////////
// setColor - stub to call dialog to revise display color
////////////////////////////////////////////////////////////////////////////////
VOID setColor( HWND hWnd, CHAR *msg, COLOR *color )
{
    messagePointer = msg;
    colorToChange = *color;
    WinDlgBox( HWND_DESKTOP, hWnd, (PFNWP)ChangeColorProc, 0, CHOOSECOLOR, 0 );
    *color = colorToChange;
    WinInvalidateRegion( WinWindowFromID( hWnd, SAMPLECAL ), 0, FALSE );
}


////////////////////////////////////////////////////////////////////////////////
// moveWindow - move the window (passed in via structure) using tracking by user
////////////////////////////////////////////////////////////////////////////////
void moveWindow( APPWINDOW window )
{
    HWND hwndFrame = window.frame;
    TRACKINFO TrackInfo;
    SWP Position;

    memset( &TrackInfo, 0, sizeof(TRACKINFO) );
    TrackInfo.cxBorder = WinQuerySysValue( HWND_DESKTOP, SV_CXSIZEBORDER );
    TrackInfo.cyBorder = WinQuerySysValue( HWND_DESKTOP, SV_CYSIZEBORDER );
    TrackInfo.cxGrid = 1;
    TrackInfo.cyGrid = 1;
    TrackInfo.cxKeyboard = 8;
    TrackInfo.cyKeyboard = 8;

    WinQueryWindowPos( hwndFrame, &Position );
    TrackInfo.rclTrack.xLeft = Position.x;
    TrackInfo.rclTrack.xRight = Position.x + Position.cx;
    TrackInfo.rclTrack.yBottom = Position.y;
    TrackInfo.rclTrack.yTop = Position.y + Position.cy;

    TrackInfo.rclBoundary.xLeft = DesktopPos.x;
    TrackInfo.rclBoundary.xRight = DesktopPos.x + DesktopPos.cx;
    TrackInfo.rclBoundary.yBottom = DesktopPos.y;
    TrackInfo.rclBoundary.yTop = DesktopPos.y + DesktopPos.cy;

    TrackInfo.ptlMaxTrackSize.x = Position.cx;
    TrackInfo.ptlMaxTrackSize.y = Position.cy;
    TrackInfo.fs = TF_MOVE;

    if( WinTrackRect( HWND_DESKTOP, NULLHANDLE, &TrackInfo ))
        WinSetWindowPos( hwndFrame, NULLHANDLE, (SHORT)TrackInfo.rclTrack.xLeft,
                         (SHORT)TrackInfo.rclTrack.yBottom, 0, 0, SWP_MOVE );
}


////////////////////////////////////////////////////////////////////////////////
// Message handler for calendar draw options dialog box
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY calDrawOptionsProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch( msg ) {
        //----------------------------------------------------------------------
        // when the dialog is being initialized, center it on desktop and check
        // the appropriate radio button for whatever setting they currently have
        //----------------------------------------------------------------------
        case WM_INITDLG:
            centerDialog( hWnd );

            if (calDaysAlignment == CENTER)
                WinSendDlgItemMsg( hWnd, CENTERALIGN, BM_SETCHECK,
                                   MPFROM2SHORT( 1, 0 ), 0);
            else if (calDaysAlignment == RIGHT)
                WinSendDlgItemMsg( hWnd, RIGHTALIGN, BM_SETCHECK,
                                   MPFROM2SHORT( 1, 0 ), 0);
            else WinSendDlgItemMsg( hWnd, LEFTALIGN, BM_SETCHECK,
                                    MPFROM2SHORT( 1, 0 ), 0);
            WinCheckButton( hWnd, USE3DEFFECT, calButtonShadows );
            WinCheckButton( hWnd, LINESBETWEEN, drawCalBorderLines );
            WinCheckButton( hWnd, DRAWDAYBORDER, calDrawDayBorder );
            WinCheckButton( hWnd, DAYSOFWEEK, labelDaysOfWeek );
            WinCheckButton( hWnd, ABBREVIATEDAYS, abbreviateWeekDays );
            WinEnableControl( hWnd, ABBREVIATEDAYS, labelDaysOfWeek );
            WinCheckButton( hWnd, HIGHLIGHTTODAY, highlightToday );
            WinCheckButton( hWnd, ABBREVIATEMONTH, abbreviateMonth );
            WinCheckButton( hWnd, QUICKREMINDER, calQuickRemind );
            WinCheckButton( hWnd, DISPLAYLASTNEXT, showAllDays );
            WinCheckButton( hWnd, SHOWCLOCK, showClock );
            WinEnableControl( hWnd, REMINDOLD, calQuickRemind );
            WinCheckButton( hWnd, REMINDOLD, remindEvenIfDone );

            // subclass the static rectangle control in the dialog box so that
            // we can draw our calendar in that box (calendar preview area)
            oldSampleProc = WinSubclassWindow( WinWindowFromID(hWnd, SAMPLECAL),
                                               sampleCalProc );
            break;
        //----------------------------------------------------------------------
        // if a radio button was clicked (or double clicked), reflect the
        // requested changes in calendar previewing area in window
        //----------------------------------------------------------------------
        case WM_CONTROL:
            if ((SHORT2FROMMP(mp1)==BN_CLICKED) ||
                (SHORT2FROMMP(mp1)==BN_DBLCLICKED)) {
                if (WinQueryButtonCheckstate( hWnd, LEFTALIGN ))
                    calDaysAlignment = LEFT;
                else if (WinQueryButtonCheckstate( hWnd, RIGHTALIGN ))
                    calDaysAlignment = RIGHT;
                else calDaysAlignment = CENTER;

                calButtonShadows = WinQueryButtonCheckstate( hWnd, USE3DEFFECT );
                drawCalBorderLines = WinQueryButtonCheckstate( hWnd, LINESBETWEEN );
                calDrawDayBorder = WinQueryButtonCheckstate( hWnd, DRAWDAYBORDER );
                calQuickRemind = WinQueryButtonCheckstate( hWnd, QUICKREMINDER );
                labelDaysOfWeek = WinQueryButtonCheckstate( hWnd, DAYSOFWEEK );
                abbreviateWeekDays = WinQueryButtonCheckstate( hWnd, ABBREVIATEDAYS );
                highlightToday = WinQueryButtonCheckstate( hWnd, HIGHLIGHTTODAY );
                abbreviateMonth = WinQueryButtonCheckstate( hWnd, ABBREVIATEMONTH );
                remindEvenIfDone = WinQueryButtonCheckstate( hWnd, REMINDOLD );
                showAllDays = WinQueryButtonCheckstate( hWnd, DISPLAYLASTNEXT );
                showClock = WinQueryButtonCheckstate( hWnd, SHOWCLOCK );

                WinEnableControl( hWnd, ABBREVIATEDAYS, labelDaysOfWeek );
                WinEnableControl( hWnd, REMINDOLD, calQuickRemind );
                WinInvalidateRegion( WinWindowFromID( hWnd, SAMPLECAL ),
                                     NULLHANDLE, FALSE );
                }
            break;
        //----------------------------------------------------------------------
        // Handle any of the controls within the dialog
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // user wants to change colors for the calendar
                //--------------------------------------------------------------
                case CHANGECOLORS:
                    WinDlgBox( HWND_DESKTOP, hWnd, (PFNWP)calColorProc, 0,
                               CALCOLORDLG, (PVOID)NULL);
                    WinInvalidateRegion( WinWindowFromID( hWnd, SAMPLECAL ),
                                         NULLHANDLE, FALSE );
                    return 0;
                //--------------------------------------------------------------
                // dismiss dialog and redraw real calendar if user is done
                //--------------------------------------------------------------
                case DID_OK:
                    WinDismissDlg( hWnd, TRUE );
                    redrawCalendar();
                    return 0;
                //--------------------------------------------------------------
                // let system handle anything we don't want to handle
                //--------------------------------------------------------------
                default:
                    break;
                }
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        // if nothing further we want to intercept, pass message onto system
        //----------------------------------------------------------------------
        default:
            break;
    }
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// Message handler for the To Do List Options dialog box
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY ToDoClientWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    //--------------------------------------------------------------------------
    // user has resized to do list frame, resize the inner MLE window, too!
    //--------------------------------------------------------------------------
    if( msg == WM_SIZE ) {
        SWP Position;
        WinQueryWindowPos( toDoList.client, &Position );
        WinSetWindowPos( toDoList.childPane, HWND_TOP, Position.x,
                         Position.y, Position.cx, Position.cy, SWP_SIZE);
        }
    return WinDefWindowProc(hWnd, msg, mp1, mp2);  // default processing
}


////////////////////////////////////////////////////////////////////////////////
// Subclassed message handler for the To Do List MLE control
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY toDoListProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch( msg ) {
        //----------------------------------------------------------------------
        // this is a signal to refresh our variables with the colors of the todo
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            if( SHORT1FROMMP(mp1) == SYNCCOLORS ) {
                toDoBackColor = LONGFROMMR( WinSendMsg( toDoList.childPane, MLM_QUERYBACKCOLOR, 0, 0 ) );
                toDoForeColor = LONGFROMMR( WinSendMsg( toDoList.childPane, MLM_QUERYTEXTCOLOR, 0, 0 ) );
                WinSendMsg( toDoList.childPane, MLM_SETTEXTCOLOR, MPFROMLONG( toDoForeColor ), 0 );
                WinSendMsg( toDoList.childPane, MLM_SETBACKCOLOR, MPFROMLONG( toDoBackColor ), 0 );
                }
            return oldToDoList( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        // the color parameters have been changed
        //----------------------------------------------------------------------
        case WM_PRESPARAMCHANGED: {
            WinPostMsg( hWnd, WM_COMMAND, MPFROMSHORT( SYNCCOLORS ), mp2 );
            return oldToDoList( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        // button 1 double clicked - modify the event that is highlighted
        //----------------------------------------------------------------------
        case WM_BUTTON1DBLCLK: {
            IPT cursorPos = LONGFROMMR( WinSendMsg( hWnd, MLM_QUERYSEL, MPFROMSHORT(MLFQS_CURSORSEL), 0L ) );
            LONG sel = LONGFROMMR( WinSendMsg( hWnd, MLM_LINEFROMCHAR, MPFROMLONG(cursorPos), 0L ) );
            LONG textLen = LONGFROMMR( WinSendMsg( hWnd, MLM_QUERYTEXTLENGTH, 0, 0 ) );

            if (cursorPos == textLen) {
                // the user has clicked in a blank region, give them a new event
                if ( numReminders == MAXREMINDERS ) {
                    WinMessageBox( HWND_DESKTOP, calendar.frame, MSG035, MSG004,
                                   0, MB_ERROR|MB_OK );
                    return 0;
                    }

                reminders[numReminders++] = new ReminderItem( 1, 0,
                                                              daysInAdvance,
                                                              todaysDate.da_mon,
                                                              todaysDate.da_day,
                                                              todaysDate.da_year,
                                                              0, 0,
                                                              TRUE, BOTH,
                                                              FALSE, 0, 3,
                                                              CHIMEWAV,
                                                              NEWITEMTEXT,
                                                              0 );
                if( modifyEvent( hWnd, numReminders-1 ) == FALSE ) {
                    numReminders--;
                    delete reminders[numReminders];
                    }
                }
            else if (sel <= toDoCount) {
                // edit an existing event
                IPT startPos = LONGFROMMR( WinSendMsg( hWnd, MLM_CHARFROMLINE, MPFROMLONG(sel), 0L ) );
                IPT endPos = startPos + strlen(reminders[toDoListArray[sel]]->getText()) + 2;
                WinSendMsg( hWnd, MLM_SETSEL, MPFROMLONG(endPos), MPFROMLONG(startPos) );
                if (modifyEvent( hWnd, toDoListArray[sel] ))
                    redrawCalendar();
                WinSendMsg( hWnd, MLM_SETSEL, 0, 0 );
                }
            return 0;
            }
        //----------------------------------------------------------------------
        // if button 2 is held down, move the window
        //----------------------------------------------------------------------
        case WM_BUTTON2DOWN:
            moveWindow( toDoList );
            return 0;
        //----------------------------------------------------------------------
        // if button 2 double clicked, toggle display of window controls
        //----------------------------------------------------------------------
        case WM_BUTTON2DBLCLK:
            todoShowControls = !todoShowControls;
            ShowControls( todoShowControls, toDoList );
            return 0;
        //----------------------------------------------------------------------
        // if nothing further we want to intercept, pass message onto system
        //----------------------------------------------------------------------
        default:
            break;
        }
    return oldToDoList( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// fillToDoList - fill the todo list with those items, relative to today's date,
//                that the user wants to appear on the todo list
////////////////////////////////////////////////////////////////////////////////
VOID fillToDoList( VOID )
{
    #define TODOSIZE 8192
    INT yr, mo, dy;
    CHAR *text = new char[ TODOSIZE ];

    if (text == NULL) {
        DosBeep(100,200);
        return;
        }

    toDoCount = 0;
    memset( text, 0, TODOSIZE );
    for (REMINDERINDEX z=0; z<numReminders; z++) {
        ReminderItem *tmp = reminders[z];

        if ((tmp->getToDo()) && (!tmp->getItemDone())) {
            yr = todaysDate.da_year;
            mo = todaysDate.da_mon;
            dy = todaysDate.da_day + tmp->getNumToShow();

            while ( dy > numberOfDaysInMonth( mo, yr ) ) {
                dy = dy - numberOfDaysInMonth( mo, yr );
                mo++;
                if (mo > 12) {
                    mo = mo - 12;
                    yr++;
                    }
                }

            if ( tmp->getTimeOfOccurence() <= calcTimeOfOccurence(yr,mo,dy,23,59) ) {
                if ((strlen(text)+strlen(tmp->getText())+5) < TODOSIZE ) {
                    char mo[4],mn[4];
                    char day = tmp->getDay();
                    char hour = tmp->getHour();
                    char tmpDate[16];

                    sprintf(mn, "%2d", tmp->getMinute() );
                    if( mn[0]==' ')
                        mn[0]='0';

                    if (tmp->getMonth()==ALLMONTHS)
                        strcpy( mo, "--" );
                    else
                        sprintf(mo, "%2d", tmp->getMonth() );

                    tmpDate[0] = 0;
                    if( toDoDisplay == DATEONLY)
                        #ifdef ENGLISH
                        sprintf( tmpDate, MONTHDAY, mo, day );
                        #else
                        sprintf( tmpDate, MONTHDAY, day, mo );
                        #endif
                    else if( toDoDisplay == TIMEONLY)
                        sprintf( tmpDate, HOURMIN, hourArray[hour], mn );
                    else if( toDoDisplay == DATEANDTIME)
                        #ifdef ENGLISH
                        sprintf( tmpDate, MONTHANDHOUR, mo, day, hourArray[hour], mn );
                        #else
                        sprintf( tmpDate, MONTHANDHOUR, day, mo, hourArray[hour], mn );
                        #endif

                    strcat( text, tmpDate );
                    strcat( text, "* " );
                    strcat( text, tmp->getText() );
                    strcat( text, "\n" );
                    toDoListArray[toDoCount++] = z;
                    }
                }
            }
        }

    if ( text[0] )
        text[strlen(text)-1] = 0;

    WinSetWindowText( toDoList.childPane, text );
    delete text;
}


////////////////////////////////////////////////////////////////////////////////
// pickAnEventProc - a day was double clicked.  Now, display a list of all the
//                   events on that day and let the user choose to delete, add
//                   or modify those days' events
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY pickAnEventProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch( msg )
    {
        //----------------------------------------------------------------------
        // Check to see if user double clicked a date entry, modify it if so
        //----------------------------------------------------------------------
        case WM_CONTROL: {
            if ( SHORT2FROMMP(mp1) == LN_ENTER )
                WinPostMsg( hWnd, WM_COMMAND, MPFROMSHORT( MODIFYEVENT ), mp2 );
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        // when the dialog is being initialized, center it on desktop
        //----------------------------------------------------------------------
        case WM_INITDLG: {
            INT tablePtr = 0;
            char tmp[ MAXITEMSIZE+5 ];

            pushStack(snoozeOn);
            snoozeOn = TRUE;

            centerDialog( hWnd );
            tmp[0] = 0;
            WinSendDlgItemMsg( hWnd, CURITEMLIST, LM_DELETEALL, 0, 0 );
            for (REMINDERINDEX z=0; z<numReminders; z++) {
                ReminderItem *remindPtr = reminders[z];
                if ( ((remindPtr->getYear() == curYear) || (remindPtr->getYear() == ALLYEARS)) &&
                     ((remindPtr->getMonth() == curMonth) || (remindPtr->getMonth() == ALLMONTHS)) &&
                      (remindPtr->getDay() == curDay)) {

                    offsetTable[tablePtr++] = z;
                    if ( remindPtr->getToDo() )
                        sprintf( tmp, " * " );
                    else
                        sprintf( tmp, " - " );

                    strcat( tmp, remindPtr->getText() );
                    WinSendDlgItemMsg( hWnd, CURITEMLIST, LM_INSERTITEM, MPFROM2SHORT( LIT_END,0 ), tmp );
                    }
                }

            if (tablePtr)
                WinSendDlgItemMsg( hWnd, CURITEMLIST, LM_SELECTITEM,
                                   MPFROM2SHORT( 0, 0 ), MPFROM2SHORT( TRUE, 0 ) );
            WinEnableControl( hWnd, DELETEEVENT, tablePtr );
            WinEnableControl( hWnd, MODIFYEVENT, tablePtr );
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        // if we receive any system message, dismiss the dialog box
        //----------------------------------------------------------------------
        case WM_COMMAND:
        {
            //------------------------------------------------------------------
            // it was not a user item, process now as usual
            //------------------------------------------------------------------
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // they cancelled the dialog, get rid of it
                //--------------------------------------------------------------
                case WM_CLOSE:
                case DID_OK:
                    WinDismissDlg( hWnd, TRUE );
                    snoozeOn = popStack();
                    return 0;
                //--------------------------------------------------------------
                // user has decided to delete a time entry
                //--------------------------------------------------------------
                case DELETEEVENT: {
                    LONG selected = WinQueryLboxSelectedItem(WinWindowFromID( hWnd, CURITEMLIST ));
                    if (selected != LIT_NONE) {
                        if (askBeforeDeleteItem)
                            if (WinMessageBox( HWND_DESKTOP, hWnd,
                                               MSG014, MSG013, 0,
                                               MB_MOVEABLE|MB_ICONQUESTION|MB_YESNO) == MBID_NO)
                                return 0;

                        INT ptr = offsetTable[selected];
                        delete reminders[ ptr ];
                        for (int z=ptr; z<numReminders-1; z++ )
                            reminders[z] = reminders[z+1];
                        numReminders--;
                        sortTimeEntries();
                        redrawCalendar();                      //fillToDoList();

                        snoozeOn = popStack();
                        WinPostMsg( hWnd, WM_INITDLG, 0, 0 );
                        }
                    return 0;
                    }
                //--------------------------------------------------------------
                // user wants to add a new event to calendar
                //--------------------------------------------------------------
                case NEWEVENT:
                    if ( numReminders == MAXREMINDERS ) {
                        WinMessageBox( HWND_DESKTOP, calendar.frame, MSG035,
                        MSG004, 0, MB_ERROR|MB_OK );
                        return 0;
                        }
                    reminders[numReminders++] = new ReminderItem( 1, 0,
                                                                  daysInAdvance,
                                                                  curMonth,
                                                                  curDay,
                                                                  curYear,
                                                                  0, 0,
                                                                  TRUE, BOTH,
                                                                  FALSE, 0, 3,
                                                                  CHIMEWAV,
                                                                  NEWITEMTEXT,
                                                                  0 );

                    if( modifyEvent( hWnd, numReminders-1 ) == FALSE) {
                        numReminders--;
                        delete reminders[numReminders];
                        }
                    snoozeOn = popStack();
                    WinPostMsg( hWnd, WM_INITDLG, 0, 0 );
                    return 0;
                //--------------------------------------------------------------
                // modify the selected event in the list box
                //--------------------------------------------------------------
                case MODIFYEVENT: {
                    LONG selected = WinQueryLboxSelectedItem(WinWindowFromID( hWnd, CURITEMLIST ));
                    if (selected != LIT_NONE) {
                        ReminderItem *tmp = reminders[offsetTable[selected]];
                        ReminderItem *tmpItem = new ReminderItem(tmp->getToDo(),
                                                                 tmp->getItemDone(),
                                                                 tmp->getNumToShow(),
                                                                 tmp->getMonth(),
                                                                 tmp->getDay(),
                                                                 tmp->getYear(),
                                                                 tmp->getHour(),
                                                                 tmp->getMinute(),
                                                                 tmp->getReminder(),
                                                                 tmp->getReminderType(),
                                                                 tmp->getMinBefore(),
                                                                 tmp->getMinsBefore(),
                                                                 tmp->getDaysBefore(),
                                                                 tmp->getSoundFile(),
                                                                 tmp->getText(),
                                                                 tmp->getReschedTime());

                        if (modifyEvent( hWnd, offsetTable[selected] ) == FALSE) {
                            reminders[offsetTable[selected]] = tmpItem;
                            tmpItem->updateTimes();
                            delete tmp;
                            }
                        else delete tmpItem;
                        }
                    snoozeOn = popStack();
                    WinPostMsg( hWnd, WM_INITDLG, 0, 0 );
                    return 0;
                    }
                //--------------------------------------------------------------
                // if nothing further, pass message onto system
                //--------------------------------------------------------------
                default:
                    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
                }
        }
        //----------------------------------------------------------------------
        // if nothing further we want to intercept, pass message onto system
        //----------------------------------------------------------------------
        default:
            break;
    }
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// Message handler for calendar color options dialog box
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY calColorProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch( msg ) {
        //----------------------------------------------------------------------
        // when initializing dialog, center it sand set up sample calendar draw
        //----------------------------------------------------------------------
        case WM_INITDLG:
            centerDialog( hWnd );
            // subclass the static rectangle control in the dialog box so that
            // we can draw our calendar in that box (calendar preview area)
            oldSampleProc = WinSubclassWindow( WinWindowFromID(hWnd, SAMPLECAL),
                                               sampleCalProc );
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
        //----------------------------------------------------------------------
        // Handle any of the controls within the dialog
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // if any of the color buttons have been pressed, get the new
                // new color from the user using setColor()
                //--------------------------------------------------------------
                case DARKCOLOR:
                    setColor( hWnd, MSG023, &CalButtonDark );
                    return 0;
                case MIDCOLOR:
                    setColor( hWnd, MSG024, &CalButtonMiddle );
                    return 0;
                case LIGHTCOLOR:
                    setColor( hWnd, MSG025, &CalButtonLight );
                    return 0;
                case TEXTCOLOR:
                    setColor( hWnd, MSG026, &CalText );
                    return 0;
                case INCOMPLETETXT:
                    setColor( hWnd, MSG029, &CalHighText );
                    return 0;
                case INCOMPLETEBKG:
                    setColor( hWnd, MSG030, &CalHighlight );
                    return 0;
                case COMPLETEBKG:
                    setColor( hWnd, MSG032, &CalDoneHighlight );
                    return 0;
                case COMPLETETXT:
                    setColor( hWnd, MSG111, &CalDayColor );
                    return 0;
                case QUICKNOTDONE:
                    setColor( hWnd, MSG031, &CalDoneText );
                    return 0;
                case QUICKDONE:
                    setColor( hWnd, MSG028, &CalRemindDone );
                    return 0;
                case OLDBACKCOLOR:
                    setColor( hWnd, MSG132, &oldMonthBack );
                    return 0;
                case OLDTEXTCOLOR:
                    setColor( hWnd, MSG136, &oldMonthText );
                    return 0;
                //--------------------------------------------------------------
                // dismiss dialog if user is done (the caller redraws calendar)
                //--------------------------------------------------------------
                case DID_OK:
                    WinDismissDlg( hWnd, TRUE );
                    break;
                //--------------------------------------------------------------
                // let system handle anything we don't want to handle
                //--------------------------------------------------------------
                default:
                    break;
                }
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        }
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// ChangeColorProc - allows the user to select a new color from 20 of the system
//                   colors.  The selected color is passed back in colorToChange
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY ChangeColorProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch(msg)
    {
        //----------------------------------------------------------------------
        // when the dialog is being initialized, center it on desktop and fill
        // in the appropriate colors in the selection box
        //----------------------------------------------------------------------
        case WM_INITDLG: {
            LONG index = -4;
            LONG clr = colorToChange + 4;

            centerDialog( hWnd );
            for (SHORT row=1; row<=4; row++)
                for (SHORT col=1; col<=5; col++)
                    WinSendDlgItemMsg( hWnd, COLORBOX, VM_SETITEM,
                                       MPFROM2SHORT(row,col), MPFROMLONG(index++));

            WinSendDlgItemMsg( hWnd, COLORBOX, VM_SELECTITEM,
                               MPFROM2SHORT(clr/5+1, clr%5+1),0);
            WinSetWindowText( hWnd, messagePointer );
            return 0;
            }
        //----------------------------------------------------------------------
        // if OK button pressed, send new color back in changeToColor
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            if( SHORT1FROMMP(mp1) == DID_OK ) {
                MRESULT MRet = WinSendDlgItemMsg(hWnd,COLORBOX,VM_QUERYSELECTEDITEM,0,0);
                SHORT row = SHORT1FROMMP(MRet);
                SHORT col = SHORT2FROMMP(MRet);
                colorToChange = ((row-1)*5+(col-1)-4);

                WinDismissDlg( hWnd, TRUE );
                return 0;
                }
            else return WinDefDlgProc(hWnd, msg, mp1, mp2);
            }
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        default:
            return WinDefDlgProc(hWnd, msg, mp1, mp2);
    }
}


////////////////////////////////////////////////////////////////////////////////
// Message handler for to do list options dialog box
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY toDoListOptionsProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch( msg ) {
        //----------------------------------------------------------------------
        // when initializing dialog, center it sand set up show todo list button
        //----------------------------------------------------------------------
        case WM_INITDLG:
            centerDialog( hWnd );
            WinCheckButton( hWnd, SHOWTODOLST, showToDoList );
            if( toDoDisplay == NONE )
                WinCheckButton( hWnd, NOPREFIX, TRUE );
            else if( toDoDisplay == TIMEONLY )
                WinCheckButton( hWnd, TIMEOFEVENT, TRUE );
            else if( toDoDisplay == DATEONLY )
                WinCheckButton( hWnd, DATEOFEVENT, TRUE );
            else WinCheckButton( hWnd, DATEANDTIMEOFEVENT, TRUE );
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
        //----------------------------------------------------------------------
        // Handle any of the controls within the dialog
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // 'change the background color' button was pressed
                //--------------------------------------------------------------
                case BCOLOR: {
                    LONG color = LONGFROMMR( WinSendMsg( toDoList.childPane,
                                             MLM_QUERYBACKCOLOR, 0, 0 ) );
                    setColor( hWnd, MSG021, &color );
                    WinSendMsg( toDoList.childPane, MLM_SETBACKCOLOR,
                                MPFROMLONG( color ), 0 );
                    return 0;
                    }
                //--------------------------------------------------------------
                // 'change the text color' button was pressed
                //--------------------------------------------------------------
                case FCOLOR: {
                    LONG color = LONGFROMMR( WinSendMsg( toDoList.childPane,
                                             MLM_QUERYTEXTCOLOR, 0, 0 ) );
                    setColor( hWnd, MSG022, &color );
                    WinSendMsg( toDoList.childPane, MLM_SETTEXTCOLOR,
                                MPFROMLONG( color ), 0 );
                    return 0;
                    }
                //--------------------------------------------------------------
                // dismiss dialog and redraw real calendar if user is done
                //--------------------------------------------------------------
                case DID_OK:
                    showToDoList = WinQueryButtonCheckstate( hWnd, SHOWTODOLST );
                    if (WinQueryButtonCheckstate( hWnd, NOPREFIX ))
                        toDoDisplay = NONE;
                    else if (WinQueryButtonCheckstate( hWnd, TIMEOFEVENT ))
                        toDoDisplay = TIMEONLY;
                    else if (WinQueryButtonCheckstate( hWnd, DATEOFEVENT ))
                        toDoDisplay = DATEONLY;
                    else toDoDisplay = DATEANDTIME;
                    WinDismissDlg( hWnd, TRUE );
                    WinSetWindowPos( toDoList.frame, HWND_TOP, 0,0,0,0,
                                     showToDoList?SWP_SHOW:SWP_HIDE);
                    break;
                //--------------------------------------------------------------
                // let system handle anything we don't want to handle
                //--------------------------------------------------------------
                default:
                    break;
                }
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        }
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// updateYear - correctly set the year spin button in the edit event dialog
//              so it correctly reflects the state of the event
////////////////////////////////////////////////////////////////////////////////
VOID updateYear( HWND hWnd, ReminderItem *ptr )
{
    char *tmpStr[1];

    WinEnableControl( hWnd, YEAR, TRUE );
    WinEnableControl( hWnd, YEARLABEL, TRUE );
    if (ptr->getYear()==ALLYEARS) {
        tmpStr[0] = new char[8];
        strcpy( tmpStr[0], NO_ITEM );
        WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_SETARRAY, MPFROMLONG(tmpStr), MPFROMLONG(1) );
        WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_SETCURRENTVALUE, 0, 0 );
        WinCheckButton( hWnd, EVERYYEAR, TRUE );
        WinEnableControl( hWnd, YEAR, FALSE );
        WinEnableControl( hWnd, YEARLABEL, FALSE );
        delete tmpStr[0];
        }
    else {
        WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_SETARRAY, 0, 0 );
        WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_SETLIMITS, MPFROMLONG(2200), MPFROMLONG(1900) );
        WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getYear()), 0 );
        WinCheckButton( hWnd, EVERYYEAR, FALSE );
        }
}


////////////////////////////////////////////////////////////////////////////////
// updateReminder - correctly set the reminder area in the edit event dialog
//                  so it correctly reflects the state of the event
////////////////////////////////////////////////////////////////////////////////
VOID updateReminder( HWND hWnd, ReminderItem *ptr )
{
    CHAR state = ptr->getReminder();

    WinCheckButton( hWnd, ptr->getReminderType(), TRUE );
    if ( ptr->getMinBefore() )
        WinCheckButton( hWnd, BUTTON1, TRUE );
    else
        WinCheckButton( hWnd, BUTTON2, TRUE );

    WinEnableControl( hWnd, MINBEFORE, TRUE );
    WinEnableControl( hWnd, DAYSBEFORE, TRUE );
    WinSendMsg( WinWindowFromID( hWnd, MINBEFORE ), SPBM_SETLIMITS, MPFROMLONG(1440), 0 );
    WinSendMsg( WinWindowFromID( hWnd, MINBEFORE ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getMinsBefore()), 0 );
    WinSendMsg( WinWindowFromID( hWnd, DAYSBEFORE ), SPBM_SETLIMITS, MPFROMLONG(30), 0 );
    WinSendMsg( WinWindowFromID( hWnd, DAYSBEFORE ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getDaysBefore()), 0 );
    WinEnableControl( hWnd, MINBEFORE, state );
    WinEnableControl( hWnd, DAYSBEFORE, state );

    WinSetDlgItemText( hWnd, SOUNDFILE, ptr->getSoundFile() );
    WinEnableControl( hWnd, BUTTON1, state );
    WinEnableControl( hWnd, BUTTON2, state );
    WinEnableControl( hWnd, DAYSBEFORELBL, state );
    WinEnableControl( hWnd, MINBEFORELBL, state );
    WinEnableControl( hWnd, VISUALLY, state );
    WinEnableControl( hWnd, AUDIBLY, state );
    WinEnableControl( hWnd, BOTH, state );
    WinEnableControl( hWnd, SELECTSOUNDFILE, state );
    WinEnableControl( hWnd, SOUNDFILE, state );
    WinCheckButton( hWnd, FLASHREMINDER, state );
}


////////////////////////////////////////////////////////////////////////////////
// updateTime - correctly set the time spin buttons in the edit event dialog
//              so it correctly reflects the state of the event
////////////////////////////////////////////////////////////////////////////////
VOID updateTime( HWND hWnd, ReminderItem *ptr )
{
/*
    char *Array[24];
    for (int z=0; z<24; z++)
        Array[z] = new char[4];
    #ifdef ENGLISH
    for (z=0; z<24; z++)
        if (z==0)
            sprintf( Array[z], "12a" );
        else if (z==12)
            sprintf( Array[z], "12p" );
        else if (z<12)
            sprintf( Array[z], "%da", z );
        else
            sprintf( Array[z], "%dp", z-12 );
    #else
    for (z=0; z<24; z++)
        sprintf( Array[z], "%d", z );
    #endif
*/
    WinSendMsg( WinWindowFromID( hWnd, HOUR ), SPBM_SETARRAY, MPFROMLONG(hourArray), MPFROMSHORT(24) );
    WinSendMsg( WinWindowFromID( hWnd, HOUR ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getHour()), 0 );
/*
    for (z=0; z<24; z++)
        delete Array[z];
*/
}


////////////////////////////////////////////////////////////////////////////////
// updateMonth - correctly set the month spin button in the edit event dialog
//               so it correctly reflects the state of the event
////////////////////////////////////////////////////////////////////////////////
VOID updateMonth( HWND hWnd, ReminderItem *ptr )
{
    char *Months[12];
    struct tm curDate;

    WinEnableControl( hWnd, MONTH, TRUE );
    WinEnableControl( hWnd, MONTHLABEL, TRUE );
    if (ptr->getMonth()==ALLMONTHS) {
        Months[0] = new char[9];
        strcpy( Months[0], NO_ITEM );
        strcat( Months[0], NO_ITEM );
        WinSendMsg( WinWindowFromID( hWnd, MONTH ), SPBM_SETARRAY, MPFROMLONG(Months), MPFROMSHORT(1) );
        WinSendMsg( WinWindowFromID( hWnd, MONTH ), SPBM_SETCURRENTVALUE, MPFROMLONG(0), MPFROMLONG(0) );
        WinCheckButton( hWnd, EVERYMONTH, TRUE );
        WinEnableControl( hWnd, MONTH, FALSE );
        WinEnableControl( hWnd, MONTHLABEL, FALSE );
        delete Months[0];
        }
    else {
        for (int z=0; z<12; z++) {
            curDate.tm_mon = z;
            Months[z] = new char[24];
            strftime( Months[z], 24, "%B", (const tm*)&curDate );
            }
        WinSendMsg( WinWindowFromID( hWnd, MONTH ), SPBM_SETARRAY, MPFROMLONG(Months), MPFROMSHORT(12) );
        WinSendMsg( WinWindowFromID( hWnd, MONTH ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getMonth()-1), MPFROMLONG(0) );
        for (z=0; z<12; z++)
            delete Months[z];
        WinCheckButton( hWnd, EVERYMONTH, FALSE );
        }
    if (ptr->getYear()==ALLYEARS)
        WinSendMsg( WinWindowFromID( hWnd, DAY ), SPBM_SETLIMITS, MPFROMLONG(numberOfDaysInMonth(ptr->getMonth(), curYear)), MPFROMLONG(1) );
    else
        WinSendMsg( WinWindowFromID( hWnd, DAY ), SPBM_SETLIMITS, MPFROMLONG(numberOfDaysInMonth(ptr->getMonth(), ptr->getYear())), MPFROMLONG(1) );
}


////////////////////////////////////////////////////////////////////////////////
// starts a timer to signal our program when the next minute occurs
////////////////////////////////////////////////////////////////////////////////
VOID restartTimer( VOID )
{
    time_t timer = time(NULL);
    time_now = localtime(&timer);
    minuteTimer = startMinuteTimer( (60-(time_now->tm_sec))*TICKSPERSEC );
    resyncTimer = TRUE;
}


////////////////////////////////////////////////////////////////////////////////
// playSound - play a .WAV file sent to us as the parameter.
////////////////////////////////////////////////////////////////////////////////
VOID playSound( CHAR* soundFile )
{
    APIRET rc;
    char *mesg = new char[ strlen(soundFile)+512 ];
    RXSTRING INSTORE[2];
    RXSTRING retstr;
    SHORT ReturnCode;
    LONG offset = 0;

    if (soundFile == 0)
        return;
    if (soundFile[0] == 0) {
        WinAlarm( HWND_DESKTOP, WA_NOTE );
        return;
        }

    MakeHourglassPointer ();
    offset =  sprintf(mesg, "/**/\r\nrc=call RxFuncAdd('mciRxInit','MCIAPI',,\r\n'mciRxInit')\r\nrc=call mciRxInit()\r\nrc=mciRxSendString('open waveaudio',\r\n'alias wave shareable wait',,\r\n'RetStr','0','0')\r\n");
    offset += sprintf(mesg+offset, "rc=mciRxSendString('load wave %s wait',,\r\n'RetStr','0','0')\r\n", soundFile );
    strcat( mesg, "rc=mciRxSendString('play wave wait',,\r\n'RetStr','0','0')\r\nrc=mciRxSendString('close wave wait',,\r\n'RetStr','0','0')\r\ncall mciRxExit\r\nexit\r\n\x1A\0");

    INSTORE[0].strptr=mesg;
    INSTORE[0].strlength=strlen(mesg);
    INSTORE[1].strptr=NULL;
    INSTORE[1].strlength=0;

    rc = rexxStart((LONG)0,
              (PRXSTRING)0,
              (PSZ)LAUNCHFILE,
              (PRXSTRING)INSTORE,
              (PSZ)"CMD",
              (LONG)RXCOMMAND,
              (PRXSYSEXIT)0,
              (SHORT*)&ReturnCode,
              (PRXSTRING)&retstr);

    // if there was an error (ie: no MMPM), make a system beep
    if (rc)
        WinAlarm( HWND_DESKTOP, WA_NOTE );

    delete mesg;
    MakeArrowPointer ();                      // switch back to arrow pointer,
}


////////////////////////////////////////////////////////////////////////////////
// This is the message processing facility for the main application thread
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY ClientWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch(msg) {
        //----------------------------------------------------------------------
        // the application has been designed such that a WM_TIMER is sent by the
        // system every minute (so that we can present reminders, if necessary
        //----------------------------------------------------------------------
        case WM_TIMER: {
            time_t timer = time(NULL);
            time_now = localtime(&timer);
            LONG timeNow = calcTimeOfOccurence( time_now->tm_year + 1900,
                                                time_now->tm_mon + 1,
                                                time_now->tm_mday,
                                                time_now->tm_hour,
                                                time_now->tm_min );

            if (showClock) {
                RECTL rect;
                HPS hps = WinGetPS( hWnd );
                WinQueryWindowRect( hWnd, &rect );
                drawClock( hps, hWnd, 0, 0, 0 );
                WinReleasePS( hps );
                }

            if (resyncTimer) {
                stopMinuteTimer();
                minuteTimer = startMinuteTimer( TICKSPERMIN );
                resyncTimer = FALSE;
                }

            // if we hit midnight, move pointer to new day
            if ((time_now->tm_hour==0) && (time_now->tm_min==0)) {
                getdate( &todaysDate );
                redrawCalendar();
                }

            //------------------------------------------------------------------
            //------------------------------------------------------------------
            //------------------------------------------------------------------
            //------------------------------------------------------------------
            if (snoozeOn)
                DosBeep(100,100);
            //------------------------------------------------------------------
            //------------------------------------------------------------------
            //------------------------------------------------------------------
            //------------------------------------------------------------------
            if (!snoozeOn)
                for( INT z=0; z<numReminders; z++ ) {
                    ReminderItem *ptr = reminders[z];
                    if ((ptr->getReminder()) && (!ptr->getItemDone()))
                        if (ptr->getReminderTime() <= timeNow) {
                            eventReminder = z;

                            if (reminders[eventReminder]->getReminderType() == AUDIBLY) {
                                playSound( reminders[eventReminder]->getSoundFile() );
                                reminders[eventReminder]->setItemDone( TRUE );
                                }
                            else WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)reminderProc, 0,
                                           REMINDER_DIALOG, (PVOID)NULL);
                            }
                    }

            return WinDefWindowProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        // the presentation parameters have changed.  If the current font has
        // changed, redraw the calendar to show this change
        //----------------------------------------------------------------------
        case WM_PRESPARAMCHANGED: {
            if ( LONGFROMMP(mp1) == PP_FONTNAMESIZE )
                redrawCalendar();
            return WinDefWindowProc(hWnd, msg, mp1, mp2);
            }
        //----------------------------------------------------------------------
        // handle QuickReminder function (show a synopsis of first couple of
        // items scheduled for each day -- if enabled)
        //----------------------------------------------------------------------
        case WM_MOUSEMOVE: {
            char tmpBuffer[ MAXITEMSIZE+5 ];
            INT totDay;
            CHAR nowMonth;
            SHORT nowYear;
            RECTL rect, rect2;
            POINTL pt;
            INT xPos = (SHORT1FROMMP( mp1 )-calendarRect.xLeft) / (calWidth/7);
            INT yPos = (SHORT2FROMMP( mp1 )-calendarRect.yBottom) / (calLength/6);

            if ((!calQuickRemind) || (SHORT2FROMMP( mp1 ) < calendarRect.yBottom) ||
                (SHORT2FROMMP( mp1 ) > calendarRect.yTop))
                return WinDefWindowProc(hWnd, msg, mp1, mp2);
            else if (yPos > 5)
                yPos = 5;

            yPos = 5 - yPos;
            totDay = yPos*7 + xPos + calStartIndex + 1;
            if (((!showAllDays) && ((totDay < 1) || (totDay > totalDaysInMonth))) ||
               ((yPos==5) && (xPos>1)))
                return WinDefWindowProc(hWnd, msg, mp1, mp2);

            if (totDay == prevDay)
                return WinDefWindowProc(hWnd, msg, mp1, mp2);
            prevDay = totDay;

            nowMonth = curMonth;
            nowYear = curYear;

            if ((totDay < 1) || (totDay > totalDaysInMonth)) {
                if (totDay<1) {
                    nowMonth--;
                    if (nowMonth<1) {
                        nowMonth = 12;
                        nowYear--;
                        }
                    totDay = totDay+numberOfDaysInMonth( nowMonth, nowYear );
                    }
                else {
                    nowMonth++;
                    if (nowMonth>12) {
                        nowMonth = 1;
                        nowYear++;
                        }
                    totDay = totDay-totalDaysInMonth;
                    }
                }


            //------------------------------------------------------------------
            // clear the area and print the date the mouse is positioned over
            //------------------------------------------------------------------
            HPS hps = WinGetPS( calendar.client );
            GpiSetColor( hps, CalButtonMiddle );
            pt.x = reminderRect.xLeft;
            pt.y = reminderRect.yBottom;
            GpiMove( hps, &pt );
            pt.x = reminderRect.xRight;
            pt.y = reminderRect.yTop;
            GpiBox( hps, DRO_FILL, &pt, 0, 0);

            rect = reminderRect;
            sprintf( tmpBuffer, "(%d/%d)  ", nowMonth,totDay);
            WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect, CalText, CalButtonMiddle, DT_WORDBREAK);
            WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect, 0, 0, DT_QUERYEXTENT);
            rect.xLeft = rect.xRight;
            rect.xRight = reminderRect.xRight;

            //------------------------------------------------------------------
            // print as many of the events for this day as we have room for
            //------------------------------------------------------------------
            INT z=0;
            while ((z<numReminders) && (rect.yTop>0)) {
                ReminderItem *ptr = reminders[z];
                if ( ((ptr->getYear() == nowYear) || (ptr->getYear() == ALLYEARS)) &&
                     ((ptr->getMonth() == nowMonth) || (ptr->getMonth() == ALLMONTHS)) &&
                      (ptr->getDay() == totDay))
                    if ((!ptr->getItemDone()) || (remindEvenIfDone)) {
                        strncpy( tmpBuffer, ptr->getText(), MAXITEMSIZE+5 );
                        rect2 = rect;
                        WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect, ptr->getItemDone()?CalRemindDone:CalText, CalButtonMiddle, DT_WORDBREAK);
                        WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect, 0, 0, DT_QUERYEXTENT );
                        rect2.yTop = rect.yBottom;
                        rect2.yBottom = rect2.yTop-(rect.yTop-rect.yBottom);
                        rect = rect2;
                        }
                z++;
                }
            WinReleasePS( hps );

            return WinDefWindowProc(hWnd, msg, mp1, mp2);
            }
        //----------------------------------------------------------------------
        // if button 1 double clicked, edit a day's events
        //----------------------------------------------------------------------
        case WM_BUTTON1DBLCLK: {
            INT xPos = (SHORT1FROMMP( mp1 )-calendarRect.xLeft) / (calWidth/7);
            INT yPos = (SHORT2FROMMP( mp1 )-calendarRect.yBottom) / (calLength/6);
            SHORT oldYear = curYear;
            CHAR oldMonth = curMonth;
            CHAR oldDay = curDay;

            if (SHORT2FROMMP(mp1) > calBottom)
                return WinDefWindowProc(hWnd, msg, mp1, mp2);

            if (yPos > 5)
                yPos = 5;
            yPos = 5 - yPos;

            curDay = yPos*7 + xPos + calStartIndex + 1;
            //if ((curDay < 1) || (curDay > totalDaysInMonth))
            //    return WinDefWindowProc(hWnd, msg, mp1, mp2);
            if (((!showAllDays) && ((curDay < 1) || (curDay > totalDaysInMonth))) ||
               ((yPos==5) && (xPos>1)))
                return WinDefWindowProc(hWnd, msg, mp1, mp2);

            if ((curDay < 1) || (curDay > totalDaysInMonth)) {
                if (curDay<1) {
                    curMonth--;
                    if (curMonth<1) {
                        curMonth = 12;
                        curYear--;
                        }
                    curDay = curDay+numberOfDaysInMonth( curMonth, curYear );
                    }
                else {
                    curMonth++;
                    if (curMonth>12) {
                        curMonth = 1;
                        curYear++;
                        }
                    curDay = curDay-totalDaysInMonth;
                    }
                }

            stopMinuteTimer();
            WinDlgBox( HWND_DESKTOP, hWnd, (PFNWP)pickAnEventProc, 0,
                       PICKANEVENT, (PVOID)0 );

            curYear = oldYear;
            curMonth = oldMonth;
            curDay = oldDay;

            redrawCalendar();
            restartTimer();
            return 0;
            }
        //----------------------------------------------------------------------
        // mouse button 3 engages pop up menu
        //----------------------------------------------------------------------
        case WM_BUTTON3MOTIONSTART: {
            POINTL pointData;
            WinQueryPointerPos( HWND_DESKTOP, &pointData );
            WinPopupMenu( HWND_DESKTOP, hWnd, popUpMenu, pointData.x,
                          pointData.y, 0, PU_VCONSTRAIN|PU_HCONSTRAIN|PU_MOUSEBUTTON3|PU_MOUSEBUTTON3DOWN);
            return WinDefWindowProc(hWnd, msg, mp1, mp2);
            }
        //----------------------------------------------------------------------
        // mouse button 3 engages pop up menu
        //----------------------------------------------------------------------
        case WM_BUTTON3CLICK: {
            POINTL pointData;
            WinQueryPointerPos( HWND_DESKTOP, &pointData );
            WinPopupMenu( HWND_DESKTOP, hWnd, popUpMenu, pointData.x,
                          pointData.y, 0, PU_VCONSTRAIN|PU_HCONSTRAIN|PU_MOUSEBUTTON1|PU_MOUSEBUTTON2|PU_MOUSEBUTTON3|PU_KEYBOARD);
            return WinDefWindowProc(hWnd, msg, mp1, mp2);
            }
        //----------------------------------------------------------------------
        // single click of button 1 moves calendar date
        //----------------------------------------------------------------------
        case WM_BUTTON1CLICK: {
            INT xPos = SHORT1FROMMP( mp1 );
            INT yPos = SHORT2FROMMP( mp1 );

            if ((xPos >= (calWidth*.95)) && (xPos <= (calWidth*.97))
                && (yPos >= calBottom+(calHeight*.10)) && (yPos <= calBottom+(calHeight*.90))) {
                // they want to go forward a month
                curMonth ++;
                if (curMonth == 13) {
                    curMonth = 1;
                    curYear++;
                    }
                redrawCalendar();
                return WinDefWindowProc(hWnd, msg, mp1, mp2);
                }
            else if ((xPos >= (calWidth*.88)) && (xPos <= (calWidth*.90))
                && (yPos >= calBottom+(calHeight*.10)) && (yPos <= calBottom+(calHeight*.90))) {
                // they want to go back a month
                curMonth --;
                if (curMonth == 0) {
                    curMonth = 12;
                    curYear--;
                    }
                redrawCalendar();
                return WinDefWindowProc(hWnd, msg, mp1, mp2);
                }
            else if ((xPos >= (calWidth*.91)) && (xPos <= (calWidth*.94))
                && (yPos >= calBottom+(calHeight*.10)) && (yPos <= calBottom+(calHeight*.90))) {
                // they want to return to current date
                curMonth=todaysDate.da_mon;
                curYear=todaysDate.da_year;
                redrawCalendar();
                return WinDefWindowProc(hWnd, msg, mp1, mp2);
                }
            return WinDefWindowProc(hWnd, msg, mp1, mp2);
            }
        //----------------------------------------------------------------------
        // if button 2 double clicked, toggle display of window controls
        //----------------------------------------------------------------------
        case WM_BUTTON2DBLCLK: {
            calShowControls = !calShowControls;
            ShowControls( calShowControls, calendar );
            return WinDefWindowProc(hWnd, msg, mp1, mp2);
            }
        //----------------------------------------------------------------------
        // if button 2 is held down, move the window
        //----------------------------------------------------------------------
        case WM_BUTTON2DOWN: {
            moveWindow( calendar );
            return WinDefWindowProc(hWnd, msg, mp1, mp2);
            }
        //----------------------------------------------------------------------
        // if window needs repainting, call our drawCalender function to do it
        //----------------------------------------------------------------------
        case WM_PAINT: {
            RECTL updateRect;
            HPS hps = WinBeginPaint( hWnd, (HPS)NULL, &updateRect);
            drawCalendar( hps, hWnd, updateRect );
            WinEndPaint( hps );
            return 0;
            }
        //----------------------------------------------------------------------
        // quit application
        //----------------------------------------------------------------------
        case WM_CLOSE: {
            if (!writeOptionFile())
                return 0;
            return WinDefWindowProc(hWnd, msg, mp1, mp2);  // default processing
            }
        //----------------------------------------------------------------------
        // process all WM_COMMAND messages below:
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            //------------------------------------------------------------------
            // it was not a user item, process now as usual
            //------------------------------------------------------------------
            switch( SHORT1FROMMP(mp1) ) {
/*
                //--------------------------------------------------------------
                //--------------------------------------------------------------
                case TESTFUNCTION: {
                    char txt[512];
                    sprintf( txt, "snoozeOn = %s", ((snoozeOn==TRUE)?"YES":"NO") );
                    WinMessageBox( HWND_DESKTOP, hWnd, txt, 0, 0,
                                   MB_INFORMATION|MB_OK );
                    return 0;
                    }
*/
                //--------------------------------------------------------------
                //--------------------------------------------------------------
                case PRINT: {
                    //print(80,1,FALSE);
                    WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)printProc, 0,
                              PRINTDIALOG, (PVOID)NULL);
                    //print(80,1,TODOTODAY);
                    //print(80,1,TODOMONTH);
                    //print(80,1,TODOYEAR);
                    return 0;
                    }
                //--------------------------------------------------------------
                //--------------------------------------------------------------
                case JUMPTO: {
                    WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)jumpToProc, 0,
                              JUMPTODATE, (PVOID)NULL);
                    redrawCalendar();
                    return 0;
                    }
                //--------------------------------------------------------------
                // save calendar
                //--------------------------------------------------------------
                case SAVECAL: {
                    char *txt;
                    if ( writeOptionFile() ) {
                        txt = new char[sizeof(MSG018)+(calendarFile?strlen(calendarFile):sizeof(FILENAME))+2];
                        sprintf( txt, MSG018, (calendarFile?calendarFile:FILENAME) );
                        WinMessageBox( HWND_DESKTOP, hWnd, txt, MSG007, 0,
                                       MB_INFORMATION|MB_OK );
                        }
                    else {
                        txt = new char[sizeof(MSG019)+(calendarFile?strlen(calendarFile):sizeof(FILENAME))+4];
                        sprintf( txt, MSG019, (calendarFile?calendarFile:FILENAME) );
                        WinMessageBox( HWND_DESKTOP, calendar.frame, txt,
                                       MSG007, 0, MB_ERROR|MB_OK );
                        }
                    delete txt;
                    return WinDefWindowProc(hWnd, msg, mp1, mp2);
                    }
                //--------------------------------------------------------------
                // show about box
                //--------------------------------------------------------------
                case ABOUT1: {
                    WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)calColorProc, 0,
                              ABOUT1, (PVOID)NULL);
                    redrawCalendar();
                    return 0;
                    }
/*
                //--------------------------------------------------------------
                // This in uncommented and enalbed on menubar for debugging purposes only
                //--------------------------------------------------------------
                // show all the reminders we currently have on file
                //--------------------------------------------------------------
                case SHOWREMINDERS: {
                    char tmp[MAXITEMSIZE+24];
                    char *tmp2 = new char[ (MAXITEMSIZE+24)*(numReminders+1) ];
                    tmp2[0] = 0;

                    for (int z=0; z<numReminders; z++) {
                        sprintf(tmp,"%2d/%2d/%4d %2d:%2d >> %s\n",
                                reminders[z]->getMonth(), reminders[z]->getDay(),
                                reminders[z]->getYear(),
                                reminders[z]->getHour(), reminders[z]->getMinute(),
                                reminders[z]->getText() );
                        strcat(tmp2, tmp );
                        }
                    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, tmp2,
                                   MSG034,0, MB_INFORMATION|MB_MOVEABLE|MB_OK );
                    delete tmp2;
                    return WinDefWindowProc(hWnd, msg, mp1, mp2);
                    }
*/
                //--------------------------------------------------------------
                // bring up the calender options... dialog box
                //--------------------------------------------------------------
                case CALOPTIONS: {
                    WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)calDrawOptionsProc,
                              0, CALDRAWOPTIONS, (PVOID)NULL);
                    redrawCalendar();
                    return 0;
                    }
                //--------------------------------------------------------------
                // bring up the calender options... dialog box
                //--------------------------------------------------------------
                case TODOOPTIONS: {
                    WinDlgBox(HWND_DESKTOP,
                              hWnd,
                              (PFNWP)toDoListOptionsProc,
                              0,
                              TODOLISTOPTIONS,
                              (PVOID)NULL);
                    fillToDoList();
                    return 0;
                    }
                //--------------------------------------------------------------
                // bring up the reminder/misc options... dialog box
                //--------------------------------------------------------------
                case REMINDEROPTIONS:
                    return (VOID*)WinDlgBox(HWND_DESKTOP,
                                            hWnd,
                                            (PFNWP)reminderOptionsProc,
                                            0,
                                            REMINDOPTIONS,
                                            (PVOID)NULL);
                //--------------------------------------------------------------
                // user wants to start a new calendar
                //--------------------------------------------------------------
                case NEWCAL:
                    if (WinMessageBox( HWND_DESKTOP, hWnd,
                                       MSG015, MSG007, 0,
                                       MB_MOVEABLE|MB_ICONQUESTION|MB_YESNO) == MBID_YES) {
                        initCalendar();
                        redrawCalendar();
                        WinMessageBox( HWND_DESKTOP, hWnd, MSG033, MSG007, 0,
                                       MB_INFORMATION|MB_OK );
                        }
                    return 0;
                //--------------------------------------------------------------
                // open a calendar file...
                //--------------------------------------------------------------
                case OPENCAL: {
                    HWND hwndDialog;
                    CHAR *oldDirectory = 0;

                    fileDlgInfo.fl = FDS_OPEN_DIALOG|FDS_CENTER;
                    fileDlgInfo.pszTitle = MSG016;

                    setItem( &oldDirectory, fileDlgInfo.szFullFile );
                    strcat(fileDlgInfo.szFullFile, "*.CAL\0");

                    hwndDialog = WinFileDlg( HWND_DESKTOP, hWnd, &fileDlgInfo );
                    if (hwndDialog && (fileDlgInfo.lReturn == DID_OK)) {
                        writeOptionFile();
                        setItem( &calendarFile, fileDlgInfo.szFullFile );
                        resetFileDialog();
                        makeBackupFile();
                        readOptionFile();
                        WinSetWindowPos( toDoList.frame, 0, 0, 0, 0, 0, (minimizeToDo?SWP_MINIMIZE:0) );
                        WinSetWindowPos( calendar.frame, 0, 0, 0, 0, 0, (minimizeCal?SWP_MINIMIZE:0) );
                        redrawCalendar();
                        }
                    else
                        if (oldDirectory)
                            strcpy( fileDlgInfo.szFullFile, oldDirectory );
                        else
                            fileDlgInfo.szFullFile[0] = 0;
                    if (oldDirectory)
                        delete oldDirectory;
                    return 0;
                    }
                //--------------------------------------------------------------
                // save calendar as...
                //--------------------------------------------------------------
                case SAVECALAS: {
                    HWND hwndDialog;
                    CHAR *oldDirectory = 0;

                    fileDlgInfo.fl = FDS_SAVEAS_DIALOG|FDS_CENTER;
                    fileDlgInfo.pszTitle = MSG017;

                    setItem( &oldDirectory, fileDlgInfo.szFullFile );
                    strcat(fileDlgInfo.szFullFile, "*.CAL\0");

                    hwndDialog = WinFileDlg( HWND_DESKTOP, hWnd, &fileDlgInfo );
                    if (hwndDialog && (fileDlgInfo.lReturn == DID_OK)) {
                        char *txt;
                        setItem( &calendarFile, fileDlgInfo.szFullFile );
                        if ( writeOptionFile() ) {
                            txt = new char[sizeof(MSG018)+(calendarFile?strlen(calendarFile):sizeof(FILENAME))+2];
                            sprintf( txt, MSG018, (calendarFile?calendarFile:FILENAME) );
                            WinMessageBox( HWND_DESKTOP, hWnd, txt, MSG017, 0,
                                           MB_INFORMATION|MB_OK );
                            }
                        else {
                            txt = new char[sizeof(MSG019)+(calendarFile?strlen(calendarFile):sizeof(FILENAME))+4];
                            sprintf( txt, MSG019, (calendarFile?calendarFile:FILENAME) );
                            WinMessageBox( HWND_DESKTOP, hWnd, txt, MSG007, 0,
                                           MB_ERROR|MB_OK );
                            }
                        delete txt;
                        resetFileDialog();
                        }
                    else
                        if (oldDirectory)
                            strcpy( fileDlgInfo.szFullFile, oldDirectory );
                        else
                            fileDlgInfo.szFullFile[0] = 0;
                    if (oldDirectory)
                        delete oldDirectory;
                    return 0;
                    }
                //--------------------------------------------------------------
                // if OS/2 wants to kill us, exit cleanly through a WM_CLOSE msg
                //--------------------------------------------------------------
                case EXIT:
                    WinPostMsg( calendar.frame, WM_CLOSE, 0, 0 );
                    return 0;
                //--------------------------------------------------------------
                // let system handle anything we don't want to handle
                //--------------------------------------------------------------
                default:
                    return WinDefWindowProc(hWnd, msg, mp1, mp2);
                }
            }
        //----------------------------------------------------------------------
        // let system handle anything we don't want to handle
        //----------------------------------------------------------------------
        default:
            return WinDefWindowProc(hWnd, msg, mp1, mp2);  // default processing
        }
}


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
VOID drawCalendar( HPS hps, HWND hWnd, RECTL updateRect )
{
    CHAR tmpBuffer[128];
    CHAR month[32];
    ULONG flag;
    RECTL rect, rect2;
    float width, length;
    POINTL pt,pt2;
    int bottom, currentDay, height, z,z1;
    struct tm curDate;

    curDate.tm_year = curYear-1900;
    curDate.tm_mon = curMonth-1;
    curDate.tm_mday = 1;
    curDate.tm_hour = 0;
    curDate.tm_min = 0;
    curDate.tm_sec = 0;
    curDate.tm_isdst = -1;

    totalDaysInMonth = (INT)numberOfDaysInMonth( curMonth, curYear );
    if (mktime(&curDate) == -1)
        DosBeep(300,300);

    WinFillRect( hps, &updateRect, CalButtonMiddle );
    WinQueryWindowRect( hWnd, &rect );

    if (abbreviateMonth)
        strftime( tmpBuffer, 128, MONTHSTRING2, (const tm*)&curDate );
    else
        strftime( tmpBuffer, 128, MONTHSTRING, (const tm*)&curDate );
    rect2 = rect;
    WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect, CalText,
                 CalButtonMiddle, DT_CENTER);
    WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect2, 0, 0,
                 DT_QUERYEXTENT);

    height = abs(rect2.yTop - rect2.yBottom-1);
    bottom = rect2.yBottom;
    rect.yTop = rect2.yBottom-1;

    drawClock( hps, hWnd, 0, 0, 0 );

    if (labelDaysOfWeek) {
        int width = rect.xRight - rect.xLeft;
        struct tm curDate2;
        RECTL rect3;
        rect3.yTop = rect2.yBottom;
        rect3.yBottom = rect3.yTop - height;

        curDate2.tm_wday = 0;
        for (z=0; z<7; z++) {
            rect3.xLeft = ((width/7)*z)+1;
            rect3.xRight = rect3.xLeft + (width/7)-2;
            if (abbreviateWeekDays)
                strftime( tmpBuffer, 128, "%a", (const tm*)&curDate2 );
            else
                strftime( tmpBuffer, 128, "%A", (const tm*)&curDate2 );

            WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect3, CalText,
                         CalButtonMiddle, DT_CENTER);
            curDate2.tm_wday++;
            }
        rect.yTop -= height;
        }

    calendarRect = rect;
    width = rect.xRight - rect.xLeft;
    length = rect.yTop - rect.yBottom;

    calWidth = width;
    calLength = length;
    calBottom = bottom;
    calHeight = height;

    reminderRect.yBottom = 0;
    reminderRect.yTop = (length/6)-3;
    reminderRect.xLeft = (width/7)*2+3;
    reminderRect.xRight = width;

    POLYGON polygonArray[3];
    POINTL poly3[6] = { {(width*.89), bottom+(height*.20)}, {(width*.88), bottom+(height*.50)},
                        {(width*.89), bottom+(height*.80)}, {(width*.90), bottom+(height*.80)},
                        {(width*.90), bottom+(height*.20)}, {(width*.89), bottom+(height*.20)}};

    POINTL poly2[6] = { {(width*.93), bottom+(height*.20)}, {(width*.92), bottom+(height*.20)},
                        {(width*.91), bottom+(height*.50)}, {(width*.92), bottom+(height*.80)},
                        {(width*.93), bottom+(height*.80)}, {(width*.94), bottom+(height*.50)}};

    POINTL poly1[6] = { {(width*.96), bottom+(height*.20)}, {(width*.97), bottom+(height*.50)},
                        {(width*.96), bottom+(height*.80)}, {(width*.95), bottom+(height*.80)},
                        {(width*.95), bottom+(height*.20)}, {(width*.96), bottom+(height*.20)}};

    polygonArray[0].ulPoints = 6;
    polygonArray[0].aPointl = poly1;
    polygonArray[1].ulPoints = 6;
    polygonArray[1].aPointl = poly2;
    polygonArray[2].ulPoints = 6;
    polygonArray[2].aPointl = poly3;
    GpiSetColor( hps, CalButtonDark );
    GpiPolygons( hps, 3, polygonArray, POLYGON_NOBOUNDARY|POLYGON_WINDING,
                 POLYGON_INCL );

    flag = DT_LEFT;
    if (calDaysAlignment == RIGHT)
        flag = DT_RIGHT;
    if (calDaysAlignment == CENTER)
        flag = DT_CENTER|DT_VCENTER;

    //--------------------------------------------------------------------------
    //
    //--------------------------------------------------------------------------
    for (z=0; z<32; z++)
        month[z] = 0;
    for (z=0; z<numReminders; z++) {
        if ( (reminders[z]->getYear()==ALLYEARS) || (reminders[z]->getYear()==curYear) )
            if ( (reminders[z]->getMonth()==ALLMONTHS) || (reminders[z]->getMonth()==curMonth) )
                if (reminders[z]->getItemDone())
                    month[reminders[z]->getDay()-1] = MAX(month[reminders[z]->getDay()-1],1);
                else
                    month[reminders[z]->getDay()-1] = 2;
        }


    //--------------------------------------------------------------------------
    //
    //--------------------------------------------------------------------------
    currentDay = -curDate.tm_wday;
    calStartIndex = currentDay;
    for (z=0; z<6; z++)
            for (z1=0; z1<7; z1++) {
                currentDay++;
                if ((((currentDay>0) && (currentDay<=totalDaysInMonth)) || (showAllDays)) && (!((z==5) && (z1>1)))) {
                    int mon = curMonth;
                    int yr = curYear;

                    rect2.xLeft =  rect.xLeft+(width/7*z1)+1;
                    rect2.xRight = rect2.xLeft+(width/7)-1;
                    rect2.yTop =    rect.yTop-(length/6*z)-1;
                    rect2.yBottom = rect2.yTop-(length/6)+1;

                    //----------------------------------------------------------
                    // draw days for previous/following month
                    //----------------------------------------------------------
                    if ((currentDay<1) || (currentDay>totalDaysInMonth)) {
                        if (currentDay<1) {
                            mon--;
                            if (mon<1) {
                                mon=12;
                                yr--;
                                }
                            sprintf( tmpBuffer, "%2d", currentDay+numberOfDaysInMonth( mon, yr ) );
                            }
                        else {
                            mon++;
                            if (mon>12) {
                                mon=1;
                                yr++;
                                }
                            sprintf( tmpBuffer, "%2d", (currentDay-totalDaysInMonth) );
                            }
                        WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect2,
                                     oldMonthText, oldMonthBack, flag|DT_ERASERECT);
                        }
                    //----------------------------------------------------------
                    // draw days for this month
                    //----------------------------------------------------------
                    else {
                        sprintf( tmpBuffer, "%2d", currentDay );
                        if ((month[currentDay-1])==1) {
                            WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect2,
                                         CalDoneText, CalDoneHighlight, flag|DT_ERASERECT);
                            }
                        else if ((month[currentDay-1])==2) {
                            WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect2,
                                         CalHighText, CalHighlight, flag|DT_ERASERECT);
                            }
                        else
                            WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect2,
                                         CalText, CalButtonMiddle, flag);
                        }

                    if (calDrawDayBorder)
                        if (calDaysAlignment != CENTER) {
                            WinDrawText( hps, strlen(tmpBuffer), tmpBuffer,
                                         &rect2, 0, 0, DT_QUERYEXTENT);

                            //--------------------------------------------------
                            GpiSetColor( hps, calButtonShadows?CalButtonLight:CalButtonMiddle );
                            pt.x = rect2.xLeft-1-1; pt.y = rect2.yTop+1-1;
                            if (calDaysAlignment == RIGHT)
                                pt.x = pt.x + (width/7)-(rect2.xRight-rect2.xLeft)-2;
                            GpiMove( hps, &pt );

                            pt.x = rect2.xRight+1-1; pt.y = rect2.yBottom-1-1;
                            if (calDaysAlignment == RIGHT)
                                pt.x = pt.x + (width/7)-(rect2.xRight-rect2.xLeft)-2;
                            GpiBox( hps, DRO_OUTLINE, &pt, 0, 0 );
                            //--------------------------------------------------
                            GpiSetColor( hps, calButtonShadows?CalButtonDark:CalButtonMiddle );
                            pt.x = rect2.xLeft-1+1; pt.y = rect2.yTop+1+1;
                            if (calDaysAlignment == RIGHT)
                                pt.x = pt.x + (width/7)-(rect2.xRight-rect2.xLeft)-2;
                            GpiMove( hps, &pt );

                            pt.x = rect2.xRight+1+1; pt.y = rect2.yBottom-1+1;
                            if (calDaysAlignment == RIGHT)
                                pt.x = pt.x + (width/7)-(rect2.xRight-rect2.xLeft)-2;
                            GpiBox( hps, DRO_OUTLINE, &pt, 0, 0 );
                            //--------------------------------------------------
                            GpiSetColor( hps, calButtonShadows?CalButtonMiddle:CalButtonDark );
                            pt.x = rect2.xLeft-1; pt.y = rect2.yTop+1;
                            if (calDaysAlignment == RIGHT)
                                pt.x = pt.x + (width/7)-(rect2.xRight-rect2.xLeft)-2;
                            GpiMove( hps, &pt );

                            pt.x = rect2.xRight+1; pt.y = rect2.yBottom-1;
                            if (calDaysAlignment == RIGHT)
                                pt.x = pt.x + (width/7)-(rect2.xRight-rect2.xLeft)-2;
                            GpiBox( hps, DRO_OUTLINE, &pt, 0, 0 );
                            }
                        }
                    }

    //--------------------------------------------------------------------------
    //
    //--------------------------------------------------------------------------
    if (drawCalBorderLines) {
        GpiSetColor( hps, calButtonShadows?CalButtonLight:CalButtonMiddle );
        for( z=0; z<8; z++ ) {
            pt.x = rect.xLeft+(width/7*z)-1;
            pt.y = rect.yTop;
            GpiMove( hps, &pt );
            if (z<3)
                pt.y = rect.yBottom;
            else
                pt.y = rect.yBottom + (length/6);
            GpiLine( hps, &pt );
            }
        for( z=0; z<7; z++ ) {
            pt.y = rect.yTop-(length/6*z)-1;
            pt.x = rect.xLeft;
            GpiMove( hps, &pt );
            pt.x = rect.xRight;
            GpiLine( hps, &pt );
            }

        GpiSetColor( hps, calButtonShadows?CalButtonDark:CalButtonMiddle );
        for( z=0; z<8; z++ ) {
            pt.x = rect.xLeft+(width/7*z)+1;
            pt.y = rect.yTop;
            GpiMove( hps, &pt );
            if (z<3)
                pt.y = rect.yBottom;
            else
                pt.y = rect.yBottom + (length/6);
            GpiLine( hps, &pt );
            }
        for( z=0; z<7; z++ ) {
            pt.y = rect.yTop-(length/6*z)+1;
            pt.x = rect.xLeft;
            GpiMove( hps, &pt );
            pt.x = rect.xRight;
            GpiLine( hps, &pt );
            }

        GpiSetColor( hps, calButtonShadows?CalButtonMiddle:CalButtonDark );
        for( z=0; z<8; z++ ) {
            pt.x = rect.xLeft+(width/7*z);
            pt.y = rect.yTop;
            GpiMove( hps, &pt );
            if (z<3)
                pt.y = rect.yBottom;
            else
                pt.y = rect.yBottom + (length/6);
            GpiLine( hps, &pt );
            }
        for( z=0; z<7; z++ ) {
            pt.y = rect.yTop-(length/6*z);
            pt.x = rect.xLeft;
            GpiMove( hps, &pt );
            pt.x = rect.xRight;
            GpiLine( hps, &pt );
            }
        }

    GpiSetColor( hps, CalButtonMiddle );
    pt.x = reminderRect.xLeft;
    pt.y = reminderRect.yBottom;
    GpiMove( hps, &pt );
    pt.x = reminderRect.xRight;
    pt.y = 2;
    GpiBox( hps, DRO_FILL, &pt, 0, 0);

    if (highlightToday) {
        if ((todaysDate.da_mon == curMonth) && (todaysDate.da_year == curYear)) {
            int tmp = (todaysDate.da_day-1) + curDate.tm_wday;
            int y = 5-(tmp / 7);
            int x = (tmp % 7);

            pt.x = x*(width/7);
            pt.y = y*(length/6);
            GpiMove( hps, &pt );
            pt2.x = pt.x + (width/7);
            pt2.y = pt.y + (length/6);
            GpiSetColor(hps, CalDayColor);
            GpiBox(hps,DRO_OUTLINE,&pt2,0L,0L);

            pt.x--; pt.y--;
            GpiMove( hps, &pt );
            pt2.x++; pt2.y++;
            GpiBox( hps, DRO_OUTLINE, &pt2, 0L, 0L );
            }
        }
}


////////////////////////////////////////////////////////////////////////////////
// makeBackupFile - copy current calendar file to BACKUP.CAL
////////////////////////////////////////////////////////////////////////////////
CHAR makeBackupFile()
{
    FILE *outFile = NULL;
    FILE *inFile = NULL;

    // backup the current calendar file
    if( calendarFile )
        inFile = fopen( calendarFile, "rt" );
    else
        inFile = fopen( FILENAME, "rt" );

    if( inFile )
        if( outFile = fopen( BACKUPFILE, "wt" )) {
            while( !feof(inFile) )
                fputc( fgetc(inFile), outFile );
            fclose( outFile );
            fclose( inFile );
            return TRUE;
            }
    return FALSE;
}


////////////////////////////////////////////////////////////////////////////////
// writeOptionFile - save a calendar configuration file
////////////////////////////////////////////////////////////////////////////////
CHAR writeOptionFile()
{
    #define BUFSIZE 128
    CHAR tmpBuffer[BUFSIZE];
    ReminderItem *tmp;
    ULONG ppid ;
    FILE *outFile = NULL;
    SWP swp1,swp2,minCal,minToDo;

    WinQueryWindowPos( calendar.frame, &minCal );
    WinQueryWindowPos( toDoList.frame, &minToDo );

    ShowControls( TRUE, toDoList );
    WinQueryWindowPos( toDoList.frame, &swp1 );
    ShowControls( todoShowControls, toDoList );

    ShowControls( TRUE, calendar );
    WinQueryWindowPos( calendar.frame, &swp2 );
    ShowControls( calShowControls, calendar );

    outFile = NULL;
    if (calendarFile) {
        outFile = fopen( calendarFile, "wt" );
        if( outFile==NULL ) {
            WinMessageBox( HWND_DESKTOP, calendar.frame, MSG006, MSG004, 0,
                           MB_ERROR|MB_OK );
            return FALSE;
            }
        }

    if( outFile==NULL )
        outFile = fopen( FILENAME, "wt" );
    if( outFile==NULL ) {
        WinMessageBox( HWND_DESKTOP, calendar.frame, MSG006, MSG004, 0,
                       MB_ERROR|MB_OK );
        return FALSE;
        }

    // output header
    fprintf( outFile, "======================================================================\n");
    fprintf( outFile, "%s - %s\n", MSG007, MSG009 );
    fprintf( outFile, "%s - %s\n", VERSION, MSG010 );
    fprintf( outFile, "======================================================================\n");
    fflush( outFile );

    // output window positions and minimized state
    fprintf( outFile, "%s=%ld,%ld,%ld,%ld,%ld\n", TODOLISTPOS, swp1.x, swp1.y, swp1.cx, swp1.cy, (minToDo.fl&SWP_MINIMIZE)?TRUE:FALSE );
    fprintf( outFile, "%s=%ld,%ld,%ld,%ld,%ld\n", CALPOS, swp2.x, swp2.y, swp2.cx, swp2.cy, (minCal.fl&SWP_MINIMIZE)?TRUE:FALSE );
    fflush( outFile );

    // output colors
    fprintf( outFile, "%s=%ld\n", OLDMONTHBACK, oldMonthBack );
    fprintf( outFile, "%s=%ld\n", OLDMONTHTEXT, oldMonthText );
    toDoBackColor = LONGFROMMR( WinSendMsg( toDoList.childPane, MLM_QUERYBACKCOLOR, 0, 0 ) );
    toDoForeColor = LONGFROMMR( WinSendMsg( toDoList.childPane, MLM_QUERYTEXTCOLOR, 0, 0 ) );
    fprintf( outFile, "%s=%ld\n", TODOBCOLOR, toDoBackColor );
    fprintf( outFile, "%s=%ld\n", TODOFCOLOR, toDoForeColor );
    fprintf( outFile, "%s=%ld\n", CALHIGHTEXT, CalHighText );
    fprintf( outFile, "%s=%ld\n", CALDAYCOLOR, CalDayColor );
    fprintf( outFile, "%s=%ld\n", CALHIGHLIGHT, CalHighlight );
    fprintf( outFile, "%s=%ld\n", CALDONEHIGHLIGHT, CalDoneHighlight );
    fprintf( outFile, "%s=%ld\n", CALDONETEXT, CalDoneText );
    fprintf( outFile, "%s=%ld\n", CALTEXT, CalText );
    fprintf( outFile, "%s=%ld\n", CALREMINDDONE, CalRemindDone );
    fprintf( outFile, "%s=%ld\n", CALBUTTONLIGHT, CalButtonLight );
    fprintf( outFile, "%s=%ld\n", CALBUTTONMID, CalButtonMiddle );
    fprintf( outFile, "%s=%ld\n", CALBUTTONDARK, CalButtonDark );
    fprintf( outFile, "%s=%ld\n", PRINTWIDTH, printWidth );
    fflush( outFile );

    // output other variables (boolean and integer)
    fprintf( outFile, "%s=%d\n", SHOWTHECLOCK, showClock );
    fprintf( outFile, "%s=%d\n", SHOWALLDAYS, showAllDays );
    fprintf( outFile, "%s=%d\n", TODODISPLAY, toDoDisplay );
    fprintf( outFile, "%s=%d\n", DAYALIGNMENT, calDaysAlignment );
    fprintf( outFile, "%s=%d\n", CALBORDERLINES, drawCalBorderLines );
    fprintf( outFile, "%s=%d\n", CALDAYBORDER, calDrawDayBorder );
    fprintf( outFile, "%s=%d\n", CALBUTTONSHADOWS, calButtonShadows );
    fprintf( outFile, "%s=%d\n", SHOWTODOLIST, showToDoList );
    fprintf( outFile, "%s=%d\n", USEQUICKREMINDER, calQuickRemind );
    fprintf( outFile, "%s=%d\n", CALSHOWCONTROLS, calShowControls );
    fprintf( outFile, "%s=%d\n", TODOSHOWCONTROLS, todoShowControls );
    fprintf( outFile, "%s=%d\n", LABELDAYSOFWEEK, labelDaysOfWeek );
    fprintf( outFile, "%s=%d\n", ABBREVWEEKDAYS, abbreviateWeekDays );
    fprintf( outFile, "%s=%d\n", ABBREVMONTH, abbreviateMonth );
    fprintf( outFile, "%s=%d\n", HIGHLIGHTTDAY, highlightToday );
    fprintf( outFile, "%s=%d\n", REMINDIFDONE, remindEvenIfDone );
    fprintf( outFile, "%s=%d\n", ASKBEFOREDEL, askBeforeDeleteItem );
    fprintf( outFile, "%s=%d\n", SOUNDREPLAY, soundReplay );
    fprintf( outFile, "%s=%d\n", SNOOZETIME, snoozeTime );
    fprintf( outFile, "%s=%d\n", DAYSINADVANCE, daysInAdvance );
    fprintf( outFile, "%s=%d\n", PRINTTODOL, printToDo );
    fprintf( outFile, "%s=%d\n", PRINTCALEN, printCal );
    fflush( outFile );

    // output fonts
    memset( tmpBuffer, 0, BUFSIZE );
    if( WinQueryPresParam( toDoList.childPane, PP_FONTNAMESIZE, 0, &ppid, BUFSIZE, &tmpBuffer, 0 ) )
        fprintf( outFile, "%s=%s\n", TODOFONT, tmpBuffer );
    else
        fprintf( outFile, "%s=%s\n", TODOFONT, MSG_NOFONT );

    memset( tmpBuffer, 0, BUFSIZE );
    if( WinQueryPresParam( calendar.client, PP_FONTNAMESIZE, 0, &ppid, BUFSIZE, &tmpBuffer, 0 ) )
        fprintf( outFile, "%s=%s\n", CALFONT, tmpBuffer );
    else
        fprintf( outFile, "%s=%s\n", CALFONT, MSG_NOFONT );
    fflush( outFile );

    // output all of the events
    fprintf( outFile, "\n%s=%d\n", NUMREMINDERS, numReminders );
    for( int z=0; z<numReminders; z++ ) {
        tmp = reminders[z];
        fprintf( outFile, "%s=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%s\n",
                                           REMINDER,
                                           tmp->getToDo(), tmp->getItemDone(),
                                           tmp->getNumToShow(), tmp->getMonth(),
                                           tmp->getDay(), tmp->getYear(),
                                           tmp->getHour(), tmp->getMinute(),
                                           tmp->getReminder(), tmp->getReminderType(),
                                           tmp->getMinBefore(), tmp->getMinsBefore(),
                                           tmp->getDaysBefore(), tmp->getReschedTime(),
                                           tmp->getSoundFile(), tmp->getText() );
        }

    fclose( outFile );

    if (minCal.fl & SWP_MINIMIZE)
        WinSetWindowPos( calendar.frame, 0, 0, 0, 0, 0, ((minCal.fl&SWP_MINIMIZE)?SWP_MINIMIZE:0) );
    if (minToDo.fl & SWP_MINIMIZE)
        WinSetWindowPos( toDoList.frame, 0, 0, 0, 0, 0, ((minToDo.fl&SWP_MINIMIZE)?SWP_MINIMIZE:0) );
    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
// readOptionFile - read in a calendar file
////////////////////////////////////////////////////////////////////////////////
VOID readOptionFile()
{
    #define SIZETMP 512
    INT reminderNum = 0;
    INT tmp, lineNum;
    CHAR *token;
    float vers;
    CHAR tmpStr[SIZETMP];
    FILE *inFile = NULL;

    //--------------------------------------------------------------------------
    // open the user specified calendar file if given
    //--------------------------------------------------------------------------
    minimizeToDo = FALSE;
    minimizeCal = FALSE;
    if( calendarFile ) {
        inFile = fopen( calendarFile, "rt" );
        if( inFile==NULL )
            WinMessageBox( HWND_DESKTOP, calendar.frame, MSG002, MSG004, 0,
                           MB_ERROR|MB_OK );
        }

    //--------------------------------------------------------------------------
    // if not available, open the local default file
    //--------------------------------------------------------------------------
    if( inFile==NULL )
        inFile = fopen( FILENAME, "rt" );
    if( inFile==NULL ) {
        WinMessageBox( HWND_DESKTOP, calendar.frame, MSG003, MSG004, 0,
                       MB_ERROR|MB_OK );
        return;
        }

    //--------------------------------------------------------------------------
    // grab version number from header
    //--------------------------------------------------------------------------
    fgets( tmpStr, SIZETMP, inFile );
    fgets( tmpStr, SIZETMP, inFile );
    fgets( tmpStr, SIZETMP, inFile );
    vers = atof( &tmpStr[8] );

    if ((vers<(float)LOWESTVER) || (vers>(float)CURRENTVER)) {
        sprintf(tmpStr, MSG005, vers);
        WinMessageBox( HWND_DESKTOP, calendar.frame, tmpStr, MSG004, 0,
                       MB_ERROR|MB_OK );
        fclose( inFile );
        initCalendar();
        return;
        }
    fgets( tmpStr, SIZETMP, inFile );

    //--------------------------------------------------------------------------
    // read the configuration file, line by line
    //--------------------------------------------------------------------------
    lineNum = 4;
    while (!feof( inFile )) {
        lineNum++;
        tmpStr[0] = 0;
        fgets( tmpStr, SIZETMP, inFile );
        if ((!tmpStr[0]) || (tmpStr[0]=='\n'))    // skip if we got a blank line
            continue;
        strtok( tmpStr, "=\n\0" );
        token = strtok( NULL, "=\n\0" );
        if (token)
            tmp = atoi( token );
        // -- read in # of reminders saved in this file
        if (strcmp( tmpStr, NUMREMINDERS) == 0) {
            if ((tmp<0) || (tmp>MAXREMINDERS))
                reportErrorLine( lineNum );
            else
                numReminders = (REMINDERINDEX)tmp;
            }
        // -- read in day alignment
        else if (strcmp( tmpStr, DAYALIGNMENT) == 0) {
            if ((tmp<LEFT) || (tmp>RIGHT))
                reportErrorLine( lineNum );
            else
                calDaysAlignment = tmp;
            }
        // -- check for drawing calendar border lines
        else if (strcmp( tmpStr, CALBORDERLINES) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                drawCalBorderLines = tmp;
            }

        // --
        else if( strcmp( tmpStr, CALFONT ) == 0) {
            if( strcmp( token, MSG_NOFONT ) != 0)
                WinSetPresParam( calendar.client, PP_FONTNAMESIZE, strlen(token)+1, token );
            }
        // --
        else if( strcmp( tmpStr, TODOFONT ) == 0) {
            if( strcmp( token, MSG_NOFONT ) != 0 )
                WinSetPresParam( toDoList.childPane, PP_FONTNAMESIZE, strlen(token)+1, token );
            }
        // --
        else if (strcmp( tmpStr, TODOFCOLOR ) == 0) {
            toDoForeColor = tmp;
            WinSendMsg( toDoList.childPane, MLM_SETTEXTCOLOR, MPFROMLONG( toDoForeColor ), 0 );
            }
        // --
        else if (strcmp( tmpStr, TODOBCOLOR ) == 0) {
            toDoBackColor = tmp;
            WinSendMsg( toDoList.childPane, MLM_SETBACKCOLOR, MPFROMLONG( toDoBackColor ), 0 );
            }
        // --
        else if (strcmp( tmpStr, CALDAYBORDER) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                calDrawDayBorder = tmp;
            }
        // --
        else if (strcmp( tmpStr, CALBUTTONSHADOWS) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                calButtonShadows = tmp;
            }
        // --
        else if (strcmp( tmpStr, SHOWTODOLIST) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                showToDoList = tmp;
            }
        // --
        else if (strcmp( tmpStr, USEQUICKREMINDER) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                calQuickRemind = tmp;
            }
        // --
        else if (strcmp( tmpStr, SHOWTHECLOCK) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                showClock = tmp;
            }
        // --
        else if (strcmp( tmpStr, PRINTTODOL) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                printToDo = tmp;
            }
        // --
        else if (strcmp( tmpStr, PRINTCALEN) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                printCal = tmp;
            }
        // --
        else if (strcmp( tmpStr, CALSHOWCONTROLS) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                calShowControls = tmp;
            }
        // --
        else if (strcmp( tmpStr, TODOSHOWCONTROLS) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                todoShowControls = tmp;
            }
        // --
        else if (strcmp( tmpStr, LABELDAYSOFWEEK) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                labelDaysOfWeek = tmp;
            }
        // --
        else if (strcmp( tmpStr, ABBREVWEEKDAYS) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                abbreviateWeekDays = tmp;
            }
        // --
        else if (strcmp( tmpStr, ABBREVMONTH) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                abbreviateMonth = tmp;
            }
        // --
        else if (strcmp( tmpStr, HIGHLIGHTTDAY) == 0) {
            if ((tmp!=FALSE) && (tmp!=TRUE))
                reportErrorLine( lineNum );
            else
                highlightToday = tmp;
            }
        // --
        else if (strcmp( tmpStr, TODOLISTPOS ) == 0) {
            LONG x, y, cx, cy;
            x  = atoi( strtok( token, "," ) );
            y  = atoi( strtok( NULL, "," ) );
            cx = atoi( strtok( NULL, "," ) );
            cy = atoi( strtok( NULL, ",") );
            minimizeToDo = atoi( strtok( NULL, "\0\n") );
            WinSetWindowPos( toDoList.frame, 0, x, y, cx, cy,
                             SWP_MOVE|SWP_SIZE );
            }
        // --
        else if (strcmp( tmpStr, CALPOS ) == 0) {
            LONG x,y,cx,cy;
            x  = atoi( strtok( token, "," ) );
            y  = atoi( strtok( NULL, "," ) );
            cx = atoi( strtok( NULL, "," ) );
            cy = atoi( strtok( NULL, ",") );
            minimizeCal = atoi( strtok( NULL, "\0\n") );
            WinSetWindowPos( calendar.frame, 0, x, y, cx, cy,
                             SWP_MOVE|SWP_SIZE );
            }
        // --
        else if (strcmp( tmpStr, SHOWALLDAYS ) == 0)
            showAllDays = tmp;
        // --
        else if (strcmp( tmpStr, TODODISPLAY ) == 0)
            toDoDisplay = tmp;
        // --
        else if (strcmp( tmpStr, CALHIGHTEXT ) == 0)
            CalHighText = tmp;
        // --
        else if (strcmp( tmpStr, CALDAYCOLOR ) == 0)
            CalDayColor = tmp;
        // --
        else if (strcmp( tmpStr, CALHIGHLIGHT ) == 0)
            CalHighlight = tmp;
        // --
        else if (strcmp( tmpStr, CALDONEHIGHLIGHT ) == 0)
            CalDoneHighlight = tmp;
        // --
        else if (strcmp( tmpStr, CALDONETEXT ) == 0)
            CalDoneText = tmp;
        // --
        else if (strcmp( tmpStr, PRINTWIDTH ) == 0)
            printWidth = tmp;
        // --
        else if (strcmp( tmpStr, CALTEXT ) == 0)
            CalText = tmp;
        // --
        else if (strcmp( tmpStr, CALREMINDDONE ) == 0)
            CalRemindDone = tmp;
        // --
        else if (strcmp( tmpStr, CALBUTTONMID ) == 0)
            CalButtonMiddle = tmp;
        // --
        else if (strcmp( tmpStr, CALBUTTONLIGHT ) == 0)
            CalButtonLight = tmp;
        // --
        else if (strcmp( tmpStr, OLDMONTHBACK ) == 0)
            oldMonthBack = tmp;
        // --
        else if (strcmp( tmpStr, OLDMONTHTEXT ) == 0)
            oldMonthText = tmp;
        // --
        else if (strcmp( tmpStr, CALBUTTONDARK ) == 0)
            CalButtonDark = tmp;
        // --
        else if (strcmp( tmpStr, REMINDIFDONE ) == 0)
            remindEvenIfDone = tmp;
        // --
        else if (strcmp( tmpStr, ASKBEFOREDEL ) == 0)
            askBeforeDeleteItem = tmp;
        // --
        else if (strcmp( tmpStr, SOUNDREPLAY ) == 0)
            soundReplay = tmp;
        // --
        else if (strcmp( tmpStr, SNOOZETIME ) == 0)
            snoozeTime = tmp;
        // --
        else if (strcmp( tmpStr, DAYSINADVANCE ) == 0)
            daysInAdvance = tmp;
        // --
        else if (strcmp( tmpStr, REMINDER) == 0) {
            INT td,dn,nd,mn,dy,yr,hr,mi,gr,gt,mb,rt,sb,db;
            char *textPtr, *sndPtr;
            td = atoi( strtok( token, "," ) );
            dn = atoi( strtok( NULL, "," ) );
            nd = atoi( strtok( NULL, "," ) );
            mn = atoi( strtok( NULL, "," ) );
            dy = atoi( strtok( NULL, "," ) );
            yr = atoi( strtok( NULL, "," ) );
            hr = atoi( strtok( NULL, "," ) );
            mi = atoi( strtok( NULL, "," ) );
            gr = atoi( strtok( NULL, "," ) );
            gt = atoi( strtok( NULL, "," ) );
            mb = atoi( strtok( NULL, "," ) );
            sb = atoi( strtok( NULL, "," ) );
            db = atoi( strtok( NULL, "," ) );
            if( vers>1.01 )
                rt = atoi( strtok( NULL, "," ) );
            else
                rt = 0;
            sndPtr  = strtok( NULL, "," );
            textPtr = strtok( NULL, "\0\n");
            reminders[reminderNum++] = new ReminderItem( td, dn, nd, mn, dy,
                                                         yr, hr, mi, gr, gt,
                                                         mb, sb, db, sndPtr,
                                                         textPtr, rt );
            }
        // -- if we made it here, we can't parse this line!
        else
            reportErrorLine( lineNum );
        }
    fclose( inFile );

    if (reminderNum != numReminders) {
        char tmp[128];
        sprintf(tmp, MSG012, NUMREMINDERS );
        WinMessageBox( HWND_DESKTOP, calendar.frame, tmp, MSG004, 0,
                       MB_ERROR|MB_OK );
        numReminders = (numReminders<reminderNum)?numReminders:reminderNum;
        }
    sortTimeEntries();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
VOID updateDay( HWND parent )
{
    ReminderItem* ptr = reminders[eventToEdit];
    struct tm curDate;
    char* tmp = new char[ 80 ];
    LONG tmpVal;
    BOOL st1, st2;
    CHAR *tmpPtr;

    tmp[0] = 0;
    st1 = (ptr->getYear()==ALLYEARS);
    st2 = (ptr->getMonth()==ALLMONTHS);

    curDate.tm_year = 95;
    curDate.tm_mon = 1;

    WinSendMsg( WinWindowFromID( parent, DAY ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
    curDate.tm_mday = tmpVal;
    WinSendMsg( WinWindowFromID( parent, YEAR ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
    if (!st1)
        curDate.tm_year = (st1?1:tmpVal-1900);
    WinSendMsg( WinWindowFromID( parent, MONTH ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
    if (!st2)
        curDate.tm_mon = (st2?1:tmpVal);

    curDate.tm_hour = 0;
    curDate.tm_min = 0;
    curDate.tm_sec = 0;
    curDate.tm_isdst = -1;

    if (mktime(&curDate) != -1) {
        if ((st1) && (st2))
            tmpPtr = CURDATESTR3;
        else if ((st1) && (!st2))
            tmpPtr = CURDATESTR1;
        else if ((!st1) && (st2))
            tmpPtr = CURDATESTR2;
        else if ((!st1) && (!st2))
            tmpPtr = CURDATESTR4;
        strftime( tmp, 80, tmpPtr, (const tm*)&curDate );
        }

    WinSetWindowText( WinWindowFromID( parent, DATETEXT ), tmp );
    delete tmp;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY spinProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    //--------------------------------------------------------------------------
    // if the sample calendar requests repainting, call our routine to do it
    //--------------------------------------------------------------------------
    if (msg == WM_CONTROL)
        updateDay( WinQueryWindow( hWnd, QW_PARENT ) );
    return oldSpinProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY modifyEventProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch( msg )
    {
        //----------------------------------------------------------------------
        // when the dialog is being initialized, center it on desktop and check
        // the appropriate radio button for whatever setting they currently have
        //----------------------------------------------------------------------
        case WM_INITDLG: {
            char* tmp = new char[1024];
            ReminderItem* ptr = reminders[eventToEdit];

            centerDialog( hWnd );
            pushStack(snoozeOn);
            snoozeOn = TRUE;

            //------------------------------------------------------------------
            // correctly display title and date and set up various spin controls
            //------------------------------------------------------------------
            WinSendMsg( WinWindowFromID( hWnd, EVENTTITLE ), EM_SETTEXTLIMIT, MPFROMSHORT(MAXITEMSIZE-1), 0 );
            WinSetWindowText( WinWindowFromID( hWnd, EVENTTITLE ), ptr->getText() );
            WinSendMsg( WinWindowFromID( hWnd, EVENTTITLE ), EM_SETSEL, MPFROM2SHORT(0,strlen(ptr->getText())), 0 );
            WinCheckButton( hWnd, ITEMDONE, ptr->getItemDone() );
            WinCheckButton( hWnd, SHOWASTODO, ptr->getToDo() );
            WinSendMsg( WinWindowFromID( hWnd, DAYSADVANCE ), SPBM_SETLIMITS, MPFROMLONG(180), 0 );
            WinSendMsg( WinWindowFromID( hWnd, DAYSADVANCE ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getNumToShow()), MPFROMLONG(0) );
            updateYear( hWnd, ptr );       // correctly set up year spin button
            updateMonth( hWnd, ptr );      // correctly set up month spin button
            WinSendMsg( WinWindowFromID( hWnd, DAY ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getDay()), MPFROMLONG(0) );

            //------------------------------------------------------------------
            // correctly display time
            //------------------------------------------------------------------
            updateTime( hWnd, ptr );
            WinSendMsg( WinWindowFromID( hWnd, MIN ), SPBM_SETLIMITS, MPFROMLONG(59), MPFROMLONG(0) );
            WinSendMsg( WinWindowFromID( hWnd, MIN ), SPBM_SETTEXTLIMIT, MPFROMSHORT(2), MPFROMLONG(0) );
            WinSendMsg( WinWindowFromID( hWnd, MIN ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getMinute()), MPFROMLONG(0) );
            WinSendMsg( WinWindowFromID( hWnd, RESCHEDTIME ), SPBM_SETLIMITS, MPFROMLONG(180), MPFROMLONG(0) );
            WinSendMsg( WinWindowFromID( hWnd, RESCHEDTIME ), SPBM_SETCURRENTVALUE, MPFROMLONG(ptr->getReschedTime()), MPFROMLONG(0) );

            updateReminder( hWnd, ptr );
            updateDay( hWnd );

            oldSpinProc = WinSubclassWindow( WinWindowFromID( hWnd, MONTH ), spinProc );
            oldSpinProc = WinSubclassWindow( WinWindowFromID( hWnd, DAY ), spinProc );
            oldSpinProc = WinSubclassWindow( WinWindowFromID( hWnd, YEAR ), spinProc );
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            //------------------------------------------------------------------
            // it was not a user item, process now as usual
            //------------------------------------------------------------------
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // user wants to launch sound recorder
                //--------------------------------------------------------------
                case LAUNCHRECORDER: {
                    launchRecorder();
                    return 0;
                    }
                //--------------------------------------------------------------
                // user wants to select a new sound file
                //--------------------------------------------------------------
                case SELECTSOUNDFILE: {
                    ReminderItem *ptr = reminders[eventToEdit];
                    HWND hwndDialog;
                    CHAR *oldDirectory = 0;

                    fileDlgInfo.fl = FDS_OPEN_DIALOG|FDS_CENTER;
                    fileDlgInfo.pszTitle = MSG020;
                    setItem( &oldDirectory, fileDlgInfo.szFullFile );
                    strcat( fileDlgInfo.szFullFile, "*.WAV\0" );

                    hwndDialog = WinFileDlg( HWND_DESKTOP, hWnd, &fileDlgInfo );
                    if (hwndDialog && (fileDlgInfo.lReturn == DID_OK)) {
                        ptr->setSoundFile( fileDlgInfo.szFullFile );
                        WinSetDlgItemText( hWnd, SOUNDFILE, ptr->getSoundFile() );
                        resetFileDialog();
                        }
                    else {
                        if (oldDirectory)
                            strcpy( fileDlgInfo.szFullFile, oldDirectory );
                        else
                            fileDlgInfo.szFullFile[0] = 0;
                        }
                    if (oldDirectory)
                        delete oldDirectory;
                    return 0;
                    }
                //--------------------------------------------------------------
                // they cancelled the dialog, get rid of it
                //--------------------------------------------------------------
                case DID_OK: {
                    LONG tmpVal;
                    CHAR tmp[MAXITEMSIZE+1];
                    ReminderItem* ptr = reminders[eventToEdit];

                    // record event name
                    WinQueryWindowText( WinWindowFromID( hWnd, EVENTTITLE ), MAXITEMSIZE, tmp );
                    ptr->setText( tmp );
                    ptr->setToDo( WinQueryButtonCheckstate( hWnd, SHOWASTODO ) );
                    ptr->setItemDone( WinQueryButtonCheckstate( hWnd, ITEMDONE ) );

                    WinSendMsg( WinWindowFromID( hWnd, HOUR ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    ptr->setHour( (char)tmpVal%24 );
                    WinSendMsg( WinWindowFromID( hWnd, MIN ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    ptr->setMinute( (char)tmpVal%60 );

                    WinSendMsg( WinWindowFromID( hWnd, DAYSADVANCE ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    ptr->setNumToShow( (char)tmpVal%365 );
                    WinSendMsg( WinWindowFromID( hWnd, MINBEFORE ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    ptr->setMinsBefore( (char)tmpVal%256 );
                    WinSendMsg( WinWindowFromID( hWnd, DAYSBEFORE ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    ptr->setDaysBefore( (char)tmpVal%256 );
                    WinSendMsg( WinWindowFromID( hWnd, RESCHEDTIME ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    ptr->setReschedTime( (char)tmpVal%256 );

                    WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    if (WinQueryButtonCheckstate( hWnd, EVERYYEAR ))
                        ptr->setYear( ALLYEARS );
                    else
                        ptr->setYear( tmpVal );

                    WinSendMsg( WinWindowFromID( hWnd, MONTH ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    if (WinQueryButtonCheckstate( hWnd, EVERYMONTH ))
                        ptr->setMonth( ALLMONTHS );
                    else
                        ptr->setMonth( tmpVal+1 );

                    WinSendMsg( WinWindowFromID( hWnd, DAY ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    ptr->setDay( tmpVal );

                    eventToEdit = TRUE;
                    WinDismissDlg( hWnd, TRUE );
                    snoozeOn = popStack();
                    return 0;
                    }
                //--------------------------------------------------------------
                // if cancelled, set appropriate flag and get rid of dialog
                //--------------------------------------------------------------
                case WM_CLOSE:
                case DID_CANCEL:
                    eventToEdit = FALSE;
                    WinDismissDlg( hWnd, TRUE );
                    snoozeOn = popStack();
                    return 0;
                //--------------------------------------------------------------
                // pass on default processing to default dialog processor
                //--------------------------------------------------------------
                default:
                    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
                }
            }
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        case WM_CONTROL: {
            ReminderItem *ptr = reminders[eventToEdit];
            if ((SHORT2FROMMP(mp1)==BN_CLICKED) ||
                (SHORT2FROMMP(mp1)==BN_DBLCLICKED)) {
                switch( SHORT1FROMMP(mp1) ) {
                    case BUTTON1:
                        ptr->setMinBefore( TRUE );
                        break;
                    case BUTTON2:
                        ptr->setMinBefore( FALSE );
                        break;
                    case VISUALLY:
                    case AUDIBLY:
                    case BOTH:
                        ptr->setReminderType( SHORT1FROMMP(mp1) );
                        break;
                    case EVERYYEAR:
                        if (ptr->getYear()==ALLYEARS)
                            ptr->setYear(curYear);
                        else
                            ptr->setYear(ALLYEARS);
                        updateYear( hWnd, ptr );
                        break;
                    case EVERYMONTH:
                        if (ptr->getMonth()==ALLMONTHS)
                            ptr->setMonth(curMonth);
                        else
                            ptr->setMonth(ALLMONTHS);
                        updateMonth( hWnd, ptr );
                        break;
                    case FLASHREMINDER:
                        ptr->setReminder( !ptr->getReminder() );
                        updateReminder( hWnd, ptr );
                        break;
                    default:
                        break;
                    }
                updateDay( hWnd );
                }
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        default:
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
    }
}


////////////////////////////////////////////////////////////////////////////////
// Message handler for
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY reminderProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch( msg ) {
        //----------------------------------------------------------------------
        // when the dialog is being initialized, center it on desktop
        //----------------------------------------------------------------------
        case WM_INITDLG: {
            ReminderItem *tmp = reminders[eventReminder];
            char tmpBuf[32];

            pushStack(snoozeOn);
            snoozeOn = TRUE;

            stopMinuteTimer();
            centerDialog( hWnd );
            if (soundReplay)
                soundTimer = WinStartTimer( hab, hWnd, TIMERID2, soundReplay*TICKSPERMIN);

            WinSetWindowText( WinWindowFromID( hWnd, REMINDERTEXT ),
                              tmp->getText() );

            if (tmp->getMinBefore())
                sprintf( tmpBuf, MSG036, snoozeTime );
            else
                sprintf( tmpBuf, MSG037 );

            if (tmp->getReminderType()==BOTH)
                WinPostMsg( hWnd, WM_TIMER, 0, 0 );

            WinSetWindowText( WinWindowFromID( hWnd, SNOOZE ), tmpBuf );
            WinSetWindowText( WinWindowFromID( hWnd, REMINDERTEXT ),
                              tmp->getText() );
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        // replay sound wav if inactive for specified length of time
        //----------------------------------------------------------------------
        case WM_TIMER: {
            ReminderItem *tmp = reminders[eventReminder];
            if( tmp->getReminderType()==BOTH )
                playSound( tmp->getSoundFile() );
            return 0;
            }
        //----------------------------------------------------------------------
        // if we receive any system message, dismiss the dialog box
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            //------------------------------------------------------------------
            // it was not a user item, process now as usual
            //------------------------------------------------------------------
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // snooze this event
                //--------------------------------------------------------------
                case WM_CLOSE:
                case SNOOZE: {
                    setSnooze( hWnd, eventReminder );
                    redrawCalendar();
                    restartTimer();
                    return 0;
                    }
                //--------------------------------------------------------------
                // erase this event
                //--------------------------------------------------------------
                case ERASEEVENT: {
                    if (askBeforeDeleteItem)
                        if (WinMessageBox( HWND_DESKTOP, hWnd,
                                           MSG014, MSG013, 0,
                                           MB_MOVEABLE|MB_ICONQUESTION|MB_YESNO) == MBID_NO)
                            return 0;

                    for( INT z=eventReminder; z<numReminders-1; z++)
                        reminders[z] = reminders[z+1];
                    numReminders--;
                    WinStopTimer( hab, hWnd, soundTimer );
                    WinDismissDlg( hWnd, TRUE );
                    redrawCalendar();
                    restartTimer();
                    snoozeOn = popStack();
                    return 0;
                    }
                //--------------------------------------------------------------
                // the user wants to revise this event
                //--------------------------------------------------------------
                case REVISEEVENT:
                    WinStopTimer( hab, hWnd, soundTimer );
                    if (modifyEvent( hWnd, eventReminder )) {
                        restartTimer();
                        WinDismissDlg( hWnd, TRUE );
                        snoozeOn = popStack();
                        }
                    else
                        soundTimer = WinStartTimer( hab, hWnd, TIMERID2, soundReplay*TICKSPERMIN);
                    redrawCalendar();
                    return 0;
                //--------------------------------------------------------------
                // user wants to mark this item as done
                //--------------------------------------------------------------
                case MARKDONE: {
                    CHAR reschedTime = reminders[eventReminder]->getReschedTime();
                    if (reschedTime)
                        moveToDate( eventReminder,
                                    reminders[eventReminder]->getMonth(),
                                    reminders[eventReminder]->getDay()+reschedTime,
                                    reminders[eventReminder]->getYear(),
                                    reminders[eventReminder]->getHour(),
                                    reminders[eventReminder]->getMinute() );
                    else
                        reminders[eventReminder]->setItemDone( TRUE );
                    WinStopTimer( hab, hWnd, soundTimer );
                    WinDismissDlg( hWnd, TRUE );
                    redrawCalendar();
                    restartTimer();
                    snoozeOn = popStack();
                    return 0;
                    }
                //--------------------------------------------------------------
                //--------------------------------------------------------------
                default:
                    break;
                }
            }
        //----------------------------------------------------------------------
        // if nothing further we want to intercept, pass message onto system
        //----------------------------------------------------------------------
        default:
            break;
        }
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// Message handler for calendar draw options dialog box
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY reminderOptionsProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch( msg ) {
        //----------------------------------------------------------------------
        // when the dialog is being initialized, center it on desktop and check
        // the appropriate radio button for whatever setting they currently have
        //----------------------------------------------------------------------
        case WM_INITDLG:
            centerDialog( hWnd );
            WinSendMsg( WinWindowFromID( hWnd, NUMSNOOZE ), SPBM_SETLIMITS, MPFROMLONG(180), MPFROMLONG(1) );
            WinSendMsg( WinWindowFromID( hWnd, NUMSNOOZE ), SPBM_SETCURRENTVALUE, MPFROMLONG(snoozeTime), 0 );
            WinSendMsg( WinWindowFromID( hWnd, NUMREPLAY ), SPBM_SETLIMITS, MPFROMLONG(180), 0 );
            WinSendMsg( WinWindowFromID( hWnd, NUMREPLAY ), SPBM_SETCURRENTVALUE, MPFROMLONG(soundReplay), 0 );
            WinSendMsg( WinWindowFromID( hWnd, NUMADVANCE ), SPBM_SETLIMITS, MPFROMLONG(180), 0 );
            WinSendMsg( WinWindowFromID( hWnd, NUMADVANCE ), SPBM_SETCURRENTVALUE, MPFROMLONG(daysInAdvance), 0 );
            WinCheckButton( hWnd, ASKBEFOREDELETE, askBeforeDeleteItem );
            break;
        //----------------------------------------------------------------------
        // Handle any of the controls within the dialog
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // dismiss dialog and redraw real calendar if user is done
                //--------------------------------------------------------------
                case DID_OK:
                    LONG tmpVal;

                    askBeforeDeleteItem = (WinQueryButtonCheckstate( hWnd, ASKBEFOREDELETE )?TRUE:FALSE);
                    WinSendMsg( WinWindowFromID( hWnd, NUMSNOOZE ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    snoozeTime = tmpVal;
                    WinSendMsg( WinWindowFromID( hWnd, NUMREPLAY ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    soundReplay = tmpVal;
                    WinSendMsg( WinWindowFromID( hWnd, NUMADVANCE ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    daysInAdvance = tmpVal;

                    WinDismissDlg( hWnd, TRUE );
                    return 0;
                //--------------------------------------------------------------
                // let system handle anything we don't want to handle
                //--------------------------------------------------------------
                default:
                    break;
                }
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        // if nothing further we want to intercept, pass message onto system
        //----------------------------------------------------------------------
        default:
            break;
    }
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// moveToDate - moves an event forward in time to the specified time and date
//              passed in, checking for validity
////////////////////////////////////////////////////////////////////////////////
VOID moveToDate( REMINDERINDEX whichReminder, CHAR mo, CHAR dy, short yr, CHAR hr, CHAR mn )
{
    ReminderItem *ptr = reminders[whichReminder];

    while( mn > 59 ) {
        mn = mn - 60;
        hr++;
        }
    while( hr > 23 ) {
        hr = hr - 24;
        dy++;
        }
    while( dy > numberOfDaysInMonth( mo, yr ) ) {
        dy = dy - numberOfDaysInMonth( mo, yr );
        mo++;
        if (mo > 12) {
            mo = mo - 12;
            yr++;
            }
        }

    ptr->setHour( hr );
    ptr->setMinute( mn );
    ptr->setDay( dy );
    ptr->setMonth( mo );
    ptr->setYear( yr );
    ptr->updateTimes();
}


////////////////////////////////////////////////////////////////////////////////
// setSnooze - snoozes a given event by the time given in the event structure
////////////////////////////////////////////////////////////////////////////////
void setSnooze( HWND hWnd, REMINDERINDEX whichReminder )
{
    ReminderItem *ptr = reminders[whichReminder];
    CHAR count = 0;
    CHAR hr, min, dy, mo;
    SHORT yr;
    time_t timer = time(NULL);

    time_now = localtime(&timer);
    yr = time_now->tm_year + 1900;
    mo = time_now->tm_mon + 1;
    dy = time_now->tm_mday;
    hr = time_now->tm_hour;
    min = time_now->tm_min;
    unsigned int today = calcTimeOfOccurence( yr, mo, dy, hr, min );

    if ( ptr->getMinBefore() ) {
        if (( hr >= ptr->getHour() ) && ( min >= ptr->getMinute() )) {
            // delay by minutes -- event is now or even past due
            ptr->setMinsBefore( 0 );
            moveToDate( whichReminder, mo, dy, yr, hr, min+snoozeTime );
            }
        else {
            // delay by minutes -- event is in future
            INT cnt = ptr->getMinsBefore()-snoozeTime;
            do {
                ptr->setMinsBefore( MAX( ptr->getMinsBefore()-snoozeTime, 0 ) );
                ptr->updateTimes();
                } while (( ptr->getReminderTime() < today ) && (++count));
            if (cnt < 0)
                moveToDate( whichReminder, mo, dy, yr, hr, min-cnt );
            }
        }
    else {
        // delay by days -- event is past!
        if ((ptr->getYear() < yr) ||
           ((ptr->getMonth() < mo) && (ptr->getYear() == yr)) ||
           ((ptr->getDay() < dy) && (ptr->getMonth() == mo) && (ptr->getYear() == yr))) {
            ptr->setDaysBefore( 0 );
            moveToDate( whichReminder, mo, ++dy, yr, ptr->getHour(), ptr->getMinute() );
            }
        // delay by days -- event is today!
        else if (( dy == ptr->getDay() ) && ( mo == ptr->getMonth() ) &&
            ( yr == ptr->getYear() )) {
            ptr->setDaysBefore( 0 );
            moveToDate( whichReminder, mo, ++dy, yr, ptr->getHour(), ptr->getMinute() );
            }
        // delay by days -- event is in the future!
        else
            do {
                ptr->setDaysBefore( MAX( ptr->getDaysBefore()-1, 0 ) );
                ptr->updateTimes();
                } while (( ptr->getReminderTime() < today ) && (++count));
        }

    redrawCalendar();
    WinDismissDlg( hWnd, TRUE );
    snoozeOn = popStack();
}


////////////////////////////////////////////////////////////////////////////////
// drawClock - draw clock in upper left of passed window
////////////////////////////////////////////////////////////////////////////////
VOID drawClock( HPS hps, HWND hWnd, CHAR timeIncluded, CHAR hr, CHAR mn )
{
    RECTL rect;
    CHAR tmpBuffer[12];

    if (showClock) {
        if (!timeIncluded) {
            time_t timer = time(NULL);
            time_now = localtime(&timer);
            hr = time_now->tm_hour;
            mn = time_now->tm_min;
            }
        WinQueryWindowRect( hWnd, &rect );
        sprintf( tmpBuffer, " %c%c:%2d%c  ", hourArray[hr][0], hourArray[hr][1], mn, hourArray[hr][2] );
        if (tmpBuffer[4]==' ')
            tmpBuffer[4]='0';
        WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect, 0, 0,
                     DT_QUERYEXTENT);
        rect.yBottom++;
        WinDrawText( hps, strlen(tmpBuffer), tmpBuffer, &rect, CalText,
                     CalButtonMiddle, DT_ERASERECT|DT_LEFT);
        }
}


////////////////////////////////////////////////////////////////////////////////
// jumpToProc - window that handles user jumping to specified month & year
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY jumpToProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch( msg )
    {
        //----------------------------------------------------------------------
        // when the dialog is being initialized, center it on desktop and check
        // the appropriate radio button for whatever setting they currently have
        //----------------------------------------------------------------------
        case WM_INITDLG: {
            char *Months[12];
            struct tm curDate;

            centerDialog( hWnd );
            WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_SETARRAY, 0, 0 );
            WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_SETLIMITS, MPFROMLONG(2200), MPFROMLONG(1900) );
            WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_SETCURRENTVALUE, MPFROMLONG(curYear), 0 );

            for (int z=0; z<12; z++) {
               curDate.tm_mon = z;
               Months[z] = new char[24];
               strftime( Months[z], 24, "%B", (const tm*)&curDate );
               }

            WinSendMsg( WinWindowFromID( hWnd, MONTH ), SPBM_SETARRAY, MPFROMLONG(Months), MPFROMSHORT(12) );
            WinSendMsg( WinWindowFromID( hWnd, MONTH ), SPBM_SETCURRENTVALUE, MPFROMLONG(curMonth-1), MPFROMLONG(0) );
            for (z=0; z<12; z++)
                delete Months[z];

            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // dismiss dialog and redraw real calendar if user is done
                //--------------------------------------------------------------
                case DID_OK: {
                    LONG tmpVal;

                    WinSendMsg( WinWindowFromID( hWnd, MONTH ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    curMonth = tmpVal + 1;
                    WinSendMsg( WinWindowFromID( hWnd, YEAR ), SPBM_QUERYVALUE, MPFROMLONG(&tmpVal), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    curYear = tmpVal;
                    WinDismissDlg( hWnd, TRUE );
                    return 0;
                    }
                //--------------------------------------------------------------
                // dismiss dialog
                //--------------------------------------------------------------
                case DID_CANCEL: {
                    WinDismissDlg( hWnd, FALSE );
                    return 0;
                    }
                }
            }
        //----------------------------------------------------------------------
        // if nothing further we want to intercept, pass message onto system
        //----------------------------------------------------------------------
        default:
            break;
    }
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


////////////////////////////////////////////////////////////////////////////////
// print a line of the calendar
////////////////////////////////////////////////////////////////////////////////
VOID printLine( FILE* outFile, CHAR pageWidth, CHAR horzRule, SHORT *dayOfWeek,
                SHORT numDaysInMonth )
{
    INT total = ((pageWidth-1)/7)-1;
    INT z, z1;

    for(z=0;z<(pageWidth-((7*total)+1))/2;z++)
        fprintf(outFile," ");

    fprintf(outFile,(horzRule==1)?"+":"|");
    for(z=1;z<8;z++) {
        for(z1=1; z1<total; z1++)
            if (horzRule==0)
                fprintf(outFile," ");
            else if (horzRule==1)
                fprintf(outFile,"-");
            else {
                if (z1<total-2)
                    fprintf(outFile," ");
                else {
                    if ((*dayOfWeek>=0) && (*dayOfWeek<numDaysInMonth))
                        fprintf( outFile,"%2d", (*dayOfWeek)+1 );
                    else
                        fprintf( outFile,"  ");
                    z1++;
                    }
                }
        fprintf(outFile,(horzRule==1)?"+":"|");
        if (horzRule==2)
            (*dayOfWeek)++;
        }
    fprintf(outFile,"\n");
}


////////////////////////////////////////////////////////////////////////////////
// cneter the passed line and send to passed output file
////////////////////////////////////////////////////////////////////////////////
VOID centerLine( FILE *outFile, CHAR *tmp, CHAR pageWidth )
{
    for(int z=0;z<((pageWidth-strlen(tmp))/2);z++) fprintf(outFile, " ");
    fprintf(outFile, tmp);
    fprintf(outFile, "\n");
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
VOID print( CHAR pageWidth, CHAR printCalendar, CHAR toDoListType )
{
    CHAR tmpBuffer[MAXITEMSIZE+32];
    RXSTRING INSTORE[2];
    RXSTRING retstr;
    SHORT ReturnCode;
    INT z,z1,z2;
    SHORT currentDay;

    FILE *outFile = fopen("OUTFILE.TMP","w");
    if (outFile==NULL) {
        WinMessageBox( HWND_DESKTOP, calendar.frame, MSG143, MSG004, 0,
                       MB_ERROR|MB_OK );
        return;
        }

    //==========================================================================
    // print the calendar
    //==========================================================================
    if (printCalendar) {
        struct tm curDate;

        curDate.tm_year = curYear-1900;
        curDate.tm_mon = curMonth-1;
        curDate.tm_mday = 1;
        curDate.tm_hour = 0;
        curDate.tm_min = 0;
        curDate.tm_sec = 0;
        curDate.tm_isdst = -1;

        totalDaysInMonth = (INT)numberOfDaysInMonth( curMonth, curYear );
        mktime(&curDate);

        strftime( tmpBuffer, 128, MONTHSTRING, (const tm*)&curDate );
        centerLine( outFile, tmpBuffer, pageWidth );
        fprintf( outFile, "\n" );

        currentDay = -curDate.tm_wday;
        printLine( outFile, pageWidth, TRUE, &currentDay, totalDaysInMonth );
        for (z1=0; z1<6; z1++) {
            printLine( outFile, pageWidth, 2, &currentDay, totalDaysInMonth );
            for (z2=0; z2<pageWidth/30; z2++)
                printLine( outFile, pageWidth, 0, &currentDay, totalDaysInMonth );
            printLine( outFile, pageWidth, 1, &currentDay, totalDaysInMonth );
            }
        for (z2=0; z2<pageWidth/30; z2++)
            fprintf(outFile,"\n");
        }


    //==========================================================================
    // print the to do list
    //==========================================================================
    if (toDoListType) {
        struct tm curDate;
        CHAR tmp[81];
        INT yr, mo, dy;
        INT tmpYr, tmpMo, tmpDy;

        tmpYr = todaysDate.da_year;
        tmpMo = todaysDate.da_mon;
        tmpDy = todaysDate.da_day;
        curDate.tm_hour = 0;
        curDate.tm_min = 0;
        curDate.tm_sec = 0;
        curDate.tm_isdst = -1;

        if (toDoListType == TODOMONTH)
            tmpDy = totalDaysInMonth = (INT)numberOfDaysInMonth( tmpMo, tmpYr );
        else if (toDoListType == TODOWEEK) {
            tmpDy--;
            do {
                tmpDy++;
                if (tmpMo > 12) {
                    tmpMo = 1;
                    tmpYr++;
                    }
                curDate.tm_year = tmpYr-1900;
                curDate.tm_mon = tmpMo-1;
                curDate.tm_mday = tmpDy;
                mktime(&curDate);
                } while (curDate.tm_wday != 6);
            }

        for(z=1;z<pageWidth;z++)
            fprintf(outFile,"=");
        fprintf(outFile,"\n");

        curDate.tm_year = tmpYr-1900;
        curDate.tm_mon = tmpMo-1;
        curDate.tm_mday = tmpDy;

        mktime(&curDate);
        strcpy( tmpBuffer, MSG145 );
        strftime( tmp, 80, CURDATESTR4, (const tm*)&curDate );
        strcat( tmpBuffer, tmp);
        centerLine( outFile, tmpBuffer, pageWidth );

        for(z=1;z<pageWidth;z++)
            fprintf(outFile,"=");
        fprintf(outFile,"\n");

        for (REMINDERINDEX z=0; z<numReminders; z++) {
        ReminderItem *tmp = reminders[z];

        if ((tmp->getToDo()) && (!tmp->getItemDone())) {
            yr = tmpYr;
            mo = tmpMo;
            dy = tmpDy + tmp->getNumToShow();

            while ( dy > numberOfDaysInMonth( mo, yr ) ) {
                dy = dy - numberOfDaysInMonth( mo, yr );
                mo++;
                if (mo > 12) {
                    mo = mo - 12;
                    yr++;
                    }
                }

            if ( tmp->getTimeOfOccurence() <= calcTimeOfOccurence(yr,mo,dy,23,59) ) {
                    char mo[4],mn[4];
                    char day = tmp->getDay();
                    char hour = tmp->getHour();
                    char tmpDate[16];
                    int tmpPtr = 0;
                    int count = 0;

                    sprintf(mn, "%2d", tmp->getMinute() );
                    if( mn[0]==' ')
                        mn[0]='0';

                    if (tmp->getMonth()==ALLMONTHS)
                        strcpy( mo, "--" );
                    else
                        sprintf(mo, "%2d", tmp->getMonth() );

                    tmpDate[0] = 0;
                    if( toDoDisplay == DATEONLY)
                        #ifdef ENGLISH
                        sprintf( tmpDate, MONTHDAY, mo, day );
                        #else
                        sprintf( tmpDate, MONTHDAY, day, mo );
                        #endif
                    else if( toDoDisplay == TIMEONLY)
                        sprintf( tmpDate, HOURMIN, hourArray[hour], mn );
                    else if( toDoDisplay == DATEANDTIME)
                        #ifdef ENGLISH
                        sprintf( tmpDate, MONTHANDHOUR, mo, day, hourArray[hour], mn );
                        #else
                        sprintf( tmpDate, MONTHANDHOUR, day, mo, hourArray[hour], mn );
                        #endif

                    strcpy( tmpBuffer, tmpDate );
                    strcat( tmpBuffer, "* " );
                    strcat( tmpBuffer, tmp->getText() );

                    while( tmpBuffer[tmpPtr] ) {
                        fprintf(outFile,"%c",tmpBuffer[tmpPtr++]);
                        count++;
                        if (count==pageWidth-1) {
                             fprintf(outFile, "\n            ");
                             count = 12;
                             }
                        }
                    fprintf(outFile, "\n");
                }
            }
        }

        for (z2=0; z2<pageWidth/30; z2++)
            fprintf(outFile,"\n");
        }

    sprintf( tmpBuffer, "%s - %s", MSG007, VERSION );
    centerLine( outFile, tmpBuffer, pageWidth );
    fclose( outFile );
    //--------------------------------------------------------------------------
    // call REXX to send our ASCII output to the printer
    //--------------------------------------------------------------------------
    sprintf(tmpBuffer, "/**/copy OUTFILE.TMP prn; delete OUTFILE.TMP;");
    INSTORE[0].strptr=tmpBuffer;
    INSTORE[0].strlength=strlen(tmpBuffer);
    INSTORE[1].strptr=NULL;
    INSTORE[1].strlength=0;

    rexxStart((LONG)0,
              (PRXSTRING)0,
              (PSZ)LAUNCHFILE,
              (PRXSTRING)INSTORE,
              (PSZ)"CMD",
              (LONG)RXCOMMAND,
              (PRXSYSEXIT)0,
              (SHORT*)&ReturnCode,
              (PRXSTRING)&retstr);
}


////////////////////////////////////////////////////////////////////////////////
// printProc - print calendar and/or todo list
////////////////////////////////////////////////////////////////////////////////
MRESULT EXPENTRY printProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch( msg )
    {
        //----------------------------------------------------------------------
        // when the dialog is being initialized, center it on desktop and check
        // the appropriate radio button for whatever setting they currently have
        //----------------------------------------------------------------------
        case WM_INITDLG: {
            centerDialog( hWnd );
            WinCheckButton( hWnd, PRINTTODO, printToDo );
            WinCheckButton( hWnd, PRINTCALENDAR, printCal );
            WinCheckButton( hWnd, PRTWEEK, TRUE );
            WinSendMsg( WinWindowFromID( hWnd, WIDTH ), SPBM_SETLIMITS, MPFROMLONG(160), MPFROMLONG(40) );
            WinSendMsg( WinWindowFromID( hWnd, WIDTH ), SPBM_SETCURRENTVALUE, MPFROMLONG(printWidth), 0 );
            return WinDefDlgProc( hWnd, msg, mp1, mp2 );
            }
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        case WM_COMMAND: {
            switch( SHORT1FROMMP(mp1) ) {
                //--------------------------------------------------------------
                // dismiss dialog and redraw real calendar if user is done
                //--------------------------------------------------------------
                case DID_OK: {
                    WinSendMsg( WinWindowFromID( hWnd, WIDTH ), SPBM_QUERYVALUE, MPFROMLONG(&printWidth), MPFROM2SHORT(0,SPBQ_DONOTUPDATE) );
                    printToDo = (WinQueryButtonCheckstate( hWnd, PRINTTODO )?TRUE:FALSE);
                    printCal = (WinQueryButtonCheckstate( hWnd, PRINTCALENDAR )?TRUE:FALSE);
                    if (!printToDo)
                        print( printWidth, printCal, FALSE );
                    else if (WinQueryButtonCheckstate( hWnd, PRTTODAY ))
                        print( printWidth, printCal, TODOTODAY);
                    else if (WinQueryButtonCheckstate( hWnd, PRTWEEK ))
                        print( printWidth, printCal, TODOWEEK);
                    else print( printWidth, printCal, TODOMONTH);
                    WinDismissDlg( hWnd, TRUE );
                    return 0;
                    }
                //--------------------------------------------------------------
                // dismiss dialog
                //--------------------------------------------------------------
                case DID_CANCEL: {
                    WinDismissDlg( hWnd, FALSE );
                    return 0;
                    }
                }
            }
        //----------------------------------------------------------------------
        // if nothing further we want to intercept, pass message onto system
        //----------------------------------------------------------------------
        default:
            break;
    }
    return WinDefDlgProc( hWnd, msg, mp1, mp2 );
}


