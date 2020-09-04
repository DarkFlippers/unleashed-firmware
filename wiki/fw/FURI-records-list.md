_ (not implemented)_

List of [FURI](FURI) records for exchange data between applications.

# Interrupts

* `/irq/buttons` &mdash; raw button press/release events.

|Name|Type|Size|
|---|---|---|
|Button|0 &mdash; Up<br/>1 &mdash; Down<br/>2 &mdash; Right<br/>3 &mdash; Left<br/>4 &mdash; Ok<br/>5 &mdash; Back|1|
|State|1 &mdash; pressed<br/>0 &mdash; released|1|

* `/irq/charge` &mdash; charge state event

# UI

|Name|Type|Size|
|---|---|---|
|State|1 &mdash; charge start<br/>0 &mdash; charge stop|1|

* `/ui/fb` &mdash; pointer to current framebuffer

|Name|Type|Size|
|---|---|---|
|Framebuffer pointer|`uint8_t[DISPLAY_WIDTH][DISPAY_HEIGHT]`|4|

* `/ui/leds` &mdash; user led state

Led state is overrided by charge state (red when charging, green when charged).

|Name|Type|Size|
|---|---|---|
|Red|pwm value (0..255)|1|
|Green|pwm value (0..255)|1|
|Blue|pwm value (0..255)|1|
|Enable|1 &mdash; user led enabled<br/>0 &mdash; user led disabled (for manual led control)|1|

* `/ui/buttons_event` &mdash; button press/release events after debounce.

|Name|Type|Size|
|---|---|---|
|Button|0 &mdash; Up<br/>1 &mdash; Down<br/>2 &mdash; Right<br/>3 &mdash; Left<br/>4 &mdash; Ok<br/>5 &mdash; Back|1|
|State|1 &mdash; pressed<br/>0 &mdash; released|1|

* `/ui/buttons_state` &mdash; current button state after debounce.

|Name|Type|Size|
|---|---|---|
|Up|1 &mdash; pressed<br/>0 &mdash; released|1|
|Down|1 &mdash; pressed<br/>0 &mdash; released|1|
|Right|1 &mdash; pressed<br/>0 &mdash; released|1|
|Left|1 &mdash; pressed<br/>0 &mdash; released|1|
|Ok|1 &mdash; pressed<br/>0 &mdash; released|1|
|Back|1 &mdash; pressed<br/>0 &mdash; released|1|

* `/ui/fullscreen` &mdash; fullscreen mode state

|Name|Type|Size|
|---|---|---|
|State|1 &mdash; fullscreen<br/>0 &mdash; no fullscreen|1|