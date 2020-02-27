# TriPlayer

TriPlayer is a 'system-wide' MP3 player for the Nintendo Switch. It requires a console capable of running Atmosphere. The name comes from it being consisted of **three** separate 'modules', and its ability to play MP**3** files.

[Download](https://github.com/tallbl0nde/TriPlayer/releases)

Curious about what's next? See my to-do list on [Trello](https://trello.com/b/teZpHfo1/triplayer)

## Contents

1. [Installation](#installation)
2. [Features](#features)
3. [Screenshots](#screenshots)
4. [How It Works](#how-it-works)
5. [Requirements](#requirements)
6. [Known Issues](#known-issues)
7. [Credits](#credits)
8. [Support](#support-3)

## Installation

To be determined...

## Features

To be determined...

## Screenshots

To be determined...

## How It Works

TriPlayer consists of three 'modules'/components. These are:

1. **sys-triplayer (Sysmodule)** [required]
    * This is the sysmodule which is run at boot and stays in the background. It listens for commands from the application/overlay in order to play/skip/etc. It handles all MP3 decoding/playing and so on. Thus it is required for TriPlayer to work whatsoever.
2. **Homebrew Application** [required]
    * This is the application launched via hbmenu in order to play/queue songs. It communicates with sys-triplayer in order to do so. This is also required in order to actually play any tracks.
3. **ovl-triplayer (Tesla Overlay)** [optional]
    * The overlay provides a quick and easy way to adjust playback. It is completely optional, however without it any playback adjustments will need to be made by suspending the current game and navigating to hbmenu.

## Requirements

* At the time of writing, you will require a Switch running Atmosphere 0.10.0+ as it has a lot more memory available to sysmodules compared to other CFW's (TriPlayer requires a lot).

## Known Issues

* Switch freezes on boot after installing TriPlayer
  * This occurs when there is not enough memory available. Please disable/delete other sysmodules that you have running.
  * Note that there is no way around this memory constraint.

## Credits

I'd like to thank:

* Exelix11 for [SysDVR](https://github.com/exelix11/SysDVR)
  * Used as a reference for implementing sockets/communication between app/overlay and sysmodule!
* KranKRival for [sys-audioplayer](https://github.com/KranKRival/sys-audioplayer)
  * Used as a reference for implementing mpg123 functionality
* Anyone else involved with the development of homebrew tools and reverse engineering of the Switch!

## Support <3

There is absolutely no obligation, however if you have found this software useful you can support me on Ko-fi!

[![ko-fi](https://www.ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/J3J718RRQ)

Knowing my software is being used is enough to motivate me to continue work on it!
