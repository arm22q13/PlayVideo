// ListManager.cpp

#include "ListManager.h"

// implementation of class ListManager
//
void ListManager::initialize(string input_list_filename) {
   list_filename = input_list_filename;  // save the list file path
   size_t f = input_list_filename.find_last_of("/\\");
   string disk_path = input_list_filename.substr(0,f+1);  // keep slash at the end

   string line;

   current_file_pointer=0;
   ifstream listfile;
   for (int i=0; i<6; i++) {  // try 6 times to open the list file
      cout << "LM: Opening list file at:" << list_filename << endl;
      listfile.open(list_filename.c_str(), std::ifstream::in);
      if (!listfile.rdstate()== cout.goodbit) {
         cout << "LM: Open failed" << endl;
         delay(1000);  // wait one second for the flash drive
      }
      else {
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
   if (current_file_pointer > last_file_pointer) current_file_pointer=0;
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
