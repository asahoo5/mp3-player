/*
    globals.h
    Global definitions for the application 

    Developed for University of Washington embedded systems programming certificate
    
    2021/2 Nick Strathy wrote/arranged it
*/

#ifndef __GLOBALS_H
#define __GLOBALS_H

#define MAXLISTOFSONGS   100
#define SUPPFILENAMESIZE 13
#define INT_MAX          0x7FFFFFFF

extern char listOfSongs[MAXLISTOFSONGS][SUPPFILENAMESIZE];    // An array to prefetch a list of songs array
extern unsigned int sizeOfList;                           // Max allowed songs list = 100, this variable will store the number fetched songs
extern unsigned int currSongFilePntr;                     // points to current song to be played

#endif