# DSB Clone
This is a replacement board for the SEGA Digital Sound Boards 1 and 2 used on several SEGA arcade games.

![Assembled DSBClone board](Docs/assets/assembled-board.jpg)


# Historical background

The SEGA Digital Sound Boards 1 and 2 are dedicated to playing music (the BGMs) on several SEGA games. The music is stored in maskroms specific to each game. It is a card separate from the main stack of the game, sometimes located in its own cage.

They bear the following references: *837-10084 DIGITAL AUDIO BD SEGA 1993, 171-6614B PC BD, Sticker: 837-12941* and *837-12273 DIGITAL SOUND BD 2 SEGA 1995, 171-7165D PC BD, Sticker: 837-12273-92, Sticker: 837-13376*. These original cards are not very reliable and become difficult to find at a reasonable price.


They were used on the following games:

* First version (DSB1):
  * Daytona USA
  * Desert Tank
  * Manx TT Deluxe
  * Scud Race / Super GT (Model 3 Step 1.5)
  * Sega Rally Deluxe
  * Sega Touring Cars (Model 2C CRX)
  * Star Wars Arcade (Model 1)
  * Virtua Cop
  * Virtua Fighter
  * Virtua Racing
  * Wing Wars

*Please note that Daytona USA, Virtua Racing, Virtua Fighter, Wing War & Star Wars Arcade use DSB1 with an addon board in charge of playing sounds. The DSBclone doesn't handle this. As a result I suggest you avoid clone for such games and **DSB1 support is disabled by default in source code**.*

* Second version (DSB2):
  * Daytona 2
  * Lost World
  * Sega Rally 2
  * Spikeout
  * Spikeout FE
  * Star Wars Trilogy
  * Top Skater


# Features

* Playback of MP3 and WAV files stored on a microSD card
* Plug'n'Play: no modification of the wiring
* Track listing can be modified simply by renumbering the files
* Output volume identical to the original DSB
* Reprogrammable firmware: based on an Arduino Nano
* 24-bit DAC, DR 90dB, SNR 85 dB
* Protection by fuse and polarizers on the connectors
* 2 audio outputs with "JST SEGA" connector


In comparison with the original card:
* There is a slight latency (a few milliseconds) when starting the music. This can be understood if you pay attention with a twin that contains a DSB Sega and a DSB Clone. We can play on the MP3 to reset everything, or simply put two clones.
* The sound quality is much better: high quality MP3 / WAV replace the original 112kbits MPEG2.
* Possibility to customize the game's BGMs.


# Build 

## PCB

You can build the PCB with PCBway services. Upload the `DSBclone.kicad_pcb.zip` file to the "Quick order" tool and keep the default settings. That's all !

It can be even easier: use the shared project: https://www.pcbway.com/project/shareproject/DSB_Clone_replacement_for_SEGA_Digital_Sound_Boards_1_and_2_b2268928.html 

## BOM:
* U1: HCPL-0600 SOIC-8 optocoupler
* D1, D4: any diode that can handle 1A (T7 is ok)
* D2: 0805 led
* D3: 0805 1N4148, used to drop down DfPlayer voltage
* R1,R3,R5: 0805 1k resistor
* R2,R4: 0805 470o resistor
* F1: 0805 1A fuse
* DfPlayer: get the original one from reliable source (Mouser, Farnell...) and not an AliExpress clone.
* Arduino Nano: a 328p is ok.
* CN5: IDC 2x5 vertical connector
* CN6,7: JST B5P SH
* CN1: JST B6P SH 
* SPK, DBG, MidiRX: debug header, shouldn't be populated unless you want to develop

# Updating from release

Update procedure can be found in [Docs/Update.md](Docs/Update.md).

# Releases

Public releases can be found in the [Releases](Releases/) folder. Update procedure can be found in [Docs/Update.md](Docs/Update.md) documentation.

The "-debug.hex" releases have debugging enabled. You can hook a RS232 adapter to the debug pin on the board (default 115200bps, 8 data bits, 0 parity bit).

Public releases:

* 2021-08-29: 
  * Fix a bug on fadeout. 
  * Improvements on latency by filtering spam sent by games to the player.
* 2022-04-02: 
  * Change fadeout behavior; sound volume is now restored after fade.
  * Latency of MP3 player is tested on start; this improves latency.
* 2022-04-03:
  * Fix intro sound not being played on some MP3 players.
  * Fix advertisement BGM on Scud Race.
* 2022-04-05:
  * Fix a case where sound effects would affect BGM.
* 2022-04-06:
  * Reverted advertisement BGM played by DSB on Scud Race.
  * Track looping now working on Scud Race.
* 2022-04-16:
  * Track loop bugfix for Scud Race.
* 2022-12-04:
  * Removed support of DSB1 command set as they conflict with some DSB2 games.
* 2024-01-04:
  * Improvements on calibration for clone/original MP3 module. Releases available for DSB2, DSB1 & SpikeOut.
* 2024-08-24:
  * A special source code is proposed for SpikeOut FE. It compress sound volume to be closer to the original game.


# Installation

1. Prepare the SD card.

Any microSD card will do: between 60 and 150MB of space is sufficient. If you have an old 128 or 256MB card that's fine.

The audio files are to be placed in the "MP3" directory located at the root. Each file is preceded by 4 digits corresponding to the track number.
Bonus: the "1000_xxx.mp3" file is automatically read when the card is started. A cool way to have a jingle, music on hold, etc. Personally I put the "SEGAAAA" jingle 


![Track list example: SCUD Race](Docs/assets/tracklist-scud.jpg) ![Track list example: SEGA Rally 2](Docs/assets/tracklist-sr2.jpg)

Several packs corresponding to the most common games are available. Here are those currently known:
* Sega Rally 2, CD quality from OST: https://segaretro.org/Sega_Rally_2_(album)
* Scud Race, CD quality from OST: https://segaretro.org/Scud_Race_Sound_Tracks
* Daytona2, CD quality from OST: https://segaretro.org/Daytona_USA_2_Sound_Tracks
* VirtuaRacing, dump arcade from Khinsider VGM
* In progress: Virtua Fighter, CD quality from OST: https://segaretro.org/Virtua_Fighter_&_Virtua_Fighter_2_Music_Tracks
* Numbers: all numbers from 1 to 100. This allows you to know which track is requested by the game without needing a computer.

As they are copyrighted materials they are not shared.

2. Connect to the arcade cabinet

It's plug'n'play without modifying the wiring:
* the black 10 pin socket for the power supply
* the 6-pin white socket for MIDI
* the white 5-pin socket(s) for the audio outputs

![Top view wiring](Docs/assets/top-view-wiring.jpg)



# Ready to use board

You can buy a ready to use board on arcade forums like [Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?f=9&t=75510) (French) or [Arcade Projects](https://www.arcade-projects.com/threads/dsb-clone-digital-sound-board-sega.17832/) (English).
