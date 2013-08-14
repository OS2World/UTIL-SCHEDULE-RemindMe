////////////////////////////////////////////////////////////////////////////////
//
// Reminder Item Class - Class Definition and misc macro definitions
//
// Written By Eric A. Wolf
// Copyright (C) 1995 - All Rights Reserved
//
////////////////////////////////////////////////////////////////////////////////
#ifndef REMINDER_CLASS_H
#define REMINDER_CLASS_H


////////////////////////////////////////////////////////////////////////////////
// external function declared in the .cpp portion of class
////////////////////////////////////////////////////////////////////////////////
void setItem( char **, char * );


////////////////////////////////////////////////////////////////////////////////
// ReminderItem Class - each calendar entry is stored as instance of this class
////////////////////////////////////////////////////////////////////////////////
class ReminderItem
{
public:
    // constructor/destructor
    ReminderItem( char, char, char, char, char, short, char, char, char, char,
                  char, char, char, char*, char*, char );
    ~ReminderItem();

    // get members
    char getMonth()                   { return month; }
    char getDay()                     { return day; }
    char getHour()                    { return hour; }
    char getMinute()                  { return minute; }
    char getToDo()                    { return todoItem; }
    char getItemDone()                { return itemDone; }
    char getReminder()                { return flashReminder; }
    char getReminderType()            { return flashType; }
    char getNumToShow()               { return numDaysToShow; }
    char getMinBefore()               { return minBefore; }
    char getMinsBefore()              { return minsBefore; }
    char getDaysBefore()              { return daysBefore; }
    char getReschedTime()             { return rescheduleTime; }
    char *getText()                   { return itemText; }
    char *getSoundFile()              { return soundFile; }
    unsigned int getYear()            { return year; }
    unsigned int getTimeOfOccurence() { return timeOfOccurence; }
    unsigned int getReminderTime()    { return timeOfReminder; }

    // set members
    void setText( char *txt )         { setItem( &itemText, txt ); }
    void setSoundFile( char *tx2 )    { setItem( &soundFile, tx2 ); }
    void setHour( char hr )           { hour = hr; }
    void setMinute( char mn )         { minute = mn; }
    void setYear( unsigned short yr ) { year  = yr; }
    void setMonth( char mn )          { month = mn; }
    void setDay( char dy )            { day = dy; }
    void setToDo( char td )           { todoItem = td; }
    void setItemDone( char id )       { itemDone = id; }
    void setReminder( char sr )       { flashReminder = sr; }
    void setReminderType( char ft )   { flashType = ft; }
    void setNumToShow( char nm )      { numDaysToShow = nm; }
    void setMinBefore( char mb )      { minBefore = mb; }
    void setMinsBefore( char me )     { minsBefore = me; }
    void setDaysBefore( char db )     { daysBefore = db; }
    void setReschedTime( char st )    { rescheduleTime = st; }
    void updateTimes( void )          { setReminderTime(); setTimeOfOccurence(); }
    void setTimeOfOccurence( void );
    void setReminderTime( void );

private:
    unsigned int year;
    unsigned int timeOfOccurence;
    unsigned int timeOfReminder;
    char *itemText;
    char *soundFile;
    char rescheduleTime;
    char month;
    char day;
    char hour;
    char minute;
    char todoItem;
    char itemDone;
    char flashReminder;
    char flashType;
    char minBefore;
    char daysBefore;
    char minsBefore;
    char numDaysToShow;            // # of days in advance to show on to-do list
};


////////////////////////////////////////////////////////////////////////////////
// Misc. Macro Definitions - used both in class and in main application
////////////////////////////////////////////////////////////////////////////////
// calcTimeOfOccurence - give unique ordinality to each possible date and time
#define calcTimeOfOccurence(YEAR,MONTH,DAY,HOUR,MIN) (YEAR-1900)*14*33*24*60 + \
                                                          (MONTH-1)*33*24*60 + \
                                                               (DAY-1)*24*60 + \
                                                                     HOUR*60 + \
                                                                         MIN

// MakeHourglassPointer - macro to make the mouse pointer the hourglass
#define MakeHourglassPointer()  WinSetPointer( HWND_DESKTOP, WinQuerySysPointer( HWND_DESKTOP, SPTR_WAIT, FALSE) );

// MakeArrowPointer - macro to make the mouse pointer the arrow
#define MakeArrowPointer()      WinSetPointer( HWND_DESKTOP, WinQuerySysPointer( HWND_DESKTOP, SPTR_ARROW, FALSE) );

// DrawRect - draws a rectange from the give 4 coordinates in specified color
#define DrawRect(x1,y1,x2,y2,color)  pt.x = (LONG) (x1);             \
                                     pt.y = (LONG) (y1);             \
                                     GpiSetCurrentPosition(hps,&pt); \
                                     pt.x = (LONG) (x2);             \
                                     pt.y = (LONG) (y2);             \
                                     GpiSetColor(hps,color);         \
                                     GpiBox(hps,DRO_FILL,&pt,0L,0L);

// MAX - returns max of two passed values
#define MAX(A,B)      ((A)>(B)?(A):(B))

// redrawCalendar - redraws the calendar on the desktop
#define redrawCalendar()  WinInvalidateRegion( calendar.client, NULLHANDLE, FALSE ); \
                          fillToDoList();

#endif
