# Changelog

## 1.3

- Add a daily alarm. Press Right to open the setup screen: toggle it On/Off and set a time with a simple hour/minute picker. It fires every day at that time.
- When the alarm goes off the screen flashes a big `ALARM` and a melody plays. Any key stops it, and the alarm stays armed so it goes off again the next day.
- The alarm setup and the on-screen alarm readout follow the system locale: 12-hour with AM/PM (the picker gets an AM/PM field) or 24-hour, matching the clock itself.
- The brightness bar now hides 3 seconds after the last Up/Down press (previously it was tied to the redraw count, so it lingered for an unpredictable time).
- Remember the alarm and the brightness level across restarts, saved under `/ext/apps_data/clock/`, and apply the saved brightness on launch.
- Internal: rebuilt around a ViewDispatcher to host the alarm setup screens alongside the clock.

## 1.2

- Fix Up/Down not actually changing the screen brightness: the new level is now pushed to the panel even while the backlight is held always-on (previously the on-screen bar moved but the backlight stayed at the startup brightness)

## 1.1

- Fix the 12-hour clock showing midnight as `00:xx AM` instead of `12:xx AM`
- Fix the brightness control so the red LED nightlight stays reachable from every starting brightness: round the inherited system brightness to the nearest step and clamp brightness to 0-100 (previously levels such as 35/45/65/70/90/95% started off-grid, so Down skipped 0 and the nightlight could never be switched on)
- Restore the notification settings before releasing the notification record when exiting
- Free the view_port and notification record (and restore the display settings) if the periodic timer fails to allocate on startup

## 1.0

- Initial release: overnight clock with screen-brightness control, red LED nightlight, and a stopwatch
