/**
  * Author: Jonathan Dang
  * WPI Username: jdang
  * Course: CS 3013
  *
  * This program basically simulates a Linux shell. The shell created in this program is able to handle common Linux commands and also is able to put
  * processes into the background. Even though this program is able to simulate a regular Linux shell, it can only simulate it to some degree. In the
  * process of writing this program, I have encountered some limitations that my shell had. From what I have noticed was that in my shell, it cannot handle
  * the pipe "\" command, the "grep" command, when entering "cd" alone it does not go back to my home directory, it does not allow the user to go thorugh
  * the previous entered commands like how a regular Linux shell can by clicking the up arrow, it can only take up to 32 arguments or 128 characters, and
  * it is probably more error prone than a regular Linux shell. All the limitations that I have listed, they are not all of them, but they are the ones that
  * I have noticed or thought of.

**/

#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;

// converts the timeval to be in milliseconds
long int convertToMilli(struct timeval t) {

  return (t.tv_sec * 1000) + (t.tv_usec / 1000);
}

// this prints out the stats
void printStats(long int start) {
    struct rusage Rusage;

    getrusage(RUSAGE_CHILDREN, &Rusage);

    // CPU Time
    long int userTime = convertToMilli(Rusage.ru_utime);
    long int systemTime = convertToMilli(Rusage.ru_stime);

    struct timeval time2; //end
    gettimeofday(&time2, NULL);
    long end = convertToMilli(time2);

    // Wall Clock
    long int wallClock = end - start;

    // number of times process was preempted involuntarily
    long int preemptedInvol = Rusage.ru_nivcsw;

    // number of times the process gave up the CPU voluntarily
    long waitResource = Rusage.ru_nvcsw;

    // number of major page faults
    long int majFault = Rusage.ru_majflt;

    // number of minor page faults
    long int minFault = Rusage.ru_minflt;

    // max resident set size
    long int maxRes = Rusage.ru_maxrss;

    cout << endl << "User CPU time used: " << userTime << " milliseconds" << endl;
    cout << "System CPU time used: " << systemTime << " milliseconds" << endl;
    cout << "Wall Clock Time: " << wallClock << " milliseconds" << endl;
    cout << "Number of times the process was preempted involuntarily: " << preemptedInvol << endl;
    cout << "Number of times the process gave up the CPU voluntarily: " << waitResource << endl;
    cout << "Number of major page faults: " << majFault << endl;
    cout << "Number of minor page faults: " << minFault << endl;
    cout << "Number of maximum resident set size used: " << maxRes << endl << endl;

}

// this function is in charge of removing the newline character
char* removeNewLine(char* input) {
  int strLength = strlen(input);

  if (strLength > 0 && input[strLength - 1] == '\n') {
    input[strLength - 1] = '\0';
  }

  return input;
}

// used to retain the process info of the process that was sent into the background
typedef struct {
  int pid; // the process info of the process
  string command; // the command of the process
  string first_command; // stores the first command (printed when bg process finishes)
  long startTime; // when the process started
} processInfo;

