//ListManager.cpp Ver 2.1
//Updated 1/25/2022 by Joseph Mitz
//Added RebootCounter, will reboot when the list file isn't found until the maximum number of attempts is reached

#include "ListManager.h"
// implementation of class ListManager
//
void ListManager::initialize(string input_list_filename, string max_reboot) {
   list_filename = input_list_filename;  // save the list file path
   max_reboot_string = max_reboot;      //save the reboot attempts
   int max_reboot_count = 0;
   try {
	max_reboot_count = stoi(max_reboot_string); //set the maximum reboot attempts
	}
	catch (...) {
		max_reboot_count = 1; // max_reboot did not contain a string with an integer
		cout << "Invalid value for environtmental variable REBOOTATTEMPTS" << endl;
	}
   size_t f = input_list_filename.find_last_of("/\\");
   string disk_path = input_list_filename.substr(0,f+1);  // keep slash at the end
   string line;
   int attempts = -1; //initialize attempts to invalid value
   current_file_pointer=0;
   ifstream listfile;
   if ((max_reboot_count > 3) || (max_reboot_count < 0)) //if the limit is anything but 0, 1, 2, or 3
	   max_reboot_count = 1;
   attempts = RebootCounter(1, max_reboot_count);

   for (int i=0; i<6; i++) {  // try 6 times to open the list file
      cout << "LM: Opening list file at:" << list_filename << endl;
      listfile.open(list_filename.c_str(), std::ifstream::in);
      if (!listfile.rdstate()== cout.goodbit && i==5) { //if on the 6th attempt, will start unmount proccess if reboot attemps have not hit limit
		 if (attempts > 0){
			 cout << "LM: Open failed. Reboot attempt "<< attempts << " of "<< max_reboot_count << endl;	//Begin unmounting and restart process
			 delay(2000);  // wait two seconds for the flash drive
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
			 cout << "Now syncing" << endl; //write all buffered data
			 system("sync");
             delay(2000);
			 system("shutdown -r now");
			 } // reboot
		else if (attempts == 0){ //If the reboot counter has hit 2 and been reset
			cout << "LM: Open failed. Reboot attempt limit reached" << endl;
			delay(2000);
			break;
			}
		else{ //this should never occur
			cout << "LM: Open failed. Something has gone horribly wrong" << endl;
			delay(3000);
			break;
			}
      }

      else if (listfile.rdstate()== cout.goodbit) {
		 attempts = RebootCounter(0,max_reboot_count); //Successful attempt, reset counter
         cout << "LM: Open succeeded" << endl;
         break;
      }
}
   if (!listfile.rdstate()== cout.goodbit) exit(-10);  // failed to find list file

   cout << "LM: On disk drive " << disk_path << endl;
   while (getline (listfile,line) ) {
      cout << "LM: " << current_file_pointer+1 << "  " << line << endl;

      // If the list file was created on a DOS/Windows machine, it will have <CR><LF> \r\n at the
      // end of each line.  Unix expects only a <LF> \n.  The <CR> \r causes problems, so it must
      // be removed, if present.
      // remove all \r
      line=ListManager::remove_char(line,'\r');
      // remove leading and trailing spaces/tabs
      line=ListManager::trim(line);
      if (line.empty()) continue;  // Go to next line if this line is now empty

      // Look for comment mark at the beginning of the line.  (mark is usually '*')
      if (line.at(0)==LIST_FILE_COMMENT_MARK) {
         cout << "LM: comment ignored" << endl;
         line.clear();
      }
      if (line.empty()) continue;  // Go to next line if this line is now empty

      // Look for change of disk mark (usually '@')
      if (line.at(0)==EXTRA_VIDEO_DISK_MARK) {
         size_t d = line.find_last_of("0123456789");  // ends with a number
         string disk_name = line.substr(1,d);   // disk name
         f = input_list_filename.find_last_of("/\\");  // find last slash
         size_t pf = input_list_filename.find_last_of("/\\",f-1); // find previous slash
         disk_path = input_list_filename.substr(0,pf+1) + disk_name + "/";
         cout << "LM: Change to new disk: " << disk_path << endl;
         continue;
      }

      if (line.empty()) continue;  // Go to next line if this line is now empty

      // Look for evidence of a legitimate volume value at the beginning of the line.
      // Range is -6000 to 0.  Positive values have no effect.  They are tagged with a warning.
      if (line.find_first_not_of("+-0123456789") == 0) {
         // invalid volume entry
         cout << "LM: invalid volume entry, skipping line" << endl;
         continue;
      }

      // Read volume value.
      // Locate space or tab between volume and file name
      int first_sp = line.find_first_of(whitespace);
      string numstr=line.substr(0,first_sp);
      int volume;
      sscanf(numstr.c_str(),"%d",&volume);
      videos[current_file_pointer].volume=volume;
      if (volume > 0) cout << "LM: Warning: positive volume values are ignored by omxplayer." << endl;

      // remainder of line is the video file name,
      // with possible leading and trailing spaces.
      string fn=line.substr(first_sp,string::npos);
      fn=ListManager::trim(fn);
      if (fn.size() < 4) {   // too short to be a file name
         cout << "LM: invalid file name. Skipping." << endl;
         continue;
      }
      // If first character of file name is @ sign, that file will set to autorepeat (loop).
      if (fn.at(0) == LOOP_VIDEO_MARK) {
         cout << "LM: This file will be looped (replayed) automatically." << endl;
      }
      videos[current_file_pointer].dvd_filename = fn;
      videos[current_file_pointer].flash_drive_path = disk_path;
      current_file_pointer++;
   } // while getline

   listfile.close();
   cout << "LM: **** END OF LIST **********" << endl;
   // set up pointers
   last_file_pointer=current_file_pointer-1;
   current_file_pointer=0;
   cout << "\n   Full list  " << endl;
   for (int i=0; i<=last_file_pointer; i++) {
      cout << i << ": " << videos[i].volume << ">" << videos[i].flash_drive_path << videos[i].dvd_filename << "<" << endl;
   }
   cout << endl;

} // initialize()

