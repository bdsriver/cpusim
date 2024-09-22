# cpusim
Simulates a an operating system with tasks, processes, memory and resources to manage. See [flowchart](flowchart.pdf)




Every process can have one of five states: processing, blocked, ready, newArrival, or done. All processes have a set size that they take up in memory. A set amount of consecutive memory is reserved for any process that is running or blocked. When assigning memory for a process, a best-fit algorithm is used to find the spot for it. If there is no spot in memory with a big enough hole to fit the process, memory is compacted until there is a big enough hole.

In this system, multiple processes can be running at once. If a process makes an I/O request and the resource it is requesting is free, the process can continue running while the I/O event is happening. If the resource is not free, the entire process is blocked. While blocked, other I/O events of the blocked process can continue. When the request is blocked, we test the request to see if it leads to deadlock. We do this with a wait-for graph, meaning we see what processes are waiting for other processes to finish, and if processes are waiting for each other in a circle, the request cannot be allowed (yet). If a request does not lead to deadlock, it is put in line for the resource. When a blocked process finishes all concurrent I/O events, it is added back to the ready list and removed from memory.

During each time interval, any newArrival processes that can start are admitted to the ready list, and one process in the ready list begins processing. Because multiple processes are running at once, each process has multiple actions to run through in a time step. If a process receives an interrupt from an I/O event being finished, that resource is deallocated and the process can continue running if it was previously running or is added to the ready list if it was blocked. Each then checks if it has any I/O requests. If it does, actions will be taken as described in the previous paragraph. If the process is still running after that, it increments the time it has spent on the CPU. When it has spent its requested amount of time on the CPU and all of its events have been completed, the process is done.

Implementation:

The output is a representation of the state of the computer at every time interval, in the terminal ([or it can be piped to a file](output.txt)). To use it, the user can either create a custom object using the object constructors or create a file to be read and input to a CPU object. The user will need to provide the size of the memory when calling the CPU object constructor. The required input for a CPU object is the memory size and a vector of processes. All processes should have a start time, duration (on the processor), size  (that it takes in memory), and a vector of I/O events (this can be empty), in that order. The I/O events should have the time they start (relative to the time the process starts), the duration, and the ID of the resource they will request, in that order. A list of resources does not need to be supplied, the CPU will look at all the IDs requested in events.

To create an object with a file, the user can copy the code in my main function and edit the input file to numbers of their choosing. If they use this method, the format should be (as seen in my input.txt file):

<process start time> <process duration> <process size>

Followed by 0 or more I/O events:

<event start time> <event duration> <event resource ID>

Followed by an ‘x’ to denote the end of every line [see example](output.txt).

If a user wants to create their own object and test the deadlock or memory functions, they can simply create the object and set their preferred values.

To run the CPU, all the user needs to do is continually call the timeStep() function. This will output the state of the machine at its current time and increment the time so it can be used again. The function will return a false boolean value when all processes are done, so it can be used in a while loop, such as:            while(cpuName.timeStep());

The output will have the time (starting at 1), memory size, the list of resources and where they are allocated, and the list of processes. The list of processes will have the current state of each process along with where it is allocated in memory and its list of events and their status.
