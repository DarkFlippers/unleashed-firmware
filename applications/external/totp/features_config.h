// Include Bluetooth token input automation
#define TOTP_BADBT_TYPE_ENABLED

// Include token input automation icons on the main screen
#define TOTP_AUTOMATION_ICONS_ENABLED

// List of compatible firmwares
#define TOTP_FIRMWARE_OFFICIAL_STABLE 1
#define TOTP_FIRMWARE_OFFICIAL_DEV 2
#define TOTP_FIRMWARE_UL_XFW 3 // XFW and UL now has same bluetooth mac/advname changing API
// End of list

// Target firmware to build for
#define TOTP_TARGET_FIRMWARE TOTP_FIRMWARE_UL_XFW

// Max custom fonts value
#define MAX_CUSTOM_FONTS (9)