## How to add new SubGHz frequencies

#### CC1101 Frequency range specs: 300-348 MHz, 386-464 MHz, and 778-928 MHz  (+ 350MHz and 467MHz was added to default range)

Edit user settings file located on your microSD card - `subghz/assets/setting_user` (remove .example from name to use config)

in this file you will find we already have extra frequencies added
if you need your custom one, make sure it doesn't listed here

### Default frequency list
```
    /* 300 - 348 */
    300000000,
    302757000,
    303875000,
    303900000,
    304250000,
    307000000,
    307500000,
    307800000,
    309000000,
    310000000,
    312000000,
    312100000,
    312200000,
    313000000,
    313850000,
    314000000,
    314350000,
    314980000,
    315000000,
    318000000,
    330000000,
    345000000,
    348000000,
    350000000,

    /* 387 - 464 */
    387000000,
    390000000,
    418000000,
    430000000,
    430500000,
    431000000,
    431500000,
    433075000, /* LPD433 first */
    433220000,
    433420000,
    433657070,
    433889000,
    433920000 | FREQUENCY_FLAG_DEFAULT, /* LPD433 mid */
    434075000,
    434176948,
    434190000,
    434390000,
    434420000,
    434620000,
    434775000, /* LPD433 last channels */
    438900000,
    440175000,
    464000000,
    467750000,

    /* 779 - 928 */
    779000000,
    868350000,
    868400000,
    868800000,
    868950000,
    906400000,
    915000000,
    925000000,
    928000000,
```

### User frequencies added AFTER that default list! You need to continue until you reach the end of that list

### If you want to disable default list and use ONLY user added frequencies from user settings file
Change that line
`#Add_standard_frequencies: true`
to
`Add_standard_frequencies: false`

**You need to have custom frequencies added in both lists! in main frequency list and in hopping list! Replacing only hopping freqs will not work with that setting set on false, you need to add something in main list since it will be empty**

### To add your own frequency to user list 
Just add new line
`Frequency: 928000000` - where `928000000` is your frequency, keep it in that format! it should be 9 digits!

### Hopper frequency list
To add new frequency to hopper:
add new line `Hopper_frequency: 345000000`<br>
But remember! You should keep it as small as possible, or hopper functionality would be useless!<br>
If `#Add_standard_frequencies: true` is not changed<br>
Your frequencies will be added after default ones

### Default hopper list
```
    310000000,
    315000000,
    318000000,
    418000000,
    433920000,
    868350000,
```
