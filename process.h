#ifndef PROCESS_H
#define PROCESS_H

#include<vector>

using namespace std;

class IOevent{
  public:
    IOevent(int start, int dur, int resID);

    //this is called every time step if the process is not running
    bool canStart(int time);

    //checks if the process is done. if not, resourceTime is incremented
    // set finished to true if true
    bool isDone();

    void start();//change started to 1

    int id;
    bool started;// == true when the IO event has started
    bool finished;// for can be called without incrementing, unlike isDone()
    int resourceTime;// == duration when process is done
    int resource;

  private:
    static int next_id;
    int startTime;//this is relative to the time its process starts
    int duration;
};
enum State { ready, processing, blocked, newArrival, done};

//the process is blocked unless all IO events that start at the current time or before are completed
//but the process can still request another resource if blocked
class Process{
  public:
    Process(int start, int dur, int MemorySize, vector<IOevent> requests);

    //returns true if a process can start
    bool canStart(int time);

    //return true if there is a request at this time
    bool checkRequests();

    //returns true if it is done with a resource
    bool checkInterrupts();

    //increment CPU time
    //the CPU class controls resources and requests, do not do management here
    void timeAction();

    //returns true if the process is completely finished
    bool processFinished();

    //start the event with int id
    void startEvent(int id);

    State state;
    int id;
    int size;//how much memory space it takes
    int startAlloc; //the beginning of where it is allocated in memory
    vector<IOevent> events;
    int CPUtime; //initialize to 0
    int duration;

  private:
    static int next_id;
    int startTime;
};







#endif