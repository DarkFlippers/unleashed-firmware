#pragma once
#define API_LOCK_INIT_LOCKED() osSemaphoreNew(1, 0, NULL);

#define API_LOCK_WAIT_UNTIL_UNLOCK_AND_FREE(_lock) \
    osSemaphoreAcquire(_lock, osWaitForever);      \
    osSemaphoreDelete(_lock);

#define API_LOCK_UNLOCK(_lock) osSemaphoreRelease(_lock);