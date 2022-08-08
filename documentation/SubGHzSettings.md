## How to add new SubGHz frequencies

#### CC1101 Frequency range specs: 300-348 MHz, 386-464 MHz, and 778-928 MHz 

Edit user settings file located on your microSD card - `subghz/assets/setting_user`

in this file you will find we already have extra frequencies added
if you need your custom one, make sure it doesnt listed here

### Default frequency list
```
    /* 300 - 348 */
    300000000,
    303875000,
    304250000,
    310000000,
    315000000,
    318000000,

    /* 387 - 464 */
    390000000,
    418000000,
    433075000, /* LPD433 first */
    433420000,
    433920000  /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,

    /* 779 - 928 */
    868350000,
    915000000,
    925000000,
```

### User frequencies added AFTER that default list! You need to continue until you reach the end of that list

### If you want to disable default list and use ONLY user added frequecies from user settings file
Change that line
`#Add_standard_frequencies: true`
to
`Add_standard_frequencies: false`

### To add your own frequency to user list 
Just add new line
`Frequency: 928000000` - where `928000000` is your frequency, keep it in that format! it should be 9 digits!

### Hopper frequency list
To add new frequecy to hopper:
add new line `Hopper_frequency: 345000000`<br>
But remember! You should keep it as small as possible, or hopper functionality would be useless!<br>
If `#Add_standard_frequencies: true` is not changed<br>
Your frequencies will be added after default ones

### Default hopper list
```
    310000000,
    315000000,
    318000000,
    390000000,
    433920000,
    868350000,
```