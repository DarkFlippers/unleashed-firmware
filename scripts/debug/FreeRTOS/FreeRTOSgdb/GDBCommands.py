# File: GDBCommands.py
# Author: Carl Allendorph
# Date: 05NOV2014
#
# Description:
#   This file contains the implementation of some custom
# GDB commands for Inspecting the FreeRTOS state


import gdb
from .List import ListInspector
from .Task import TaskInspector
from .HandleRegistry import HandleRegistry
from .QueueTools import *


class ShowQueueInfo(gdb.Command):
    """Generate a print out of info about a particular
    set of queues.
    """

    def __init__(self):
        super(ShowQueueInfo, self).__init__("show Queue-Info", gdb.COMMAND_SUPPORT)

    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)

        qTypes = []
        if len(argv) > 0:
            for a in argv:
                try:
                    qType = QueueMode.Map[a]
                    qTypes.append(qType)
                except KeyError:
                    print("Arg %s does not map to a Queue Type!" % a)

        reg = HandleRegistry()
        qToShow = []
        if len(qTypes) > 0:
            # We will only print info about queues
            for qType in qTypes:
                qObjs = reg.FilterBy(qType)
                qToShow.extend(qObjs)
        else:
            qToShow = reg.FilterBy(None)

        print("Num Queues: %d" % len(qToShow))
        print("%20s %4s %16s %16s" % ("NAME", "CNT", "SEND", "RECEIVE"))
        for q in qToShow:
            self.PrintQueueInfo(q)

    def PrintQueueInfo(self, q):
        """Print Info about the Queue"""
        sendList = q.GetTasksWaitingToSend()
        rxList = q.GetTasksWaitingToReceive()

        # print("TxLen: %d, RxLen: %d" % (len(sendList), len(rxList)))

        maxCount = max(len(sendList), len(rxList))
        outputFmt = "%20s %4s %16s %16s"
        if maxCount == 0:
            print(outputFmt % (q.GetName(), q.GetQueueMessagesWaiting(), "", ""))
        else:
            for i in range(0, maxCount):
                txName = ""
                if i < len(sendList):
                    tcbRef, val = sendList[i]
                    tcb = TaskInspector(tcbRef)
                    txName = tcb.GetName()
                rxName = ""
                if i < len(rxList):
                    tcbRef, val = rxList[i]
                    tcb = TaskInspector(tcbRef)
                    rxName = tcb.GetName()

                if i == 0:
                    print(
                        outputFmt
                        % (q.GetName(), q.GetQueueMessagesWaiting(), txName, rxName)
                    )
                else:
                    print(outputFmt % ("", "", txName, rxName))


class ShowHandleName(gdb.Command):
    """Generate a print out of the handle by name"""

    def __init__(self):
        super(ShowHandleName, self).__init__("show Handle-Name", gdb.COMMAND_SUPPORT)

    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)
        if len(argv) != 1:
            print("Invalid Argument: Requires one handle arg")
        handle = int(argv[0], 0)
        reg = HandleRegistry()
        name = reg.GetName(handle)
        print("Handle 0x%08x: %s" % (handle, name))


class ShowRegistry(gdb.Command):
    """Generate a print out of the queue handle registry"""

    def __init__(self):
        super(ShowRegistry, self).__init__("show Handle-Registry", gdb.COMMAND_SUPPORT)

    def invoke(self, arg, from_tty):
        reg = HandleRegistry()
        reg.PrintRegistry()


class ShowList(gdb.Command):
    """Generate a print out of the elements in a list
    passed to this command. User must pass a symbol that
    will be looked up.
    """

    def __init__(self):
        super(ShowList, self).__init__(
            "show List-Handle", gdb.COMMAND_SUPPORT, gdb.COMPLETE_SYMBOL
        )

    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)

        CastTypeStr = None
        if len(argv) > 0:
            symbolArg = argv[0]

        if len(argv) > 1:
            CastTypeStr = argv[1]

        listVal = ListInspector(symbolArg)

        elems = listVal.GetElements(CastTypeStr)

        for elem in elems:
            print("Elem: %s" % str(elem))
