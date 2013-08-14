////////////////////////////////////////////////////////////////////////////////
//
// Reminder Item Class - Member Functions
//
// Written By Eric A. Wolf
// Copyright (C) 1995 - All Rights Reserved
//
////////////////////////////////////////////////////////////////////////////////
#include "rmndclas.h"
#include <string.h>


////////////////////////////////////////////////////////////////////////////////
// function prototype
////////////////////////////////////////////////////////////////////////////////
void setItem( char **, char * );
char numberOfDaysInMonth( char, short );


////////////////////////////////////////////////////////////////////////////////
// ReminderItem - create a new reminder item.  Fill in month/day/year, time and
//                text for reminder when called
////////////////////////////////////////////////////////////////////////////////


ReminderItem::ReminderItem( char todo, char itemDone, char numShow, char month,
                            char day, short year, char hour, char minute,
                            char reminder, char reminderType, char minBefore,
                            char minsBefore, char daysBefore, char *sound,
                            char *txt, char resched )
{
    soundFile = 0;
    itemText = 0;
    setToDo( todo );
    setItemDone( itemDone );
    setNumToShow( numShow );
    setMonth( month );
    setDay( day );
    setYear( year );
    setHour( hour );
    setMinute( minute );
    setReminder( reminder );
    setReminderType( reminderType );
    setMinBefore( minBefore );
    setMinsBefore( minsBefore );
    setDaysBefore( daysBefore );
    setSoundFile( sound );
    setText( txt );
    setReschedTime( resched );
    setTimeOfOccurence();
    setReminderTime();
}


////////////////////////////////////////////////////////////////////////////////
// when deleteing a reminder item, delete any dynamic string space
////////////////////////////////////////////////////////////////////////////////
ReminderItem::~ReminderItem()
{
    if( itemText )
        delete( itemText );
    if( soundFile )
        delete( soundFile );
}


////////////////////////////////////////////////////////////////////////////////
// give ordanilty to all unique integer time and dates possible
////////////////////////////////////////////////////////////////////////////////
void ReminderItem::setTimeOfOccurence( void )
{
    timeOfOccurence = calcTimeOfOccurence( year, month, day, hour, minute );
}


////////////////////////////////////////////////////////////////////////////////
// calculate the time when the flash reminder is supposed to go off
////////////////////////////////////////////////////////////////////////////////
void ReminderItem::setReminderTime( void )
{
    short yr;
    char mo, dy, hr, mn;

    yr = getYear();
    mo = getMonth();
    dy = getDay();
    hr = getHour();
    mn = getMinute();

    if ( getMinBefore() )
        mn = mn - getMinsBefore();
    else
        dy = dy - getDaysBefore();

    if ( mn < 0 ) {
        mn = mn + 60;
        hr--;
        }
    if ( hr < 0 ) {
        hr = hr + 24;
        dy--;
        }
    if ( dy < 1 ) {
        mo--;
        if( mo < 1 ) {
            mo = mo + 12;
            yr--;
            }
        dy = dy + numberOfDaysInMonth( mo, yr );
        }

    timeOfReminder = calcTimeOfOccurence( yr, mo, dy, hr, mn );
}


////////////////////////////////////////////////////////////////////////////////
// an intelligent strcpy routine for use with dynamic strings
////////////////////////////////////////////////////////////////////////////////
void setItem( char **destination, char *source ) {
    if (*destination)
        delete *destination;
    if (source == 0) {
        *destination = new char[2];
        strcpy( *destination, "" );
        }
    else {
        *destination = new char[ strlen( source )+1 ];
        strcpy( *destination, source );
        }
}

