# File: HandleRegistry.py
# Author: Carl Allendorph
# Date: 06NOV2014
#
# Description:
#   THis file contains the implementation of a class for accessing the
# handle registry. This contains a mapping of queue handles to
# strings for labeling purposes.

import gdb
from .Types import StdTypes
from .QueueTools import *


class HandleRegistry:
    """The FreeRTOS system can be configured with a table that
    associates a name with a QueueHandle_t.
    This class can be used to access this table and
    label queue/mutex/semaphore/event groups
    """

    def __init__(self, regSymbol="xQueueRegistry"):
        symbol, methodObj = gdb.lookup_symbol(regSymbol)
        self._registry = symbol.value()
        self._minIndex = 0
        self._maxIndex = 0
        self._minIndex, self._maxIndex = self._registry.type.range()

    def GetName(self, handle):
        """Find the string name associated with a queue
        handle if it exists in the registry
        """
        for i in range(self._minIndex, self._maxIndex):
            elem = self._registry[i]
            h = elem["xHandle"]
            val = h.cast(StdTypes.uint32_t)
            if handle == val:
                print("Found Entry for: %x" % handle)
                name = elem["pcQueueName"].string()
                return name

    def PrintRegistry(self):
        for i in range(self._minIndex, self._maxIndex):
            elem = self._registry[i]
            h = elem["xHandle"]
            if h != 0:
                name = elem["pcQueueName"].string()
                print("%d: %3s %16s" % (i, h, name))

    def FilterBy(self, qMode):
        """Retrieve a List of Mutex Queue Handles"""
        resp = []
        for i in range(self._minIndex, self._maxIndex):
            elem = self._registry[i]
            h = elem["xHandle"]
            if h != 0:
                name = elem["pcQueueName"].string()
                q = QueueInspector(h)
                q.SetName(name)
                if qMode != None:
                    qType = q.GetQueueType()
                    if qType != None:
                        if qType == qMode:
                            resp.append(q)

                    else:
                        print("qType == None")
                else:
                    resp.append(q)

        return resp

    def GetMutexes(self):
        """Retrieve all the Mutex Objects in the Handle Registry"""
        return self.FilterBy(QueueMode.MUTEX)

    def GetSemaphores(self):
        """Retrieve all the Semaphore Objects in the Handle Registry"""
        return self.FilterBy(QueueMode.BINARY)

    def GetQueues(self):
        """Retrieve all the Queue Objects in the Handle Registry"""
        return self.FilterBy(QueueMode.QUEUE)
