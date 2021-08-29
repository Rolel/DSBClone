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

* Second version (DSB2):
  * Daytona 2
  * Lost World Special
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

Hardware, Software and Sound Pack creation details not public yet.


# Updating

Update procedure can be found in [Docs/Update.md](Docs/Update.md) documentation.

# Releases

Public releases can be found in the [Releases](Releases/) folder. Update procedure can be found in [Docs/Update.md](Docs/Update.md) documentation.

The "-debug.hex" releases have debugging enabled. You can hook a RS232 adapter to the debug pin on the board (default 115200bps, 8 data bits, 0 parity bit).

Public releases:

* 2021-08-29: Fix a bug on fadeout. Improvements on latency by filtering spam sent by games to the player.



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
