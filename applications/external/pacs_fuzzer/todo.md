## Working Improvement

#### Quality of life

- [ ] Make the "Load File" independent of the current protocol
- [x] Add pause
    - [ ] Switching  UIDs if possible
- [x] Led and sound Notification
    - [x] Led
    - [x] Vibro
    - [ ] Sound?
- [x] Error Notification
    - [x] Custom UIDs dict loading 
    - [x] Key file loading
    - [ ] Anything else

#### App functionality

- [x] Add `BFCustomerID` attack
    - [x] Add the ability to select index
- [ ] Save key logic

## Code Improvement

- [ ] GUI
    - [x] Rewrite `gui_const` logic
    - [x] Icon in dialog
    - [x] Description and buttons in `field_editor` view
    - [ ] Protocol carousel in `main_menu`
        - [x] prototype 
    - [x] Add the ability to edit emulation time and downtime separately
        - [x] Decide on the display
- [x] UID
    - [x] Simplify the storage and exchange of `uids.data` `uid.data_size` in `views`
    - [x] Using `FuzzerPayload` to store the uid
    - [x] `UID_MAX_SIZE`
- [x] Add pause
    - [x] Fix `Custom dict` attack when ended
- [ ] Pause V2
    - [ ] Save logic
    - [ ] Switching  UIDs if possible
- [ ] Worker
    - [ ] Use `prtocol_id` instead of protocol name
    - [x] this can be simplified `fuzzer_proto_items`