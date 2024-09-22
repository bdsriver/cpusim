#include "process.h"
#include "cpu.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

CPU::CPU(int memoryAmount, vector<Process> processList){
  memorySize = memoryAmount;
  processes = processList;
  time = 0;
  makeResourceList();

  //create memory here
  for(int i=0; i<memorySize; i++){
    memory.push_back(0);//all memory is unallocated at start
  }
}

int CPU::findMemorySpace(Process p){
  int minHoleSize = p.size;
  //First record hole information

  //record where each hole in memory starts and how big it is
  vector<int> holeSizes;
  vector<int> holeStartLocations;
  int currHoleSize = 0;
  bool lastValue = 1;//when this changes we know a hole begins/ends

  for(int i=0; i<memorySize; i++){
    if(memory[i] == 0){
      currHoleSize++;//this means hole is still going
    }

    if(memory[i] != lastValue){//hole starts or ends here
      if(memory[i] == 0){//hole begins at index i
        currHoleSize = 1;
        holeStartLocations.push_back(i);//note this is where a hole starts
      }

      if(memory[i] == 1){//hole ends at index i
        holeSizes.push_back(currHoleSize);
      }
    }

    lastValue = memory[i];
  }

  //if it ends on a hole
  if(memory[memorySize - 1] == 0){
    holeSizes.push_back(currHoleSize);//record the last hole
  }

  int bestStartLocation = -1;//we will return -1 if there is no fit
  int bestFitMargin = memorySize + 1;
  int fitMargin;
  //find the best fit
  for(int i=0; i<int(holeStartLocations.size()); i++){
    if(holeSizes[i] >= minHoleSize){//the process fits in this hole
      fitMargin = holeSizes[i] - minHoleSize;
      if(fitMargin < bestFitMargin){//this hole is the best fit
        bestFitMargin = fitMargin;
        bestStartLocation = holeStartLocations[i];
      }
    }
  }

  return bestStartLocation;
}

void CPU::makeResourceList(){
  //add every resource to resource list based on id's in processes
  for(auto p: processes){
    for(auto e: p.events){
      int r_id = e.resource;
      //find if this resource has already been added to the list of resources
      bool rAlreadyAdded = 0;
      for(auto r: resources){
        if(r.id == r_id){
          rAlreadyAdded = 1;
          break;
        }
      }
      if(! rAlreadyAdded){//add the resource to resource list
        Resource res;
        res.id = r_id;
        resources.push_back(res);
      }

    }
  }

  return;
}

void CPU::handleInterrupts(Process & p){
  for(auto e : p.events){
    if(e.finished && (e.resource != -2)){
      int r_id = e.resource;
      //deallocate the event from the resource

      //find the resource
      for(auto & r: resources){
        if(r.id == r_id){
          r.eventID = -1;//there should be no event with ID of -1
          e.resource = -2;
          break;
        }
      }
    }
  }
}

int CPU::parentProcess(IOevent event){
  for(auto p: processes){
    for(auto e: p.events){
      if(e.id == event.id){
        return p.id;
      }
    }
  }
  throw runtime_error("could not find event");
}

bool CPU::deadlockDetection(IOevent event){
  //is the resource already allocated?
  bool alreadyAllocated = 0;
  //find the resource
  for(auto r: resources){
    if(r.id == event.resource){//the eventID will be -1 if the resource is not allocated
      alreadyAllocated = ((r.eventID == -1) ? 0 : 1);
      if(r.eventID == r.id){//for when the eventID is r.id
        alreadyAllocated = 0;
      }
      break;
    }
  }

  if(alreadyAllocated){//deadlock detection only if the request would be blocked
    return (waitForGraph(event));
  }
  //if it was not blocked then deadlock cannot happen due to this request
  return false;
}

