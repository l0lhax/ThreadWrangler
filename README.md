# ThreadTest

A crudely implemented tool for playing with thread affinity (not process affinity) for testing thread behaviour, mostly with respect to AMD Ryzen Windows Schedular 'scandal' and it's behaviour with respect to threads being randomly scheduled across all CCX's.

Needs to be ran as Administrator and probably should avoid using it against Games which have anticheat as how this would be seen is unknown, however there is no process executable modification or injection involved.

Project files are Visual Studio 2015, pre-compiled binaries are availible in Win32 and x64 folders.

```text
Thread Affinity Thingi

Sorting: (optional)
   -sortbythreadid          Sort threads by Thread ID.
   -sortbywalltime          Sort threads by thread wall time.

Output: (optional)
   -showdormant             Include threads that are dormant in output.
   -showsleeping            Include threads that are sleeping in output.
   -showall                 Include all threads in output.

Behaviour: (optional)
   -switchevery <value>     Move domain every <value> threads.
   -domains <value>         Number of CPU domains to emulate.
   -realcoresonly           Allocate to only every second logical core.
   -cores                   Override the number of cores detected, must be even.
   -lockdeadthreadsto       Lock threads that are dormant to a specific logical core, 0-based.

Target: (required)
   -process <value>         Name of the running executable file to target, ie. "calc.exe"
```

Examples:
- Emulate XXXX | XXXX (2 domains) on a 8 core processor, lock every other thread to opposite domain.
```text
ThreadTest.exe -process "DOOMx64vk.exe" -domains 2 -cores 8 -showsleeping
```

- Emulate XXXXXXXX | XXXXXXXX (2 domains) on a 16 core processor, lock every other thread to opposite domain.
```text
ThreadTest.exe -process "DOOMx64vk.exe" -domains 2 -cores 16 -realcoresonly -sortbywalltime -showsleeping
```
