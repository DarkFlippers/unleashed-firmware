# File: FreeRTOS.py
# Author: Carl Allendorph
# Date: 05NOV2014
#
# Description:
#   This file contains some python code that utilizes the GDB API
# to inspect information about the FreeRTOS internal state. The
# idea is to provide the user with the ability to inspect information
# about the tasks, queues, mutexs, etc.
#

from os import path
import sys

directory, file = path.split(__file__)
directory = path.expanduser(directory)
directory = path.abspath(directory)

sys.path.append(directory)

import gdb
import pprint

from FreeRTOSgdb.Types import StdTypes
from FreeRTOSgdb.List import ListInspector
from FreeRTOSgdb.GDBCommands import ShowHandleName, ShowRegistry, ShowList
from FreeRTOSgdb.GDBCommands import ShowQueueInfo


class Scheduler:
    def __init__(self):
        self._blocked = ListInspector("xSuspendedTaskList")
        self._delayed1 = ListInspector("xDelayedTaskList1")
        self._delayed2 = ListInspector("xDelayedTaskList2")
        self._readyLists = []
        readyTasksListsStr = "pxReadyTasksLists"
        readyListsSym, methodType = gdb.lookup_symbol(readyTasksListsStr)
        if readyListsSym != None:
            readyLists = readyListsSym.value()
            minIndex, maxIndex = readyLists.type.range()
            for i in range(minIndex, maxIndex + 1):
                readyList = readyLists[i]
                FRReadyList = ListInspector(readyList)
                self._readyLists.append(FRReadyList)
        else:
            print("Failed to Find Symbol: %s" % readyTasksListsStr)
            raise ValueError("Invalid Symbol!")

    def ShowTaskList(self):
        self.PrintTableHeader()
        for i, rlist in enumerate(self._readyLists):
            if i == 0:
                items = rlist.GetElements("TCB_t", 0)
            else:
                items = rlist.GetElements("TCB_t", 1)
            if len(items) > 0:
                print("Ready List {%d}: Num Tasks: %d" % (i, len(items)))
                for tcb, val in items:
                    self.PrintTaskFormatted(tcb)

        items = self._blocked.GetElements("TCB_t")
        print("Blocked List: Num Tasks: %d" % len(items))
        for tcb, val in items:
            self.PrintTaskFormatted(tcb)

        items = self._delayed1.GetElements("TCB_t")
        print("Delayed {1}: Num Tasks: %d" % len(items))
        for tcb, val in items:
            self.PrintTaskFormatted(tcb, val)

        items = self._delayed2.GetElements("TCB_t")
        print("Delayed {2}: Num Tasks: %d" % len(items))
        for tcb, val in items:
            self.PrintTaskFormatted(tcb, val)

    def PrintTableHeader(self):
        print("%16s %3s %4s" % ("Name", "PRI", "STCK"))

    def PrintTaskFormatted(self, task, itemVal=None):
        topStack = task["pxTopOfStack"]
        stackBase = task["pxStack"]
        highWater = topStack - stackBase
        taskName = task["pcTaskName"].string()
        taskPriority = task["uxPriority"]
        if itemVal != None:
            print("%16s %3s %4s %5s" % (taskName, taskPriority, highWater, itemVal))
        else:
            print("%16s %3s %4s" % (taskName, taskPriority, highWater))


class ShowTaskList(gdb.Command):
    """Generate a print out of the current tasks and their states."""

    def __init__(self):
        super(ShowTaskList, self).__init__("show Task-List", gdb.COMMAND_SUPPORT)

    def invoke(self, arg, from_tty):
        sched = Scheduler()
        sched.ShowTaskList()


ShowRegistry()
ShowList()
ShowTaskList()
ShowHandleName()
ShowQueueInfo()
