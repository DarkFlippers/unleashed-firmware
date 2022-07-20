/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2014                                                          */
/*   Portions COPYRIGHT 2017 STMicroelectronics                           */
/*   Portions Copyright (C) 2014, ChaN, all right reserved                */
/*------------------------------------------------------------------------*/

/**
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
**/

#include "fatfs/ff.h"

#if _FS_REENTRANT
/*------------------------------------------------------------------------*/
/* Create a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object, such as semaphore and mutex. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_cre_syncobj(/* 1:Function succeeded, 0:Could not create the sync object */
                   BYTE vol, /* Corresponding volume (logical drive number) */
                   _SYNC_t* sobj /* Pointer to return the created sync object */
) {
    int ret;

    //osSemaphoreDef(SEM);
    //*sobj = osSemaphoreCreate(osSemaphore(SEM), 1);
    *sobj = furi_mutex_alloc(FuriMutexTypeNormal);
    ret = (*sobj != NULL);

    return ret;
}

/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj(/* 1:Function succeeded, 0:Could not delete due to any error */
                   _SYNC_t sobj /* Sync object tied to the logical drive to be deleted */
) {
    furi_mutex_free(sobj);
    return 1;
}

/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant(/* 1:Got a grant to access the volume, 0:Could not get a grant */
                 _SYNC_t sobj /* Sync object to wait */
) {
    int ret = 0;

    if(furi_mutex_acquire(sobj, _FS_TIMEOUT) == FuriStatusOk) {
        ret = 1;
    }

    return ret;
}

/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant(_SYNC_t sobj /* Sync object to be signaled */
) {
    furi_mutex_release(sobj);
}

#endif

#if _USE_LFN == 3 /* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc(/* Returns pointer to the allocated memory block */
                  UINT msize /* Number of bytes to allocate */
) {
    return ff_malloc(msize); /* Allocate a new memory block with POSIX API */
}

/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree(void* mblock /* Pointer to the memory block to free */
) {
    ff_free(mblock); /* Discard the memory block with POSIX API */
}

#endif
