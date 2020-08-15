Flipper Universal Registry Implementation or FURI is important part of Flipper firmware. It is used to:

* application control (start, exit, switch between active)
* data exchange between application (create/open channel, subscribe and push messages or read/write values)
* non-volatile data storage for application (create/open value and read/write)

# Application registry and control (FURIAC)

### Start and change application wrokflow

* `FuriApp furiac_start(void(app*)(void*), char* name, void* param)` simply starts application. It call `app` entrypoint with `param` passed as argument. Useful for daemon applications and pop-up.
* `FuriApp furiac_switch(void(app*)(void*), char* name, void* param)` swtich to other application. FURI **stop current app**, call `app` entrypoint with `param` passed as argument and save current application entrypoint to `prev` field in current application registry. Useful for UI or "active" application.

### Exit application

* `furiac_exit(void* param)` stop current application (stop thread and clear application's stack), start application from `prev` entry in current application registry, cleanup current application registry.
* `furiac_kill(FuriApp app)` stop specified `app` without returning to `prev` application.

# Data exchange

* `FuriRecord furi_create(char* name)` creates named FURI record. Returns NULL if registry have not enough memory for creating.
* `FuriRecord furi_open(char* name, bool solo, bool no_mute)` opens existing FURI record by name. Returns NULL if record does not exist. If `solo` is true **another applications handlers set into "muted" state**. When appication has exited or record has closed, all handlers is unmuted. It may be useful for concurrently acces to resources like framebuffer or beeper. If `no_mute` is true, another applications cannot mute this handler.
* `bool furi_close(FuriRecord record)` close handler and unmute anothers.
* `bool furi_read(FuriRecord record, void* data, size_t size)` read message from record. Returns true if success, false otherwise.
* `bool furi_write(FuriRecord record, const void* data, size_t size)` write message to record. Returns true if success, false otherwise (handler gone or muted).
* `bool furi_take(FuriRecord record, void* data, size_t size)` works as `furi_read` but lock global mutex. It can be useful if records contain pointer to buffer which you want to change. You must call `furi_give` after operation on data and you cannot block executing between `take` and `give` calls
* `bool furi_give(FuriRecord record, const void* data, size_t size)` works as `furi_wrte` but unlock global mutex.
* `bool furi_global_take()` lock global mutex (as `furi_take` but without read)
* `boold furi_global_give()` unlock global mutex ((as `furi_give` but without write))
* `bool furi_unmute(FuriRecord record)` unmutes muted record.
* `bool furi_mute(FuriRecord record)` mutes unmuted record.
* `bool furi_subscribe(FuriRecord record, void(cb*)(const void* data, size_t size))` set record change callback.
* `bool furi_state_subscribe(FuriRecord record, void(cb*)(bool muted))` set record state change callback (mute/unmute). For example, you can unmute itself after some application open same record, or redraw your application UI when popup application ends.

# Usage example
_Diagram below describes furi states_

![FURI states](https://github.com/Flipper-Zero/wiki/raw/master/images/furi_states.png)

* After start, init code run some applications: in this example there is status bar, a background task and Home screen
* Status bar open access to framebuffer by opening "/ui/fb" FURI record
* "Home screen" call "Menu" application by `furiac_switch`. "Home screen" application stops and then "Menu" application starts.
* "Menu" application call "Your cool app" the same way. It also get access to framebuffer by open "/ui/fb" FURI record
* If "Your cool app" needs some backend app, it call this by `furiac_start` and then kill by `furiac_kill`
* If background task needs to show popup message (for example "Low battery") it can call new app or simply open "/ui/fb" record.
* When "/ui/fb" record is opened by popup message, FURI mute framebuffer handle in "Your cool app". This prevent to overwrite popup message by application drawing.
* "Status bar" framebuffer handle also is muted, but it call `furi_unmute` and unmute itself.
* After popup message is closed by `furiac_exit` or closing "/ui/fb", FURI unmute previous muted "Your cool app" framebuffer handle.

_Status bar also get mute and unmute itself every time when Home screen, Menu or "Your cool app" open framebuffer but diagramm not show it_

# Data storage

* `furi_create_var(char* name)` create static-like value handler. You can use all furi_ calls for