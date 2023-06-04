## Working Improvement

#### Quality of life

- [ ] Make the "Load File" independent of the current protocol
- [ ] Add pause
    - [ ] Switching  UIDs if possible
- [ ] Led and sound Notification
- [ ] Error Notification
    - [ ] Custom UIDs dict loading 
    - [ ] Key file loading
    - [ ] Anything else

#### App functionality

- [ ] Add `BFCustomerID` attack
- [ ] Save key logic

## Code Improvement

- [ ] GUI
    - [ ] Rewrite `gui_const` logic
    - [ ] Separate protocol name from `fuzzer_proto_items` 
    - [ ] Icon in dialog
    - [ ] Description and buttons in `field_editor` view
    - [ ] Protocol carousel in `main_menu`
- [ ] UID
    - [ ] Simplify the storage and exchange of `uids.data` `uid.data_size` in `views`
    - [ ] `UID_MAX_SIZE`
- [ ] Add pause
    - [ ] Fix `Custom dict` attack when ended
- [ ] this can be simplified `fuzzer_proto_items`
