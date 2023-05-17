# File: Task.py
# Author: Carl Allendorph
# Date: 05NOV2014
#
# Description:
#  This file contains the implementation of a class to use for
# inspecting the state of a FreeRTOS Task in GDB
#

import gdb


class TaskInspector:
    TCBType = gdb.lookup_type("TCB_t")

    def __init__(self, handle):
        self._tcb = None
        # print("Task: Pass Handle: %s" % str(handle))

        try:
            if handle.type == TaskInspector.TCBType:
                self._tcb = handle
                return
            else:
                print("Handle Type: %s" % str(handle.type))

        except AttributeError as aexc:
            print("Attribute Error: %s" % str(aexc))
            pass
        except Exception as exc:
            print("Error Initializing Task Inspector: %s" % str(exc))
            raise

        try:
            tcbPtr = gdb.Value(handle).cast(TaskInspector.TCBType.pointer())
            self._tcb = tcbPtr.dereference()
            return
        except Exception as exc:
            print("Failed to convert Handle Pointer: %s" % str(handle))

        self._tcb = handle

    def GetName(self):
        if self._tcb != None:
            return self._tcb["pcTaskName"].string()
        else:
            raise ValueError("Invalid TCB")

    def GetPriority(self):
        if self._tcb != None:
            return self._tcb["uxPriority"]
        else:
            raise ValueError("Invalid TCB")

    def GetStackMargin(self):
        if self._tcb != None:
            topStack = self._tcb["pxTopOfStack"]
            stackBase = self._tcb["pxStack"]
            highWater = topStack - stackBase
            return highWater
        else:
            raise ValueError("Invalid TCB")
