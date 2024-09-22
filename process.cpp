#include "process.h"
#include <vector>

int IOevent::next_id = 1;//increment this in the constructor
int Process::next_id = 1;//increment this in the constructor

bool IOevent::canStart(int time){
  if(time >= startTime){
    return 1;
  }
  return 0;
}

bool IOevent::isDone(){
  if(resourceTime >= duration){
    finished = 1;
    return 1;
  }
  else{
    resourceTime++;
    if(resourceTime >= duration){
      finished = 1;
      return 1;
    }
    return 0;
  }
}

void IOevent::start(){
  started = 1;
  return;
}

IOevent::IOevent(int start, int dur, int resID){
  startTime = start;
  duration = dur;
  resource = resID;
  id = next_id;
  next_id++;
  resourceTime = 0;
  started = 0;
  finished = 0;
}

Process::Process(int start, int dur, int MemorySize, vector<IOevent> requests){
  startTime = start;
  duration = dur;
  size = MemorySize;
  events = requests;
  id = next_id;
  next_id++;
  CPUtime = 0;
  state = newArrival;
  startAlloc = -1;//not allocated at first
}

bool Process::canStart(int time){
  if(time >= startTime){
    return 1;
  }
  return 0;
}

bool Process::checkRequests(){
  for(auto e : events){
    if((!e.started) && e.canStart(CPUtime)){
      return 1;
    }
  }

  //no unserviced requests at this time
  return 0;
}

bool Process::checkInterrupts(){
  
  for(auto& e : events){
    if(e.started && (!e.finished)){
      bool done = e.isDone();
      if(done){
        return 1;
      }
    }
  }

  //all started IO events are still going
  return 0;
}

void Process::timeAction(){
  CPUtime++;
}

bool Process::processFinished(){
  if(CPUtime >= duration){
    return 1;
  }

  return 0;
}


void Process::startEvent(int id){
  for(auto& e: events){
    if (e.id == id){
      e.start();
      return;
    }
  }
}