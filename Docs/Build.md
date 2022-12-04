# Build from sources

## Hardware
PCB details not public yet but please note that all official DSBclone have been distributed with an Arduino Nano equipped with ATmega328 and MiniCore bootloader.

## Software requirements

* Arduino IDE: it has been successfully tested with 1.8/2.0
* MiniCore bootloader recommended. Board config is located here: https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json and full instructions here: https://github.com/MCUdude/MiniCore
* Some third parties libraries are used and may need to be installed in "Tools/Manage Libraries"
    * https://github.com/PaulStoffregen/AltSoftSerial
    * https://github.com/NitrofMtl/TimeOut
    * https://github.com/FortySevenEffects/arduino_midi_library (1) see note
    * https://github.com/Makuna/DFMiniMp3 (1) see note

(1) MIDI and DFminiMP3 libraries have been modified to workaround custom MIDI protocol by SEGA and slow unofficial MP3 modules. A modified version is included in the project.

## Board configuration

1. In Arduino IDE, menu Tools/Board, select "MiniCore / ATmega328".
2. All other settings are default.

## Uploading

Compilation and uploading are done as usual. You refer to official Arduino IDE documentation: https://support.arduino.cc/hc/en-us/articles/4733418441116-Upload-a-sketch-in-Arduino-IDE
