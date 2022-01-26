// main.cpp of PlayVideo program
//
//  Single or double pushbutton-based video jukebox player for individuals with disabilities.
//  A. Mitz 2017.  This is the 2nd generator version of software developed for D. Mitz. The original
//  is called FreedomDVD and was developed for Windows XP.  This version is designed for a Raspberry
//  Pi 3 running Raspbian, a Debian Linux variant.
//
//  Updated 1/20/2022 by J. Mitz.
//  Functions have been added to allow the system to automatically reboot a defined number of times if
//  the system is unable to locate the list file. This facilitates the unmounting and remounting of drives.
//
//  PlayVideo assumes there is a flash drive of size 64 MB formatted at FAT32 plugged into
//  one of the USB sockets. A SD or microSD card formatted as FAT32 will also work if USB adapter is used.
//  The USB drive must have .MP4 (or other) video files on the root directory along with a special
//  text file called list.txt. The text file must have an ordered DVD file name list that determines
//  the order in which videos can be selected.  Each entry in that file starts with a volume value
//  (positive or negative) that is the gain parameter in decibels. The gain value is followed by
//  spaces or tabs then the video file name.
//
//  If the first character of a line is '*', that line is treated as a comment. Blank lines are ignored.
//  If the first character of a line is '@', that line redefines the directory of the subsequent video files.
//  The @ is used when video files are spread across multiple flash drives.  The normal naming scheme is this:
//  VIDEOS   name of the main flash drive with list.txt.
//  VIDEOS1  first "extra" flash drive
//  VIDEOS2  second extra flash drive
//  ... VIDEOS9  more extra flash drives
//  Currently, VIDEOS, VIDEOS1, VIDEOS2, VIDEOS3 are hard-coded in ShutDown
//  The main flash drive will get automounted at:  /media/pi/VIDEOS
//  The assumption is that other flash drives will be mounted on the same mount directory (e.g., /media/pi/VIDEOS2 )
//  The actual requirements are:
//     DVD list file can be found using the environment variable DVDLISTFILE
//     The videos for the main flash drive are on the same directory as the list file (i.e., root directory of flash drive)
//     The name for each extra flash drive ends with a numeric (0 to 9) and is on the same subdirectory as the main flash drive
//     The videos for each extra flash drive are on the root directory of the flash drive
//  Note that there is only one list file.  See the example list files.
//  Note, this scheme requires the flash drives to have names.
//  Example list file:
//  * Example list file  /media/pi/VIDEOS/list.txt   (on USB drive called VIDEOS)
//  *volume  file-name  volume value range -6000 to 0  (no positive values)
//  -0       Eagles Hell Freezes Over.mp4
//  * This file name is prepended with @ sign.  It runs in loop mode (restarts when finished).
//  -0       @Cat Stevens Majikat.mp4
//  -400     Garth Brooks NYC.mp4
//  -1000    George Benson Live.mp4
//  * The next two videos are found on the second USB drive
//  @VIDEOS2
//  -0       The Police Synchronicity.mp4
//  -0       Sheryl Crow Rockin The Globe.mp4
//  * This file determines the play order and we can go back to the first USB drive, if desired
//  @VIDEOS
//  -600     Bonnie Raitt.mp4
//
//  As just discussed, the location of the list.txt file is defined in the environment variable DVDLISTFILE (see below).
//  The automatic mount of Raspbian will mount this under the user's name under the /media directory.  Since the [default]
//  user name is "pi", a USB drive named VIDEOS will be mounted at: /media/pi/VIDEOS.
//
//  PlayVideo assumes its name is "PlayVideo". That name is hard-coded when it makes sure only one instance is running.
//
//  These environment variables are set in the system file /etc/profile.
//  DVDLISTFILE="/media/pi/VIDEOS/list.txt"      full path of list file. DVD video files must be in same directory
//  DVDPLAYER="/usr/bin/omxplayer"         full path of video player. Note, only omxplayer supports hardware decoding
//  DVDPLAYEROPTIONS="--adev hdmi"     play audio through the hdmi device
//                   "--adev local"    play audio through the Raspberry Pi speaker jack
//                   "--adev both"     play audio through both
//            Other options can be included, like:  --win \"100 100 450 450\"   small video window on the desktop
//
//  Two pushbuttons are supported, one to step forward and one backward through the list of file names.
//  These buttons connect to general purpose I/O (GPIO) pins and use the WiringPi utilities
//  (http://wiringpi.com/).  Buttons trigger interrupts.  Software debounces the buttons.
//  Mechanical buttons work as do wireless keyfob buttons.  Normal debounce time can be changed
//  to a longer duration by jumpering GPIO header pins 19 and 20.
//
//  Default configuration is to play audio through both the Raspberry Pi audio port itself, and through the HDMI device
//  by including "--adev both" in the DVDPLAYEROPTIONS string. Placing a jumper between pins 25 and 26 on the GPIO header
//  with modify the options string, replacing "--adev both" with "--adev local".  This provides a simple way to disable
//  HDMI audio without requiring Linux expertise.
//
//  Coding was done with Code Blocks IDE and a relatively generic port of the gcc compiler. Currently, Code Blocks can
//  be installed using the Raspbian software manager.  WiringPi utilities appear to come with the standard installation.
//  However, you will need to add /usr/lib/libwiringPi.so and /usr/lib/libwiringPiDev.so to the link options.
//  The C++ port seems to support C99 rather than C11.
//
//  The PlayVideo program is not called directly at boot time.  For various reasons, it is easiest to
//  startup at boot time after loading an instance of the lxterminal program.
//  Put StartVideo.sh in the pi home directory and make it executable
//    #!/bin/bash
//    killall -q omxplayer.bin
//    sleep 1
//    killall -q PlayVideo
//    sleep 2
//    PlayVideo
//
//  Start the StartVideo.sh script by placing it at the end of: /home/pi/.config/lxsession/LXDE-pi/autostart
//     @lxterminal -e /home/pi/StartVideos.sh
//
//  Shutdown button: See the program ShutDown.
//
//  v 0.1  10 July 2017  Initial version working.
//  v 0.2  11 July 2017  Get location and name of list file from the environment variable  DVDLISTFILE
//  v 0.3  12 July 2017  Support for pulsed button inputs.  Added environment variable DVDPLAYER
//  v 0.4  13 July 2017  Support for Shutdown button.
//  v 0.5  16 July 2017  Support for comments in list file. Added environment variable DVDPLAYEROPTIONS
//  v 0.6  20 July 2017  Detect if this is the only copy of PlayVideo that is running
//  v 0.7  23 July 2017  Process pushbuttons the same as pulsed pushbuttons. Make more tolerant to list file errors.
//  v 0.8  23 July 2017  Mechanical pushbuttons given longer bounce time.
//  v 0.9  28 July 2017  Raised switch debounce time to 120 ms. Fixed bug in ListManager that was not counting the numbers
//                       of videos correctly or the wrap-around correctly.  Removed some unused code.
//  v 1.0  28 July 2017  Sometimes the USB flash drive does not repond to an open file request.  Try to open the list
//                       file 3 times before giving up.
//  v 1.1  11 Aug  2017  New program called ShutDown runs in Linux in the background.  That makes the Shutdown function of
//                       this program unnecessary. It has been disabled here.   New jumper option disables audio via HDMI port.
//                       IO pins remapped for easier cabling.
//  v 1.2  27 Aug 2017   ListManager now supports videos spread across multiple flash drives. Other classes changed to accomodate.
//  v 1.3  16 Sep 2017   The previous child process is not always getting killed.  This causes two videos to play simultaneously.
//                       New variable KILL_WAIT_TIME for PlayVideo class controls the wait time after a killall command.
//                       Removed left-over shutdown button code.
//  v 1.4  16 Sep 2017   ExecuteCommand converted from c program to c++ object.
//  v 1.5   3 Oct 2017   SLOW_BOUNCETIME is now the default bounce time.  Increased to 800 ms.
//  v 1.6   4 Oct 2017   Holding down the FOWARD or REVERSE button now continuously steps through the videos.
//                       KILL_WAIT_TIME=500 to make sure previous video has stopped.  SLOW_BOUNCETIME is 2000 to give the user time
//                       to release the button before the next video starts.
//  v 1.7   4 Nov 2017   Prepend the file name with the @ sign to make that video loop indefinitely.
//  v 1.8   5 Nov 2017   Bug in PlayVideo caused files greater than 2.147 GB to not be found. (fopen() replaced with fopen64())
//  v 1.9   5 Nov 2017   ListManager now tries 6 times to open the list directory
//  v 2.1  25 Jan 2022   Added ListManager functions for automatic reboots
// please update the VERSION string with each new version.