videospec_t ListManager::currentVideo() {
   return (videos[current_file_pointer]);
}

videospec_t ListManager::nextVideo() {
   current_file_pointer++;
   if (current_file_pointer>=last_file_pointer) current_file_pointer=0;
   cout << "LM: pointer=" << current_file_pointer << " video=" << videos[current_file_pointer].dvd_filename << endl;
   return (videos[current_file_pointer]);
}  // nextVideo()

videospec_t ListManager::previousVideo() {
   current_file_pointer--;
   if (current_file_pointer < 0) current_file_pointer=last_file_pointer;
   cout << "LM: pointer=" << current_file_pointer << " video=" << videos[current_file_pointer].dvd_filename << endl;
   return (videos[current_file_pointer]);
} // previousVideo()

int ListManager::videoCount() {
   return last_file_pointer+1;
}

void ListManager::resetVideoPointer() {
   last_file_pointer=current_file_pointer-1;
   current_file_pointer=0;
}

// Count the number of reboots. Used when the list manager failes to
// find the list file.  Looks for file in /home/pi to count reboots
//
//  input    0  Initialize the counter
//           1  Check counter
//  returns
//     0   No more tries available, Quit
//    >0   More tries available, do a reboot
//    -1   Cannot open the file for counting Quit
//    -2   Counting file is corrupt Quit
//    -3   Error trying to update the count Quit
//
//
// If the file for counting is missing, try to create it and return
// error value -1
//
// Assumes we are using the namespace std::
int ListManager::RebootCounter(int action, int max_reboot_count)   {
	string counter_file_name = "/home/pi/rebootcount" ;
	string counterline;
	int current_count=0;

	if (action==0) {
		// Initialize counter
		ofstream counter_file(counter_file_name);
		if (counter_file.is_open()) {
			counter_file << "0";  // initialize a new counter file
			counter_file.close();
		}
		return 0;  //
	}

    // Try to open the counter file as an input stream
	ifstream counter_file(counter_file_name);
	if (!counter_file.is_open()) {   // problem opening file
		// try to initialize a new counter file
	    ofstream counter_file(counter_file_name);
		if (counter_file.is_open()) {
			counter_file << "0";  // initialize a new counter file
			counter_file.close();
		}
		return -1;  // we still return an error
	}
	// if we got here the input file is open
	getline(counter_file,counterline);  // read one line of the file
	counter_file.close();  // close
	try {
		current_count = stoi(counterline);
	}
	catch (...) {
		return -2; // the file did not contain a string with an integer
	}
	// we have the count as an integer
	current_count++; // increment the count
	if (current_count > max_reboot_count)
		current_count = 0;  // reset to zero, we reached our limit
	// Store the new count
	ofstream counter_file_out(counter_file_name);
	if (counter_file_out.is_open()) {
		counter_file_out << current_count;   // store the new string into a file
		counter_file_out.close();
	}
	else {  // if we cannot store the count, terminate
		return -3; // unable to store the new count
	}

	return current_count;
}

// Remove all occurrences of char ch from str
string ListManager::remove_char(string str, char ch ) {
   if (str.empty()) return str;
   size_t loc;
   loc=str.find(ch);
   while (loc != string::npos) {
      str.erase(loc,1);
      loc=str.find(ch);
   }
   return str;
}

// Trim white spaces from front and back of a string
string ListManager::trim(const string str) {
   if (str.empty()) return str;

   size_t strBegin = str.find_first_not_of(whitespace);
   if (strBegin == string::npos) return ""; // empty string
   size_t strEnd = str.find_last_not_of(whitespace);
   size_t number_of_characters = strEnd - strBegin + 1;
   string trimmed_str = str.substr(strBegin, number_of_characters);  // Trimmed string
   return trimmed_str;
}
