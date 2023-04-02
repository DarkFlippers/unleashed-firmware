#pragma once

typedef enum {
    //SubmenuIndex
    SubmenuIndexAvrIspProgrammer = 10,
    SubmenuIndexAvrIspReader,
    SubmenuIndexAvrIspWriter,
    SubmenuIndexAvrIsWiring,
    SubmenuIndexAvrIspAbout,

    //AvrIspCustomEvent
    AvrIspCustomEventSceneChipDetectOk = 100,
    AvrIspCustomEventSceneReadingOk,
    AvrIspCustomEventSceneWritingOk,
    AvrIspCustomEventSceneErrorVerification,
    AvrIspCustomEventSceneErrorReading,
    AvrIspCustomEventSceneErrorWriting,
    AvrIspCustomEventSceneErrorWritingFuse,
    AvrIspCustomEventSceneInputName,
    AvrIspCustomEventSceneSuccess,
    AvrIspCustomEventSceneExit,
    AvrIspCustomEventSceneExitStartMenu,
} AvrIspCustomEvent;