#include <iostream>
#include "PlayVideo.h"
#include <stdlib.h>
#include <wiringPi.h>
#include "ListManager.h"
#include <linux/reboot.h>
#include "ExecuteCommand.cpp"

using namespace std;

const string VERSION = "v 1.9  5 Nov 2017";


//	GPIO pin numbers
const int FORWARD_BUTTON     =   2;   // note GPIO 2 and 3 have built-in 1.8K pullup resistors.
const int REVERSE_BUTTON     =   3;
const int FAST_DEBOUNCE      =  10;
const int DISABLE_HDMI_AUDIO =   7;

static volatile int forwardButtonFlag =  0 ;
static volatile int reverseButtonFlag =  0 ;

// Environment variables that locate list file and DVD player program
const char LIST_FILE_ENV_VAR[] = "DVDLISTFILE";
const char DVD_PLAYER_ENV_VAR[] = "DVDPLAYER";
const char DVD_PLAYER_OPTIONS_ENV_VAR[] = "DVDPLAYEROPTIONS";
const char REBOOT_ATTEMPTS_ENV_VAR[] = "REBOOTATTEMPTS";

static int MyPID = 0;

// SLOW_BOUNCETIME allows the user to see each video start before moving on
// to next video
const int FAST_BOUNCETIME    = 120;   // milliseconds
const int SLOW_BOUNCETIME    = 2000;