////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  char *argvNew[32]; // array to hold 32 arguments
  char userInput[128]; // array to hold 128 characters

  // user typed in arguments
  if (argc > 1) {
    int pid; // the process id of the process
    int hasCD = 0; // will determine if user typed in the "cd" command (0 = no "cd", 1 = yes "cd")

    struct timeval time1;
    gettimeofday(&time1, NULL);
    long int start = convertToMilli(time1); // the start time

    // handles a fork error
    if ((pid = fork()) < 0) {
      cerr << "Fork Error!!!" << endl;
    }

    else if (pid == 0) {
      // in the child process

      // goes through the commands and parses them into argvNew
      // this for loop starts at 1 because don't want to include doit
      for (int i = 1; i < argc; i++) {

        argvNew[i - 1] = argv[i]; // adds the parsed input into argvNew

        // if a "cd" was detected in the user input, set hasCD to 1
        if (strcmp(argvNew[i - 1], "cd") == 0) {
          hasCD = 1;
        }

      }
      argvNew[argc - 1] = NULL; // adds the null terminator at the end of array

      // if the user typed in the "cd" command
      if (hasCD != 0) {
        if (strcmp(argvNew[1], "&") == 0) {
          // do nothing
        }

        // handles "cd" command
        else {
          chdir(argvNew[1]);
        }

      }

      // use execvp to run the command
      else {
        if (execvp(argvNew[0], argvNew) < 0) {
          cerr << "Execvp Error!!!" << endl;
          exit(1);
        }
      }
    }

    else {
      // in the parent process
      wait(0);
      printStats(start);
    }
  }

  // waits for user to type in commands (no arguments)
  else if (argc == 1) {
    cout << "Welcome to Shell" << endl;

    vector<processInfo> background; // will store all the processes that were put into the background

    string prompt = "==>";

    while (1) {
      // checks to see if there any of the child processes in the background have changed its process state
      for (unsigned long i = 0; i < background.size(); i++) {
        int status;
        pid_t val = waitpid(background.at(i).pid, &status, WNOHANG);

        // the child process in the background has not changed state (still running)
        if (val == 0) {

        }

        // there was an error detected
        else if (val < 0) {
          //There was an error
        }

        // the child process that was in the background had changed its state
        else if (val > 0) {
          cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
          cout << "[" << i + 1 << "] " << background.at(i).pid << " " << background.at(i).first_command << " Completed" << endl;
          cout << endl << background.at(i).command << " Stats";
          printStats(background.at(i).startTime);
          cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
          background.erase(background.begin() + i);
        }

      }

      cout << prompt;

      fgets(userInput, 128, stdin); // gets the user input, limits user to 128 characters

      // if an end-of-file was detected
      if (feof(stdin)) {
        cout << "Detected EOF character. Terminating doit program" << endl;

        if (background.size() > 0) {
          // checks for any unfinished background tasks
          for (unsigned int i = 0; i < background.size(); i++) {
            int status;
            pid_t val = waitpid(background.at(i).pid, &status, WNOHANG);

            if (val == 0) {
              //background is running
            }

            else if (val < 0) {

            }

            else {
                background.erase(background.begin() + i);
            }
          }
        }

        exit(0);

      }

      removeNewLine(userInput); // removes the newline character from the user input
      string command = string(userInput); //converts the user input into a string

      //turn the user inputed command into a stringstream in order to parse through it
      //while using getline
      istringstream input(command);

      // indicates if a background was detected (0 = user doesn't want command in background, 1 = user does want command in background)
      int bg = 0;

      string line; // the string that getline is placing the string it got before it saw delimitor
      int i = 0; // keeps track of the index position in the argvNew array

      //this is used to check for the "&" character and parsing each command into argvNew
      while (getline(input, line, ' ')) {
        //cout << line.c_str() << endl;
        char* a_command = strdup(line.c_str()); //creating pointer to the new string (line)

        // if it detects that the user wants to put in background
        if (strcmp(a_command, "&") == 0) {
          bg = 1;
        }

        // if it did not see a "&" character in the user input
        else {
          argvNew[i] = a_command; // adds the parsed command into argvNew
          i++;
        }

      }

      string firstCommand = argvNew[0]; // stores the first command

      argvNew[i] = NULL; // adds the null character into the char array

      // special case when user wants to exit the program
      // it must make sure that all the background processes have been finished before exiting
      if (strcmp(argvNew[0], "exit") == 0) {
        if (background.size() > 0) {
          cout << "Must wait for the background tasks to finish" << endl;

          // checks for any unfinished background tasks
          for (unsigned int i = 0; i < background.size(); i++) {
            int status;
            pid_t val = waitpid(background.at(i).pid, &status, 0);

            if (val == 0) {
              //background is running
            }

            else if (val < 0) {

            }

            else {
                cout << "[" << i + 1 << "] " << background.at(i).pid << " " << background.at(i).first_command << " Completed" << endl;
                printStats(background.at(i).startTime);
                background.erase(background.begin() + i);
            }
          }
          cout << "All the background tasks have been finished" << endl;
        }

        cout << "Exiting Shell" << endl;
        exit(0);
      }

      // handles case if user inputed cd
      else if (strcmp(argvNew[0], "cd") == 0) {
        chdir(argvNew[1]);
        continue;

      }

      // if the user wants to change the prompt
      else if ( (strcmp(argvNew[0], "set") == 0)
              && (strcmp(argvNew[1], "prompt") == 0)
              && (strcmp(argvNew[2], "=") == 0) ) {

                prompt.assign(argvNew[3]); // the new prompt
                continue;
      }

      // lists through all the processes in the background
      else if (strcmp(argvNew[0], "jobs") == 0) {
        for (unsigned int i = 0; i < background.size(); i++) {
          cout << "[" << i + 1 << "] " << background.at(i).pid << " " << background.at(i).command << endl;

        }
        cout << endl;
        continue;
      }

      // if the command isn't a special case scenario
      else {
        int pid; // the process id of the process
        struct timeval time1; //start

        gettimeofday(&time1, NULL);
        long int start = convertToMilli(time1);

        if ((pid = fork()) < 0) {
          cerr << "Fork Error!!!" << endl;;
          exit(1);
        }

        else if (pid == 0) {
          // inside the child process
          if (execvp(argvNew[0], argvNew) < 0) {
            cerr << "Execvp Error!!!" << endl;
            exit(1);
          }
        }

        else {
          // inside the parent process

          // if the user did not want the command to be in the background
          if (bg != 1) {
            int status;

            // I used this version of the wait() command to avoid the system
            // of getting confused on which process the parent should wait for, which was
            // either the process in the background or the current process
            waitpid(pid, &status, 0); // waits for the pid returned from the child process

            printStats(start); // this prints out the process statistics
          }

          // if the user wanted the command to be in the background
          else {

            // create a struct that will retain the information of the process that will be
            // going into the background
            processInfo p = {pid, command, firstCommand, start};
            background.push_back(p);

            unsigned long index = background.size();

            // this prints out a statement that has the same format as a real shell in
            // the event of the process being in the background
            cout << "[" << index << "] " << background.at(index - 1).pid << " " << background.at(index - 1). command << " "<< endl;
          }
        }
      }
    }

  }

  return 0;

}
