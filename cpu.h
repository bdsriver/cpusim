#ifndef CPU_H
#define CPU_H


#include "process.h"
#include <vector>

using namespace std;

//The system will create a list of resources by looking at all 
//the resources requested by processes
struct Resource{
  int id;
  int eventID = -1;//id of the event it is handling
};

class CPU{
  public:
    CPU(int memoryAmount, vector<Process> processList);

    //return the appropriate starting memeory space for a process
    //return -1 if there is no space
    //return the starting position such that the start is at
    //the start of the best fit space
    int findMemorySpace(Process p);

    //find what resources will be used from scanning the IO events
    void makeResourceList();

    //free the resource for all interrupts
    //this should call checkInterrupts() and increment resource time
    void handleInterrupts(Process & p);

    //return the process that originally sent out a request
    int parentProcess(IOevent event);

    //check if a request leads to deadlock, return true if it does
    bool deadlockDetection(IOevent event);

    //called by deadlockDetection, returns true if the wait-for graph determines there can be deadlock
    bool waitForGraph(IOevent event);

    //called by waitForGraph to look through the graph and find a loop
    //return true if a loop is found
    bool recursiveLoopFind(vector<vector<int>> g, int start, int base);

    //call at the end of every time step
    void printCPU();

    //Do everything necessary for a time step
    //Return false if all processes are done
    bool timeStep();

    //Add the process to memory
    //compact memory outside of this function
    // return false if it cannot be added
    bool addProcess(Process & p);

    //Remove the process from memory
    //this will be used with compaction and process finishing procedure
    void removeProcess(Process & p);

    //minSize is the minimum size hole that needs to exist to allocate a process
    //we will stop compacting when the minsize requirement is met
    void compactMemory(int minSize);

  private:
    static int loopTimes;//set it to 0 before calling recursiveLoopFind
    int time;
    int memorySize;
    vector<Process> processes;
    vector<Resource> resources;
    vector<bool> memory; //each element value 1 or 0 if the memory is allocated or not
};







#endif