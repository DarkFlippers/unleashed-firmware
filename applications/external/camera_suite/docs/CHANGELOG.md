## v1.1

- Support and picture stabilization for all camera orientations (0°, 90°, 180°, 270°).
- Rename "Scene 1" to "Camera". No UX changes, strictly internal.
- Clean up unused "Scene 2". This was inaccessible to users previously and unused.
- Add new dithering variations (needs new module firmware, see https://github.com/CodyTolene/Flipper-Zero-Camera-Suite#firmware-installation):
  - Add `Jarvis Judice` Ninke Dithering option
  - Add `Stucki` dithering option.
  - Add ability to toggle dithering options from default `Floyd-Steinberg` and back.
- Resolves issue https://github.com/CodyTolene/Flipper-Zero-Camera-Suite/issues/7
- Resolves issue https://github.com/CodyTolene/Flipper-Zero-Camera-Suite/pull/17

## v1.0

- Builds upon Z4urce's software found here (updated 6 months ago): https://github.com/Z4urce/flipperzero-camera
- Utilizes the superb C boilerplate examples laid out by leedave (updated last month): https://github.com/leedave/flipper-zero-fap-boilerplate
- Repurpose and build upon the "[ESP32] Camera" software into the new "[ESP32] Camera Suite" application with new purpose:
  - Adding more scene for a guide.
  - Adding more scene for saveable settings.
  - Add ability to rotate the camera orientation.
