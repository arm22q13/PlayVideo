// main.cpp of ShutDown program
//
//  Support a Shutdown button.  Because the program is run from /usr/bin, the program can execute a shutdown.
//
//  v 0.1  31 July 2017  Initial version.
//  v 0.2  11 Aug  2017  Changed GPIO pin
//  v 0.3   3 Oct  2017  Eject all USB drives
//  v 0.4  15 Oct  2017  Delete directories VIDEOS,VIDEOS1,VIDEOS2,VIDEOS3,VIDEOS4
//                       These can be phantom directories owned by ROOT if SHUTDOWN does not
//                       go smoothly.
//
// Note:  The eject commands hard-code the video flash drive names.  In the future, we should
// use the environmental variables and the list.txt file to determine what is mounted.
// The best approach may be to use the environment to find the path of the main USB drive and then
// list all the directories on the same path.
// If system does not find the USB flash drive upon power-up, do a proper startup and shutdown
// once or twice. This will erase phantom directories and restore normal path names and ownerships.

#include <iostream>
#include <stdlib.h>
#include <wiringPi.h>
#include <linux/reboot.h>

using namespace std;

//	GPIO pin number
// const int SHUTDOWN_BUTTON    =  23;
const int SHUTDOWN_BUTTON    =  4;

int main()  {

   // Initialize pushbutton: input with pull-up
   wiringPiSetupGpio ();
   pinMode(SHUTDOWN_BUTTON,INPUT);
   pullUpDnControl(SHUTDOWN_BUTTON,PUD_UP);

   if (!digitalRead(SHUTDOWN_BUTTON)) {  // Button pressed?
      delay(100);  // glitch filter
      if (!digitalRead(SHUTDOWN_BUTTON)) {  // power down
         system("killall omxplayer.bin");
         delay(500);
         system("sudo eject /media/pi/VIDEOS");
         system("sudo eject /media/pi/VIDEOS1");
         system("sudo eject /media/pi/VIDEOS2");
         system("sudo eject /media/pi/VIDEOS3");
         system("sudo eject /media/pi/VIDEOS4");
         delay(500);
         system("sudo rmdir /media/pi/VIDEOS");
         system("sudo rmdir /media/pi/VIDEOS1");
         system("sudo rmdir /media/pi/VIDEOS2");
         system("sudo rmdir /media/pi/VIDEOS3");
         system("sudo rmdir /media/pi/VIDEOS4");
         delay(200);
         system("shutdown -P now");
      } // power down
   } // button pressed?

} // end main
