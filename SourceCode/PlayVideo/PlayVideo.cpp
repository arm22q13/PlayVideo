// PlayVideo.cpp
//
#include <sys/types.h>
#include <signal.h>
#include "PlayVideo.h"

//
// implementation of class PlayVideo
//

// initialize the player
void PlayVideo::initialize(string player_filename, string player_options)
{
    child_process_PID = -1;
    // extract the DVD library path from the input_list_filename (path and name of list file)
    PPPath = player_filename + " ";
    PPOptions = player_options;
}

// start the player in a child process
// Returns true if start was successful
bool PlayVideo::playStart(videospec_t video)
{
    FILE *ftest;
    string dvd_filename;
    string loopOption;

    // check to see if this video file is to be looped repeatedly
    // if so, it's name is prepended with the loop mark.
    if ((video.dvd_filename.length() > 2) && (video.dvd_filename.at(0) == LOOP_VIDEO_MARK))
    {
        dvd_filename = video.dvd_filename.substr(1); // remove prepended character
        loopOption = " --loop ";                     // tell omxplayer to loop this video indefinitely
    }
    else
    {
        dvd_filename = video.dvd_filename;
        loopOption = " ";
    }

    string PPVolume = "--vol " + SSTR(video.volume + SYSTEM_VOLUME);
    string VFN = video.flash_drive_path + dvd_filename;
    string playString = PPPath + PPVolume + " " + PPOptions + loopOption + " \"" + VFN + "\"";
    // printf("PV: playString\n");
    cout << "PV: " << playString << endl;

    // Make sure the video file exists and can be opened
    cout << "PV: ... looking for: " << VFN << endl;
    // requires fopen64() to recognize files larger than 2.147 GB
    ftest = fopen64(VFN.c_str(), "rb");
    if (ftest != NULL)
    {
        fclose(ftest); // successful test
        cout << "PV: ...found it." << endl;
    }
    else
    {
        cout << "PV: ...cannot open video file." << endl;
        return false;
    }
    child_process_PID = fork();
    if (child_process_PID < 0)
    {
        cout << "PV: FORK FAILED." << endl;
        return false; // some hope of recovery, but not much.
    }
    
    if (child_process_PID == 0)
    { // CHILD
        cout << endl
             << "PV: CHILD process is starting the video player" << endl
             << "PV: PID number is " << getpid() << endl;

        // This is a blocking function. So the child process will just sit here
        // until the process ends
        system(playString.c_str());

        // when the video player process is killed, the Child process continues here.
        cout << endl    
             << "PV: CHILD: Video player terminated" << endl;
        exit(0);
    } // CHILD

    this->MyChildPID = child_process_PID;
    printf("PV: child pid: %d\n", child_process_PID);

    return true; // PARENT
} //playStart

// kill the existing child player process
void PlayVideo::playEnd()
{
    cout << endl 
         << "PV: Killing player" << endl;
    system("killall omxplayer.bin");

    cout << endl
         << "PV: Waiting for process to teminate" << endl;

    // we don't need to check if player died; either it did or it didn't
    //      delay(KILL_WAIT_TIME);
    //
    //      // The following is for debugging.
    //      // See if omxplayer is still running after the kill attempt
    //      // If we find out it is running, we need to increase KILL_WAIT_TIME or find
    //      // another way to assure the old process is killed.
    //      ExecuteCommand CMD;
    //      string grepResult = CMD.execute("ps ax | grep omxplayer");
    //      cout << "PV: ps ax found: " << endl;
    //      cout << grepResult << endl;
    cout << "PV: Done killing player." << endl;
} // playEnd

// return whether child player process is still active
bool PlayVideo::isPlaying()
{
    bool playing = true;

    // if we don't have a child process, we aren't running
    if (MyChildPID == 0)
        playing = false;

    // check the child process execution state
    int pid = kill(MyChildPID, 0);
    if (0 != pid)
    {
        playing = false;
        MyChildPID = 0;
    }

    //  cout << "PV: isPlaying: " << playing << " kill(" << MyChildPID << "): " << pid << endl;

    return playing;
}
