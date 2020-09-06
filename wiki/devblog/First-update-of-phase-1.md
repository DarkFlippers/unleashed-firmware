# What is done

## Peoples and management

1. We have added many contributors within the **phase-1**. Checkout [all welcome issues here](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues?q=is%3Aissue+label%3Awelcome+) and get to know each other.
2. Now we have an Official [Discord server](https://flipperzero.one/discord) with separate channels for developers and users. There you can chat and join voice channels. [Read more](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Communication) about how it works.
3. [Developers blog](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Developer-blog) created. Here you can read project updates in digest format.
4. [backlog](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues?q=label%3Abacklog+) label added for tasks that are not a priority at the moment.

## Environment

1. Added pipeline to automate Wiki pages building [#63](https://github.com/Flipper-Zero/flipperzero-firmware-community/pull/63). Please read (Hot to edit wiki)[https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Contributing#how-to-edit-wiki] in a right way.
2. Adding CI workflow has begun:  [#70](https://github.com/Flipper-Zero/flipperzero-firmware-community/pull/70). Now GitHub pipelines checking that `target_lo` and `target_f1` are successfully compiling.
3. Added Rust support into docker image: [#41](https://github.com/Flipper-Zero/flipperzero-firmware-community/pull/41) + [#68](https://github.com/Flipper-Zero/flipperzero-firmware-community/pull/68). Now you can build Rust code, link it with C and together, using Bindgen and Cbindgen.

## Core and stuff

1. Added `target_f1`, now you can build your code for [F1](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F1B1C0.0) board.
2. Added implementation of [FURI](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/FURI) (with many issues -- see [#59](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/59))...
3. ...and add many examples of how to use FURI, HAL and do some funny things ([List of Application Examples](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Application-examples)):
	1. [LED Blink](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Blink-app)
	2. [Writing to UART](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/UART-write)
	3. [Communication between apps](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/IPC-example)

## Hardware

1. We designed and manufactured [F2B0C1.1](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F2B0C1.1)! You can see that these boards already got an SD-card slot!  This will be a current Dev Kits.

<img width="500" src="https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki_static/blog/f2b0c1.1.jpeg" />

# What are we doing right now

1. Making UI and display driver [#98](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/98), and implementing dummy display and UI emulator [#97](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/97). With this emulator everyone will be able to develop UI features without a physical FLipper!
2. We continue to work on FURI API design and implementations [#59](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/59). If you have proposals or remarks about this component, or you don't understand what we are doing -- read [FURI and FURI AC description](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/FURI), look at the [examples](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Application-examples) and feel free to comment this issue or discuss it in [discord](https://flipperzero.one/discord).
3. We started a big work of dynamic loading and linking applications. Flipper is different from many embedded systems because we want to run user applications, load it by USB, Bluetooth, SD-card and other ways, so we need to implement it on a small limited system without MMU. You can see progress and discuss it here [#73](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/73)
4. We got an interesting proposal about Zephyr OS [comment in #17](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/17#issuecomment-683929900) and porting it on our new WB55 board [№89](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/89).
5. Working on new Flipper's PCB design based on STM32WB55RB MCU and new PMIC (we're using AXP173).
6. Creating a unit test environment and pipelines for CI [#40](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/40). if you want to see how building and testing is working right now, check out [Environment](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Environment) page.
7. Very soon we will have a remote testing and debugging bench! I think it's a very funny idea! It will be useful for developers who haven't real hardware and also for running CI on physical hardware: [#96](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/96)
8. In the next week we will start to blow off magic smoke and breathe life in new F2B0C1.1 boards, stay tuned!

# We need help

1. Linting and control code style [#12](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/12) is stuck
2. We have a big discussion about integration with IDE. If you feel pain with our current development environment and want to use your favorite IDE, welcome to [#18](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/18)!
3. Please check out and discuss the idea of attaching issues to wiki pages: [#66](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/66)
4. We want to make a web interface for UI emulator ([#97](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/97)) and looking for people who want to design the webpage. React/TS is preferred.

# Our plans

First of all, we need to completely set up our environment for building, testing and debugging code:

1. Make UI emulator
2. Make remote debug/test bench
3. Test F2B0C1.1 and send it to contributors
4. Make automatic code style checking, unit testing and overall CI.

Then, we need to make a build system (including dynamic linking specificity), toolset for loading apps on Flipper and run hardware tests, IDE integrations.

And we should concentrate on core API and architecture: improving FURI features, making examples and porting old Flipper's prototype code to check that our API is usable. Also I want to design core API so that changing HAL/OS will not very painful for app developers.

After we make UI emulator and deliver real hardware to UI developers we can start UI architecture: interface guidelines, GUI toolkit.

Also you can analyse features right now, design and propose how Flipper's user features can work. It also helps us to design core API and requirement for core, test bench and build system.

--  
Best,  
Andrew Strokov @glitchcore 
