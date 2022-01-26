// ListManager.h
//
//  The ListManager class keeps track of the list of video files to play and the volume settings for
//  each video.
//  Updated to PlayVideo version 2.1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <wiringPi.h>
#include <sstream>
using namespace std;

#ifndef _VIDEOSPEC_H
#define _VIDEOSPEC_H

typedef struct videospec {
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
const string whitespace = " \t";


class ListManager {

   private:
      string list_filename;  // full path and name of list file
	  string max_reboot_string; // maximum number of reboot attempts
      videospec_t videos[MAXVIDEOFILES];  // Pre-allocate space
      int last_file_pointer;
      int current_file_pointer;
      string remove_char( string str, char ch);
      string trim (const string str);
	  int RebootCounter(int action, int max_reboot_count);
   public:
      void initialize(string input_list_filename, string max_reboot);
      videospec_t currentVideo();
      videospec_t nextVideo();
      videospec_t previousVideo();
      int videoCount();
      void resetVideoPointer();

}; // ListManager


#endif
