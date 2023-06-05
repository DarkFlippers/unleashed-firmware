## Working Improvement

#### Quality of life

- [ ] Make the "Load File" independent of the current protocol
- [x] Add pause
    - [ ] Switching  UIDs if possible
- [ ] Led and sound Notification
- [ ] Error Notification
    - [ ] Custom UIDs dict loading 
    - [ ] Key file loading
    - [ ] Anything else

#### App functionality

- [x] Add `BFCustomerID` attack
- [ ] Save key logic

## Code Improvement

- [ ] GUI
    - [x] Rewrite `gui_const` logic
    - [ ] Separate protocol name from `fuzzer_proto_items` 
    - [x] Icon in dialog
    - [ ] Description and buttons in `field_editor` view
    - [ ] Protocol carousel in `main_menu`
        - [x] prototype 
- [x] UID
    - [x] Simplify the storage and exchange of `uids.data` `uid.data_size` in `views`
    - [x] `UID_MAX_SIZE`
- [x] Add pause
    - [x] Fix `Custom dict` attack when ended
- [x] this can be simplified `fuzzer_proto_items`
