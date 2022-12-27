Needs the [WAV Player application](https://github.com/flipperdevices/flipperzero-firmware/tree/zlo/wav-player/applications/wav_player) compiled into firmware. Original code from [Zlo](https://github.com/flipperdevices/flipperzero-firmware/tree/zlo/wav-player) and most files provided by [RogueMaster](https://github.com/RogueMaster).

Create a wav_player folder in the root of your SD card. You'll need **8-bit, 2 channel, unsigned WAV files** (try [Audacity](https://www.audacityteam.org/) for conversion.)

WAV Player is very particular about the format. WavPlayer needs BitExact, NO headers, 48k PCM, unsigned, 2 channel, 8-bit stereo. Example:

`Input #0, wav, from '.\rickroll.wav':`<br>
`  Duration: 00:02:03.38, bitrate: 768 kb/s`<br>
`    Stream #0:0: Audio: pcm_u8 ([1][0][0][0] / 0x0001), 48000 Hz, stereo, u8, 768 kb/s`

To produce a compatible file, you can also use [FFmpeg](https://ffmpeg.org/):<br>
`ffmpeg -i .\input.mp3 -c:a pcm_u8 -fflags +bitexact -flags:a +bitexact -ac 2 -ar 48k output.wav`

-----

I've had so many asking for me to add this.<br>
![Flipper_Blush](https://user-images.githubusercontent.com/57457139/183561666-4424a3cc-679b-4016-a368-24f7e7ad0a88.jpg) ![Flipper_Love](https://user-images.githubusercontent.com/57457139/183561692-381d37bd-264f-4c88-8877-e58d60d9be6e.jpg)

**BTC**: `3AWgaL3FxquakP15ZVDxr8q8xVTc5Q75dS`<br>
**ETH**: `0x0f0003fCB0bD9355Ad7B124c30b9F3D860D5E191`

So, here it is. All donations of *any* size are humbly appreciated.<br>
![Flipper_Clap](https://user-images.githubusercontent.com/57457139/183561789-2e853ede-8ef7-41e8-a67c-716225177e5d.jpg) ![Flipper_OMG](https://user-images.githubusercontent.com/57457139/183561787-e21bdc1e-b316-4e67-b327-5129503d0313.jpg)

Donations will be used for hardware (and maybe caffine) to further testing!<br>
![UberGuidoZ](https://cdn.discordapp.com/emojis/1000632669622767686.gif)
