// PlayVideo.h
//
//  The PlayVideo class object plays a video and then kills the play when requested.
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <wiringPi.h>
#include <sstream>
#include<sys/types.h>
#include <signal.h>
#include "ListManager.h"
#include "ExecuteCommand.cpp"

using namespace std;

#ifndef _SSTR
#define _SSTR

//  Converts a number to a c++ string
// Use   string s = SSTR("455");
#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

#endif

#ifndef _PLAYVIDEO_H
#define _PLAYVIDEO_H

// Set the baseline loudness of all video files
const int SYSTEM_VOLUME = 0;

// Period to wait after sending a killall command
const int KILL_WAIT_TIME = 500;

class PlayVideo {

   private:
      int child_process_PID;
      int MyChildPID;
      string PPPath;
      string PPOptions;

   public:
      void initialize(string player_filename, string player_options);
      bool playStart(videospec_t video);
      void playEnd();

}; // PlayVideo

#endif