bool CPU::waitForGraph(IOevent event){
  //each element of the graph variable represents the process of id i
  vector<vector<int>> graph;//each element is an array of the processes it waits for

  for(auto p: processes){
    vector<int> a;
    graph.push_back(a);
  }
  vector<int> a;//to have n+1 elements in the graph and make indexing easier
  graph.push_back(a);

  //add "arrows" to graph
  for(auto p : processes){
    if(p.state == blocked){//this means it is waiting for a resource
      for(auto e: p.events){
        //if event is started, it is either running or waiting
        //will not add itself to the wait-for list
        if((e.started && (parentProcess(e) != p.id))){
          graph[p.id].push_back(parentProcess(e));
        }
      }
    }
  }

  //find the parent of the event
  int eventParent = -1;
  for(auto p: processes){
    for(auto e: p.events){
      if (e.id == event.id){
        eventParent = p.id;
        break;
      }
    }
    //break after finding the parent
    if(eventParent != -1){
      break;
    }
  }
  //add the new event to the graph
  graph[eventParent].push_back(parentProcess(event));

  //look for cycles in the graph
  for(int i=0; i<int(graph.size()); i++){
    for(auto n: graph[i]){
      loopTimes = 0;
      bool loopFound = recursiveLoopFind(graph, n, (i));
      if(loopFound){
        return 1;
      }
    }
  }

  //no loop was found at this point
  return 0;

}

int CPU::loopTimes = 0;

bool CPU::recursiveLoopFind(vector<vector<int>> g, int start, int base){
  //static variable. if it becomes more than the amount of processes then there is a cycle
  loopTimes++;
  //base case
  if(start == base){
    return true;
  }
  if(g[start].empty()){//this means the process does not wait on any others
    return false;
  }
  if(loopTimes > int(g.size())){
    return true;//too many iterations. a loop must exist
  }

  for(auto n: g[start]){
    if(recursiveLoopFind(g, n, base)){
      return true;
    }
  }

  return false;//if no loop was ever found
}

void CPU::printCPU(){
  cout << "time: " << time << "\tmemory size: " << memorySize << endl;
  
  
  cout << "Resources:" << endl;
  for(auto r: resources){
    cout << 'r' << r.id << ":\tservicing ";
    if(r.eventID == -1){
      cout << "none" << endl << endl;
    }
    else{
      //output the request using the resource
      cout << "request " << r.eventID << endl << endl;
    }
  }

  cout << "Processes:" << endl << endl;
  for(auto p: processes){
    cout << 'p' << p.id << ":\tstate: ";
    switch(p.state){
      case newArrival: 
        cout << "newArrival";
        break;
      case ready:
        cout << "ready";
        break;
      case processing:
        cout << "processing";
        break;
      case blocked:
        cout << "blocked";
        break;
      case done:
        cout << "done";
        break;
    }
    cout << endl;
    //memory section
    if((p.state == processing) || (p.state == blocked)){
      cout << "in memory from " << p.startAlloc << " to " << (p.startAlloc + p.size) << endl;
    }
    cout << "requests:" << endl;
    for(auto e: p.events){
      cout << e.id;
      if(e.started){
        if(e.finished){
          cout << ": finished";
        }
        else{
          cout << ": in progress";
        }
      }
      else{
        if(e.canStart(p.CPUtime - 1)){
          cout << ": blocked";
        }
        else{
          cout << ": not yet occurred";
        }
      }
      cout << endl;
    }
    cout << endl;
  }
}

bool CPU::addProcess(Process & p){
  int mStart = findMemorySpace(p);
  if(mStart == -1){
    return false;
  }
  //else, there was a valid memeory space found
  p.startAlloc = mStart;
  for(int i=0; i<p.size; i++){
    memory[p.startAlloc + i] = 1;//"allocate" the memory
  }
  return true;
}

void CPU::removeProcess(Process & p){
  for(int i=0; i<p.size; i++){
    memory[p.startAlloc + i] = 0;//"deallocate" the memory
  }
  p.startAlloc = -1;
  return;
}

