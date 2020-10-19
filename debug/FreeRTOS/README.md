FreeRTOS-GDB
============

Python API Library for inspecting FreeRTOS Objects in GDB

Basically, the FreeRTOS internal state is kind of hard to inspect 
when working with GDB. This project provides some scripts for GDB's 
Python API that make accessing some of these internals a little easier
to inspect. 

##Requirements: 

1. You need to have the python API enabled in your version of GDB. This is a 
    compile time option when building GDB. You should be able to do something
	  like this: 
```
	gdb> python print "Hello World" 
```

and get predictable results. If it throws an error - then you don't have 
python compiled in your version of GDB.

2. Need to be using FreeRTOS 8.0+. This code could probably be used with FreeRTOS
    version 7.0 or previous versions, but the current code doesn't support it.

3. You need to use the Handle Registry for Queue info to be any use.
    Note that this only works for Queue based objects and not 
    for EventGroups 

4. You need to put the FreeRTOS-GDB/src directory on your python path: 
```
	$> export PYTHONPATH=~/src/FreeRTOS-GDB/src/
```

How To Use: 
```
$> gdb ./bin/program.elf 
(gdb) c 
Program runs on embedded device, sets up tasks, and queues 
<Break>
(gdb) source ~/FreeRTOS-GDB/src/FreeRTOS.py 
(gdb) show Task-List
            Name PRI STCK
Ready List {0}: Num Tasks: 1
            IDLE   0  107
Blocked List: Num Tasks: 11
       EMAC Task   1  239
      AFEC0 Task   1  295
     LDSENS Task   1  195
      AFEC1 Task   1  295
 LineSample Task   1  281
        DMAUART0   1  225
        Log Task   1  273
        BAT Task   1  169
         Rx Task   1  421
        Mng Task   2  551
       Cell Task   1  275
Delayed {1}: Num Tasks: 5
         Tmr Svc   3  355 62254
       WLAN Task   1  205 13817
       Init Task   1  445 10015
        LED Task   1  179  7105
         DMACOM1   1  265  7065
Delayed {2}: Num Tasks: 0

(gdb) show Queue-Info mutex
Num Queues: 6
            NAME  CNT             SEND          RECEIVE
        LOG:LOCK    1             NONE             NONE
     STREAM:LOCK    1             NONE             NONE
       TWI:MUTEX    1             NONE             NONE
     CC3000:LOCK    1             NONE             NONE
       WLAN:LOCK    0             NONE             NONE
        SPI:LOCK    1             NONE             NONE

(gdb) show Queue-Info queue
Num Queues: 14
            NAME  CNT             SEND          RECEIVE
            TmrQ    0                           Tmr Svc
     LOG:MSGPOOL   12             NONE             NONE
        LOG:MSGQ    0                          Log Task
       TWI:QUEUE    0             NONE             NONE
       SPI:QUEUE    0             NONE             NONE
    DMAAFEC:POOL    1             NONE             NONE
   DMAAFEC:QUEUE    0                        AFEC0 Task
    DMAAFEC:POOL    1             NONE             NONE
   DMAAFEC:QUEUE    0                        AFEC1 Task
      COM:TXPOOL    3             NONE             NONE
         COM:TXQ    0             NONE             NONE
      COM:RXPOOL    5             NONE             NONE
         COM:RXQ    0             NONE             NONE
     FATFS:MUTEX    0             NONE             NONE

```

@note - the NONE's above may just be empty strings.

This code adds the following custom GDB commands: 

* show List-Handle (symbol|address) [CastType]
	CastType is an optional argument that will cast all of the 
	handles in a list to a particular type. 
* show Task-List
* show Handle-Registry
* show Handle-Name  (symbole|address) 
* show Queue-Info [filter]
   filter can be "queue","mutex","semaphore", "counting", "recursive"



@TODO
=====

* With GDB's Python API - it doesn't seem to handle code is separate
    files very well. 

* Currently, the EventGroup objects don't have an inspector. 
    Work in progress - ideal solution would likely modify the struct
    of the Event Group to provide a similar piece of info that the 
    Queue handle does so that we could use the same registry.
