#include "process.h"
#include "cpu.h"
#include <vector>
#include <fstream>
#include <string>

using namespace std;

int main(){
  ifstream inputFile("input.txt");

  string inputNum;
  char inputChar;
  vector<Process> processes;
  //turning file input into cpu object
  while(getline(inputFile, inputNum, ' ')){
    int pStart = stoi(inputNum);
    getline(inputFile, inputNum, ' ');
    int pDur = stoi(inputNum);
    getline(inputFile, inputNum, ' ');
    int pSize = stoi(inputNum);

    vector<IOevent> events;

    inputFile.get(inputChar);
    while(inputChar != 'x'){
      inputFile.unget();
      getline(inputFile, inputNum, ' ');
      int IOstart = stoi(inputNum);
      getline(inputFile, inputNum, ' ');
      int IOdur = stoi(inputNum);
      getline(inputFile, inputNum, ' ');
      int IOres = stoi(inputNum);

      IOevent e = IOevent(IOstart, IOdur, IOres);
      events.push_back(e);

      inputFile.get(inputChar);
    }
    inputFile.get(inputChar);
    Process p = Process(pStart, pDur, pSize, events);
    processes.push_back(p);
  }

  CPU cpu = CPU(40, processes);
  inputFile.close();

  //loops until all processes are done
  while(cpu.timeStep());

  return 0;
}

