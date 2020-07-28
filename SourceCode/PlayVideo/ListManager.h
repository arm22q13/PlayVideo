// ListManager.h
//
//  The ListManager class keeps track of the list of video files to play and the volume settings for
//  each video.
//
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <wiringPi.h>

using namespace std;

#ifndef _VIDEOSPEC_H
#define _VIDEOSPEC_H

typedef struct videospec
{
    string flash_drive_path;
    string dvd_filename;
    int volume;
} videospec_t;

// Pre-allocated space for video files names.
static const int MAXVIDEOFILES = 300;

#endif

#ifndef _LISTMANAGER_H
#define _LISTMANAGER_H

const char LIST_FILE_COMMENT_MARK = '*';
const char EXTRA_VIDEO_DISK_MARK = '@';
const char LOOP_VIDEO_MARK = '@';
const char INSTRUCTION_MARK = '$';
const string whitespace = " \t";
const int DEFAULT_MIN_PLAY_TIME_SECONDS = 0;
const bool DEFAULT_AUTO_ADVANcE_ENABLED = false;

class ListManager
{

private:
    // full path and name of list file
    string list_filename;

    // pre-allocated circular buffer of videos
    int last_file_pointer;
    int current_file_pointer;
    videospec_t videos[MAXVIDEOFILES];

    // remove all occurrences of char ch from str
    string remove_char(string str, char ch);

    // remove whitespace from the head and tail of given string
    string trim(const string str);

    // convert a string to uppercase
    string toUppercase(const string str);

    // minimum duration that a video will play
    // before user can move to the next video (in seconds)
    int minimum_play_time;

    // automatically advance to the next video when 
    // the current video ends
    bool auto_advance_enabled;

public:
    void initialize(string input_list_filename);
    videospec_t currentVideo();
    videospec_t nextVideo();
    videospec_t previousVideo();
    void resetVideoPointer();

    // accessors
    bool isAutoAdvanceEnabled();
    int minimumPlayTime();
    int videoCount();

}; // ListManager

#endif
