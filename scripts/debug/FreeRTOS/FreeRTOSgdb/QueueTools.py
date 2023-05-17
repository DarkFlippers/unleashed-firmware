# File: Queue.py
# Author: Carl Allendorph
# Date: 05NOV2014
#
# Description:
#   This file contains the implementation of a Queue Inspector
# class.
#

import gdb
from .List import ListInspector
from .Task import TaskInspector


class QueueMode:
    QUEUE = 0
    MUTEX = 1
    COUNTING = 2
    BINARY = 3
    RECURSIVE = 4

    Map = None

    @staticmethod
    def IsValid(qType):
        if (
            qType == QueueMode.QUEUE
            or qType == QueueMode.MUTEX
            or qType == QueueMode.COUNTING
            or qType == QueueMode.BINARY
            or qType == QueueMode.RECURSIVE
        ):
            return True
        else:
            return False


QueueMap = {
    "mutex": QueueMode.MUTEX,
    "queue": QueueMode.QUEUE,
    "semaphore": QueueMode.BINARY,
    "counting": QueueMode.COUNTING,
    "recursive": QueueMode.RECURSIVE,
}

QueueMode.Map = QueueMap


class QueueInspector:
    QueueType = gdb.lookup_type("Queue_t")

    def __init__(self, handle):
        #    print("Queue: Handle: %s" % handle)
        self.name = None
        queueObjPtr = None
        if type(handle) == gdb.Value:
            queueObjPtr = handle.cast(QueueInspector.QueueType.pointer())
            self._queue = queueObjPtr.dereference()
        else:
            queueObjPtr = gdb.Value(handle).cast(QueueInspector.QueueType.pointer())
        self._queue = queueObjPtr.dereference()

    def GetName(self):
        return self.name

    def SetName(self, name):
        self.name = name

    def GetTasksWaitingToSend(self):
        """Retrieve a list of gdb.Value objects of type
        TCB that are the tasks that are currently waiting to
        send data on this queue object.
        """
        sendList = ListInspector(self._queue["xTasksWaitingToSend"])

        return sendList.GetElements(TaskInspector.TCBType)

    def GetTasksWaitingToReceive(self):
        """Retrieve a list of gdb.Value objects of Type
        TCB that are the tasks that are currently waiting to
        receive data on this queue object.
        """
        rxList = ListInspector(self._queue["xTasksWaitingToReceive"])
        return rxList.GetElements(TaskInspector.TCBType)

    def GetQueueMessagesWaiting(self):
        """Return the number of messages waiting as a
        L{gdb.Value} object
        """
        return self._queue["uxMessagesWaiting"]

    def GetQueueType(self):
        """Return the Type of the Queue as a enumerated number"""
        try:
            qType = self._queue["ucQueueType"]
            if QueueMode.IsValid(int(qType)):
                return qType
            else:
                raise ValueError(
                    "Invalid Queue Type In Queue Object! Are you sure this is a Queue Handle?"
                )

        except Exception as exc:
            # If the TRACE functionality of the RTOS is not enabled,
            #  then the queue type will not be availabe in the queue
            #  handle - so we return None
            print("Failed to get Type: %s" % str(exc))
            return None
