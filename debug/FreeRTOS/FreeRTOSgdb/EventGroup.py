# File: EventGroup.py
# Author: Carl Allendorph
# Date: 05NOV2014
#
# Description:
#   This file contains the implementation of a Event Group Inspector


import gdb
from .List import ListInspector
from .Task import TaskInspector


class EventGroupInspector:
    EvtGrpType = gdb.lookup_type("EventGroup_t")

    def __init__(self, handle):
        self._evtgrp = gdb.Value(handle).cast(EventGroupInspector.EvtGrpType)

    def GetTasksWaiting(self):
        taskListObj = self._evtgrp["xTasksWaitingForBits"]
        taskList = ListInspector(taskListObj)
        return taskList.GetElements(TaskInspector.TCBType)

    def GetEventBits(self):
        """Get the Event Flag Bits
        @return L{gdb.Value} of EventBits_t
        """
        return self._evtgrp["uxEventBits"]
