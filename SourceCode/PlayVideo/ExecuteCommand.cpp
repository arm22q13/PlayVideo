// ExecuteCommand.cpp   Run system command and get result

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>

using namespace std;

#ifndef _EXECUTE_COMMAND_OBJECT
#define _EXECUTE_COMMAND_OBJECT

class ExecuteCommand {
   public:
      string execute(const char* SystemCmd) {
        char buffer[128];
        string result = "";
        FILE* pipe = popen(SystemCmd, "r");

        if (!pipe) throw runtime_error("popen() failed!");
        try {
           while (!feof(pipe)) {
              if (fgets(buffer, 128, pipe) != NULL) result += buffer;
           } // while
        } // try 
        catch (...) {
           pclose(pipe);
           throw;
        } // catch
        pclose(pipe);
        return result;
      } // end execute
}; // ExecuteCommand


#endif