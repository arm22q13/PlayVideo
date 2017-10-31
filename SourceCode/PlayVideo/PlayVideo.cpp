// PlayVideo.cpp
//
#include "PlayVideo.h"

//
// implementation of class PlayVideo
//

void PlayVideo::initialize(string player_filename, string player_options) {
   child_process_PID = -1;
   // extract the DVD library path from the input_list_filename (path and name of list file)
   PPPath = player_filename + " ";
   PPOptions = player_options;
}

// Returns true if start was successful
bool PlayVideo::playStart(videospec_t video) {
   FILE *ftest;
   string PPVolume = "--vol " + SSTR(video.volume+SYSTEM_VOLUME);
   string VFN = video.flash_drive_path+video.dvd_filename;
   string playString = PPPath + PPVolume + " " + PPOptions + " \"" + VFN + "\"";
   printf("PV: playString\n");
   cout << "PV: " << playString << endl;

   // Make sure the video file exists and can be opened
   cout << "PV: ...looking for: " << VFN << endl;
   ftest = fopen(VFN.c_str(), "rb");
   if (ftest!=NULL) {
       fclose(ftest);  // successful test
       cout << "PV: ...found it." << endl;
   } else {
      cout << "PV: ...cannot open video file." << endl;
      return false;
   }
   child_process_PID = fork();
   if (child_process_PID < 0) {
      printf("PV: FORK FAILED.\n");
      return false; // some hope of recovery, but not much.
   }
   if (child_process_PID == 0) { // CHILD
      printf("\nPV: CHILD process is starting the video player\n");
      MyChildPID = getpid();
      cout << "PID number is " << getpid() << endl;
      // This is a blocking function. So the Child process will just sit here.
      system(playString.c_str());
      // When the video player process is killed, the Child process continues here.
      printf("\nPV: CHILD: Video player terminated\n");
      exit(0);
   } // CHILD
   return true;  // PARENT
} //playStart

void PlayVideo::playEnd(){
      printf("\nPV: Killing player\n");
      system("killall omxplayer.bin");
      printf("\nPV: Waiting for process to teminate\n");
      delay(KILL_WAIT_TIME);
      // The following is for debugging.
      // See if omxplayer is still running after the kill attempt
      // If we find out it is running, we need to increase KILL_WAIT_TIME or find
      // another way to assure the old process is killed.
      ExecuteCommand CMD;
      string grepResult = CMD.execute("ps ax | grep omxplayer");  
      cout << "PV: ps ax found: " << endl;
      cout << grepResult << endl;
      printf("PV: Done killing player. \n");
} // playEnd