void forwardButtonISR (void) {
  forwardButtonFlag=1 ;
}
void reverseButtonISR (void) {
  reverseButtonFlag=1 ;
}


int main()  {
   cout << "PlayVideo " << VERSION << endl;

  // If PlayVideo is already running, exit immediately.
   ExecuteCommand CMD;
   string grepResult = CMD.execute("ps ax | grep PlayVideo | grep -v grep | grep -v lxterminal");
   cout << "ps ax found: " << endl;
   cout << grepResult << endl;
   // Count the instances of this program
   size_t found = grepResult.find("PlayVideo");
   if (found!=string::npos) {
      cout << "first occurance.  This is us." << endl;
      // Get PID numbers
      sscanf(grepResult.c_str(),"%d",&MyPID);
      cout << "our PID is " << MyPID << endl;
      found=grepResult.find("PlayVideo",found+1);
      if (found!=string::npos) {
        cout << "second occurance.  This is someone else " << endl;
        cout << "PlayVideo is already running.  Quitting." << endl;
        exit(0);
      }
   }
   else {
      cout << "Error: the PlayVideo program must be called 'PlayVideo'" << endl;
      exit(0);
   }

   // Initialize Pushbuttons: set up input pins with pull-ups
   cout << "Initialize buttons" << endl;
   wiringPiSetupGpio ();
   pinMode(FORWARD_BUTTON,INPUT);
   pinMode(REVERSE_BUTTON,INPUT);
   pinMode(FAST_DEBOUNCE,INPUT);
   pinMode(DISABLE_HDMI_AUDIO,INPUT);
   pullUpDnControl(FORWARD_BUTTON,PUD_UP);
   pullUpDnControl(REVERSE_BUTTON,PUD_UP);
   pullUpDnControl(FAST_DEBOUNCE,PUD_UP);
   pullUpDnControl(DISABLE_HDMI_AUDIO,PUD_UP);
   // Attach ISRs
   wiringPiISR (FORWARD_BUTTON, INT_EDGE_FALLING, &forwardButtonISR);
   wiringPiISR (REVERSE_BUTTON, INT_EDGE_FALLING, &reverseButtonISR);

   // Report the type of button in use
   if (!digitalRead(FAST_DEBOUNCE)) cout << " Using slow debounce (normal)" << endl;
   else cout << " Using fast debounce" << endl;

   // Locate video list and dvd player program in the environment variables.
   char *list_file_name=getenv(LIST_FILE_ENV_VAR);
   if (list_file_name == NULL) {
      cout << " Environment variable " << LIST_FILE_ENV_VAR << " not found.  Quitting!" << endl;
      exit(-1);
   }
   cout << "Will try to use this list file: " << list_file_name << endl;

   char *player_file_name=getenv(DVD_PLAYER_ENV_VAR);
   if (player_file_name == NULL) {
      cout << " Environment variable " << DVD_PLAYER_ENV_VAR << " not found.  Quitting!" << endl;
      exit(-1);
   }
   cout << "Will try to use this video player: " << player_file_name << endl;

   char *player_options=getenv(DVD_PLAYER_OPTIONS_ENV_VAR);
   if (player_options == NULL) {
      cout << " Environment variable " << DVD_PLAYER_OPTIONS_ENV_VAR << " not found. Quitting!" << endl;
      exit(-1);
   }
   
   char *reboot_count=getenv(REBOOT_ATTEMPTS_ENV_VAR);
      if (reboot_count == NULL) {
      cout << " Environment variable " << REBOOT_ATTEMPTS_ENV_VAR << " not found. Quitting!" << endl;
      exit(-1);
   }
   
   cout << "Will try to use these player option settings: " << player_options << endl;

   string PlayerOptions = player_options;

   // If necessary, modify player options to turn off HDMI audio output.  Replace "--adev both" with "--adev local"
   // if the DISABLE_HDMI_AUDIO jumper is in place.
   if (!digitalRead(DISABLE_HDMI_AUDIO)) {
      cout << " Disable HDMI audio" << endl;
      size_t option_position=PlayerOptions.find("--adev both");  // is the plan to use both audio sources (default)?
      if (option_position != string::npos) {
         // make room for one more character
         PlayerOptions.insert(option_position," ");  // does not matter what we insert.  Just need the extra space
         PlayerOptions.replace(option_position,12,"--adev local");
         cout << " New option settings: " << PlayerOptions << endl;
      }
   }  // if (!digitalRead(DISABLE_HDMI_AUDIO))

   // POPULATE LIST OF VIDEOS
   ListManager LM;
   // We give ListManager the location of the list file and player
   cout << "Fetching list of videos" << endl;
   LM.initialize(list_file_name, reboot_count);

   // START FIRST VIDEO
   PlayVideo play;
   videospec_t video;
   play.initialize(player_file_name, PlayerOptions);  //  video player and options
   video = LM.currentVideo();  // get the first video file name
   forwardButtonFlag=0;
   reverseButtonFlag=0;

   // PLAY VIDEO UNTIL A BUTTON IS PUSHED
   for (;;) {
      bool vfn_found = false;
      string vfn = video.dvd_filename;
      if (!vfn.empty()) {
         cout << "Main: Playing this file: " << vfn <<  endl;
         // look for and play the video file
         vfn_found = play.playStart(video);
         if (!vfn_found) {
            cout << "Main: That video was not found on the disk." << endl;
         }
      }
      else {
         cout << "Main: Video name is empty string" << endl;
      }

      // Delay here to give the video a chance to begin.
      // SLOW_BOUNCETIME is preferred if you want to see each video play briefly before moving to the
      // next one when the button is held down continuously.
      if (!digitalRead(FAST_DEBOUNCE)) delay(FAST_BOUNCETIME);  // select fast bounce time if jumper installed
      else delay(SLOW_BOUNCETIME);

      // if the user is holding down a button continuously, allow another repeat of the loop.
      if (!digitalRead(FORWARD_BUTTON)) forwardButtonFlag=1;
      if (!digitalRead(REVERSE_BUTTON)) reverseButtonFlag=1;

      while ((!forwardButtonFlag) && (!reverseButtonFlag)) delay(100) ;

      // Only one button at a time.
      if (forwardButtonFlag && reverseButtonFlag) reverseButtonFlag=0;

      // Note, some loop time delay comes from play.playEnd(), which has
      // a built in wait time to make sure the previous video is terminated.
      // See the value of KILL_WAIT_TIME in PlayVideo.h
      if (vfn_found) play.playEnd();  // kill current video

      if (forwardButtonFlag) {    // see if this was a forward request
         printf ("\nMain: ********** Foward button. \n") ;
         video = LM.nextVideo();  // get specifications for next video
      }
      else {
         printf ("\nMain: ********** Reverse button. \n") ;
         video = LM.previousVideo();
      } // else

      forwardButtonFlag=0; reverseButtonFlag=0;


   } // for (;;)

} // end main
