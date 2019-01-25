# ThreadWrangler

A crudely implemented tool for playing with thread affinity (not process affinity) for testing thread behaviour.

Needs to be ran as Administrator and probably should avoid using it against Games which have anticheat as how this would be seen is unknown, however there is no process executable modification or injection involved.

Project files are Visual Studio 2017, pre-compiled binaries are availible in x64 folder.


# Configuration

Add a process by clcking the gear, whilst a file selection window opens you only need to enter an executable name.


Disabling "Ideal Thread Mode" and "Affinity Mode" will effectively allow one to monitor only.

ITC = Ideal Thread Candidate

DTC = Dorment Thread Candidate

Excl = Exclude


It would be recommended to leave these enabled except for testing purposes as they are crudely implemented.

# Thread Allocation

User Time (Sticky) = Try to re-assign the same threads to the same affinities and groups.

User Time (Blind) = Blindly re-allocate the threads every second.

