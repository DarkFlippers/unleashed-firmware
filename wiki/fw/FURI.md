Flipper Universal Registry Implementation or FURI is important part of Flipper firmware. It is used to:

* application control (start, exit, switch between active)
* data exchange between application (create/open channel, subscribe and push messages or read/write values)
* non-volatile data storage for application (create/open value and read/write)

# Application registry and control (FURIAC)



# Data exchange

**`bool furi_create(char* name, void* value, size_t size)`**

creates named FURI record. Returns NULL if registry have not enough memory for creating. If value is NULL, create FURI Pipe (only callbacks management, no data/mutex).

**`FuriRecordHandler furi_open(char* name, bool solo, bool no_mute, void(*FlipperRecordCallback)(const void*, size_t), void(*FlipperRecordStateCallback)(FlipperRecordState))`**

opens existing FURI record by name. Returns NULL if record does not exist. If `solo` is true **another applications handlers set into "muted" state**. When appication has exited or record has closed, all handlers is unmuted. It may be useful for concurrently acces to resources like framebuffer or beeper. If `no_mute` is true, another applications cannot mute this handler.

**`bool furi_close(FuriRecordHandler* record)`**

close handler and unmute anothers.

**`bool furi_read(FuriRecordHandler* record, void* data, size_t size)`**

read message from record. Returns true if success, false otherwise.

**`bool furi_write(FuriRecordHandler* record, const void* data, size_t size)`**

write message to record. Returns true if success, false otherwise (handler gone or muted).

**`void* furi_take(FuriRecordHandler* record)` works as `furi_read`**

lock value mutex. It can be useful if records contain pointer to buffer which you want to change. You must call `furi_give` after operation on data and you cannot block executing between `take` and `give` calls

**`bool furi_give(FuriRecordHandler* record)`**

unlock value mutex works as `furi_wrte` but unlock global mutex.

# Usage example
_Diagram below describes furi states_

![FURI states](https://github.com/Flipper-Zero/flipperzero-firmware-community/blob/master/wiki_static/furi_states.png)

* After start, init code run some applications: in this example there is status bar, a background task and Home screen
* Status bar open access to framebuffer by opening "/ui/fb" FURI record
* "Home screen" call "Menu" application by `furiac_switch`. "Home screen" application stops and then "Menu" application starts.
* "Menu" application call "Your cool app" the same way. It also get access to framebuffer by open "/ui/fb" FURI record
* If "Your cool app" needs some backend app, it call this by `furiac_start` and then kill by `furiac_kill`
* If background task needs to show popup message (for example "Low battery") it can call new app or simply open "/ui/fb" record.
* When "/ui/fb" record is opened by popup message, FURI mute framebuffer handle in "Your cool app". This prevent to overwrite popup message by application drawing.
* "Status bar" framebuffer handle not is muted, beacuse open framebuffer with no_mute=true.
* After popup message is closed by `furiac_exit` or closing "/ui/fb", FURI unmute previous muted "Your cool app" framebuffer handle.