void CPU::compactMemory(int minSize){
  int holeStartLoc = 0;
  int currHoleSize = 0;
  do{
    //find where the hole is
    for(int i=holeStartLoc; i<memorySize; i++){
      if(memory[i] == 0){
        holeStartLoc = i;
        break;
      }
    }

    //find the where the hole ends
    int nextStartLoc = 0;
    for(int i=holeStartLoc; i<memorySize; i++){
      if(memory[i] == 1){
        nextStartLoc = i;
        break;
      }
    }

    //move the process from the end of the hole to the beginning, creating a bigger hole
    for(auto & p: processes){
      if(nextStartLoc == p.startAlloc){
        removeProcess(p);
        p.startAlloc = holeStartLoc;
        for(int i=0; i<p.size; i++){
          memory[p.startAlloc + i] = 1;
        }

        //find the size of the new hole created
        currHoleSize = 0;
        for(int i = (p.startAlloc +p.size); i<memorySize; i++){
          currHoleSize++;
          if(memory[i] == 1){
            currHoleSize--;
            break;
          }
        }
        break;
      }
    }
    holeStartLoc = nextStartLoc;
  //repeat until there is a big enough hole
  }while(currHoleSize<minSize);
  return;
}

bool CPU::timeStep(){
  time++;
  //All processes added and run to completion?
  int runningProcessAmount = int(processes.size());
  for(auto p: processes){
    if(p.state == done){
      runningProcessAmount--;
    }
  }
  if(runningProcessAmount == 0){
    return false;//all processes are done
  }

  //Are there any newArrival processes?
  for(auto & p: processes){
    if((p.state == newArrival) && (p.canStart(time))){
      //add it to the ready list
      p.state = ready;
    }
  }

  //Does the ready list have any processes?
  for(auto & p: processes){
    if(p.state == ready){
      //is there a space in memory that can fit the process?
      bool processesAdded = addProcess(p);//add the process if there is space
      p.state = processing;
      if(! processesAdded){
        //compact memory until it can be added
        compactMemory(p.size);
        //add the process
        addProcess(p);
      }
      break;//only add one
    }
  }

  //end time step if no processes are running or blocked
  bool NoProcessRunningOrBlocked = 1;
  for(auto p: processes){
    if((p.state == blocked) || (p.state == processing)){
      NoProcessRunningOrBlocked = 0;
      break;
    }
  }

  if(NoProcessRunningOrBlocked){
    printCPU();
    return true;
  }

  //For each process
  for(auto & p: processes){
    if((p.state != processing) && (p.state != blocked)){
      continue;
    }
    //is the process blocked?
    //check for interrupts if process is running
    bool interrupts;
    if((p.state == processing)){
      interrupts = p.checkInterrupts();
      if(interrupts){
        handleInterrupts(p);
      }
    }

    bool processBlocked = (p.state == blocked ? 1:0);
    bool startedOperationsCompleted = 1;
    if(processBlocked){

      //Are all started I/O operations completed?
      for(auto & e: p.events){
        if(e.started){
          if(!e.finished){
            startedOperationsCompleted =0;
          }
        }
      }

      //Are there pending interrupts?
      if(!startedOperationsCompleted){
        interrupts = p.checkInterrupts();
        if(interrupts){
          handleInterrupts(p);//handle all interrupts of the process
        }
        
      }    
    }


    //check for requests
    bool requests = p.checkRequests();
    if( (!requests) && (startedOperationsCompleted) && (p.state != processing)){//append the process to ready list
      p.state = ready;
      removeProcess(p);
      continue;
    }
    else{
      for(auto e: p.events){
        if(e.finished){
          continue;
        }
        if(!e.canStart(p.CPUtime)){
          continue;
        }
        if(e.started){
          continue;
        }
        //is the resource avaiable and does the request lead to deadlock?
        if((!deadlockDetection(e)) && (e.canStart(p.CPUtime))){
          for(auto & r: resources){
            if(r.id == e.resource){
              //assign the process to the resource 
              r.eventID = e.id;
              p.startEvent(e.id);
              break;
            }
          }
        }
        // block the process
        else if(deadlockDetection(e) && (e.canStart(p.CPUtime))){
          p.state = blocked;
        }
      }

    }

    //is the process blocked?
    if(p.state != blocked){
      //increment CPUtime
      p.state = processing;
      p.CPUtime++;
    }
    bool AllRequestsFinished = 1;
    for(auto e: p.events){
      if(!e.finished){
        AllRequestsFinished = 0;
        break;
      }
    }
    if(AllRequestsFinished && (p.CPUtime >= p.duration)){
      p.state = done;
      removeProcess(p);
    }
  }
  printCPU();
  return true;
}