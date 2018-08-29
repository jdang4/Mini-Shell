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
    int end = convertToMilli(time2);
    // Wall Clock
    long int wallClock = end - start;

    // number of times process was preempted involuntarily
    long int preemptedInvol = Rusage.ru_nivcsw;

    // number of times the process gave up the CPU voluntarily
    long int waitResource = Rusage.ru_nvcsw;

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

// this removes the newline character so that the execvp can work properly
char* removeNewLine(char* input) {
  int strLength = strlen(input);

  if (strLength > 0 && input[strLength - 1] == '\n') {
    input[strLength - 1] = '\0';
  }

  return input;
}

// splits input by each command and put into argvNew
void parseInput(char *args[32], char input[128]) {
  char *token;
  int i;
  //const char* excludedChars;

  // if the background was requested
  /*
  if (bg == 1) {
    //token = strtok(input, " ");
    cout << "Background was requested" << endl;
    excludedChars = " &";
  }

  else {
    excludedChars = " ";
  }
  */
  token = strtok(input, " \n");
  while (token != NULL && i <=  32) {
    args[i] = token;
    //printf("%s ", token);
    args[i + 1] = NULL;
    token = strtok(NULL, " \n");
    i++;
  }

  //args[i] = NULL;
}

// checks to see if the user wanted to do a background
int backgroundChecker(char* input) {
  int background = 0;
  for (unsigned int i = 0; i < strlen(input); i++) {
    if (input[i] == '&') {
      background = 1;
    }
  }

  return background;
}

typedef struct {
  int pid;
  string command;
  long int startTime;
} processInfo;

int main(int argc, char *argv[]) {
  char *argvNew[32]; // array to hold 32 arguments
  char userInput[128]; // array to hold 128 characters
  // waits for user to type in commands (no arguments)
  if (argc == 1) {
    cout << "Welcome" << endl;
    vector<processInfo> background;
    vector<processInfo> backgroundTemp;
    pid_t val;
    while (1) {
      processInfo temp;
      for (unsigned long i = 0; i < background.size(); i++) {
        int status;
        //cout << "Pid: " << background.at(i).pid << endl;
        val = waitpid(-1, &status, WNOHANG);
        //cout << "Val: " << val << endl;
        //cout << "Status: " << status << endl;
        if (val == 0) {

        }

        if (val < 0) {

        }

        if (val > 0) {
          cout << "[" << i + 1 << "] " << background.at(i).pid << " " << background.at(i).command << "[FINISHED]" << endl;
          cout << background.at(i).command << " Stats" << endl;
          printStats(background.at(i).startTime);
          background.erase(background.begin() + i);
        }
      }

      //cout << "Please enter a command into the shell" << endl;
      cout << "==>";
      fgets(userInput, 128, stdin); // gets the user input
      //getline(cin, userInput);

      //removeNewLine(userInput); // this removes the new line character

      int bg = backgroundChecker(userInput);

      string tempCommand;

      if (bg == 1) {
        tempCommand = string(userInput);
        //tempCommand.pop_back();
        //cout << "Background was requested" << endl;
        std::string command(userInput);
        char *newUserInput = new char[128];
        strcpy(newUserInput, command.c_str());
        char *token = strtok(newUserInput, " &\n");
        int i = 0;
        while (token != 0) {
          argvNew[i] = token;
          //cout << token << endl;
          //cout << argvNew[i] << endl;
          argvNew[i + 1] = NULL;
          i++;
          token = strtok(NULL, " &\n");
        }

      }

      else {
        parseInput(argvNew, userInput);
      }


      //parseInput(argvNew, userInput);

      if (strcmp(userInput, "exit") == 0) {
        cout << "Exiting shell" << endl;
        exit(0);

      }

      if (strncmp(userInput, "cd", 2) == 0) {
        chdir(argvNew[1]);
        //cout << argvNew[1] << endl;
        continue;
      }

      if (strcmp(userInput, "jobs") == 0) {
        for (unsigned int i = 0; i < background.size(); i++) {
          cout << "[" << i + 1 << "] " << background.at(i).pid << " " << background.at(i).command << endl;

        }
        continue;
      }

      int pid;
      struct timeval time1; //start


      gettimeofday(&time1, NULL);
      int start = convertToMilli(time1);

      if ((pid = fork()) < 0) {
        cerr << "Fork Error!!!" << endl;;
        exit(1);
      }

      // in the child process
      else if (pid == 0) {
        //cout << "In Child Process" << endl;
        //cout << "In the child process" << endl;
        if (execvp(argvNew[0], argvNew) < 0) {
          cerr << "Execvp Error!!!" << endl;
          exit(1);
        }
      }

      // in the parent process
      else {
        // not background
        if (bg != 1) {
          // waits till the child process finishes
          //cout << pid << endl;
          int status1;
          waitpid(pid, &status1, 0);



          printStats(start);
        }

        // user requested command to be background
        else {
          //cout << "Background" << endl;
          int status;
          processInfo p = {pid, tempCommand, start};

          background.push_back(p);
          backgroundTemp.push_back(p);
          unsigned long index = background.size();
          cout << "[" << index << "] " << background.at(index - 1).pid << endl;
        }
      }


    }

  }

  return 0;

}
