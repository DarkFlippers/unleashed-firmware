#pragma once

typedef enum {
    DesktopMainEventOpenLockMenu,
    DesktopMainEventOpenArchive,
    DesktopMainEventOpenFavorite,
    DesktopMainEventOpenMenu,
    DesktopMainEventOpenDebug,
    DesktopMainEventOpenPassport, /**< Broken, don't use it */

    DesktopLockedEventUnlocked,
    DesktopLockedEventUpdate,
    DesktopLockedEventShowPinInput,

    DesktopPinInputEventResetWrongPinLabel,
    DesktopPinInputEventUnlocked,
    DesktopPinInputEventUnlockFailed,
    DesktopPinInputEventBack,

    DesktopPinTimeoutExit,

    DesktopDebugEventDeed,
    DesktopDebugEventWrongDeed,
    DesktopDebugEventSaveState,
    DesktopDebugEventExit,

    DesktopFirstStartCompleted,
    DesktopFirstStartPoweroff,

    DesktopLockMenuEventLock,
    DesktopLockMenuEventPinLock,
    DesktopLockMenuEventExit,

    DesktopAnimationEventCheckAnimation,
    DesktopAnimationEventNewIdleAnimation,
    DesktopAnimationEventInteractAnimation,

    // Global events
    DesktopGlobalBeforeAppStarted,
    DesktopGlobalAfterAppFinished,
    DesktopGlobalAutoLock,
} DesktopEvent;
