  /*
   Use:
   https://github.com/FortySevenEffects/arduino_midi_library
   https://github.com/Makuna/DFMiniMp3
   https://github.com/PaulStoffregen/AltSoftSerial
   https://github.com/NitrofMtl/TimeOut

  How to organize SD card:
      every files in MP3 folder
      file should be name XXXX-whatever.mp3 with XXXX being the track number with leading 0
      file 1000-whatever.mp3 is played on boot

  Note on MIDI Library:
      Midi protocol is not respected by VirtuaRacing DSB, as a result we need to "hack" the library.
      Correct NoteOff and ControlChange messages are 3 bytes MIDI, but only 2 with VirtuaRacing.
      MIDI.hpp file should be modified with this code at line 896:
        // HACK for Virtua Racing
        if (mPendingMessage[0] == 129 || mPendingMessage[0] == 176) {
                mPendingMessageExpectedLength = 2;
        }

        
  Note on DFMiniMp3 Library:
      By default library keep 50ms space between 2 commands.
      Clone chip may need larger delay, up to 150ms.
      Library has been modified to allow this delay to be set from code.
*/

#include "LibMod/DFMiniMp3.h"
#include "LibMod/MIDI/MIDI.h"
#include <TimeOut.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>



// Enable debug messages over serial
// #define DEBUG 

// Enable DSB1 support
#define ENABLE_DSB1 false

// Enable SpikeOut patches
#define ENABLE_SPIKEOUT true

// This is the minimal delay between 2 dfPlayer commands
#define DFPLAYER_SENDSPACE 30

// Init some global variable
byte soundVolume = 20; // initial volume of dfPlayer
byte soundTable[128] = {
  // Repeating each value 2 times from 0 to 5
  0, 0, // 0 = 0 => 1
  1, 1, // 1 = 2 => 3
  2, 2, // 2 = 4 => 5
  3, 3, // 3 = 6 => 7
  4, 4, // 4 = 8 => 9
  5, 5, // 5 = 10 => 11
  
  // Repeating each value 3 times from 6 to 11
  6, 6, 6, // 6 = 12 => 14
  7, 7, 7, // 7 = 15 => 17
  8, 8, 8, // 8 = 18 => 20
  9, 9, 9, // 9 = 21 => 23
  10, 10, 10, // 10 = 24 => 26
  11, 11, 11, // 11 = 27 => 29
  
  // Repeating each value 4 times from 12 to 17
  12, 12, 12, 12, // 12 = 30 => 33
  13, 13, 13, 13, // 13 = 34 => 37
  14, 14, 14, 14, // 14 = 38 => 41
  15, 15, 15, 15, // 15 = 42 => 45
  16, 16, 16, 16, // 16 = 46 => 49
  17, 17, 17, 17, // 17 = 50 => 53
  
  // Repeating each value 5 times from 18 to 25
  18, 18, 18, 18, 18, // 18 = 54 => 58
  19, 19, 19, 19, 19, // 19 = 59 => 63
  20, 20, 20, 20, 20, // 20 = 64 => 68
  21, 21, 21, 21, 21, // 21 = 69 => 73
  22, 22, 22, 22, 22, // 22 = 74 => 78
  23, 23, 23, 23, 23, // 23 = 79 => 83
  24, 24, 24, 24, 24, // 24 = 84 => 88
  25, 25, 25, 25, 25, // 25 = 89 => 93

  // Repeating each value 6 times from 26 onwards
  26, 26, 26, 26, 26, 26, // 26 = 94 => 99
  27, 27, 27, 27, 27, 27, // 27 = 100 => 105
  28, 28, 28, 28, 28, 28, // 28 = 106 => 111
  29, 29, 29, 29, 29, 29, // 29 = 112 => 117
  30, 30, 30, 30, 30, 30, // 30 = 118 => 123
  31, 31, 31, 31  // 31 = 124 => 127 (Note: Adjusted to fit within the limit)
};




uint16_t errorCounter = 0; // global counter used to track communication error with dfPlayer
byte lastCommand = 0; // last command sent to arduino, see LastCommandType enum
enum LastCommandType: byte
{
    Play     = 1,
    Stop     = 2,
    FadeOut  = 3,
    Reset    = 4,
    Volume   = 5,
};


// UARTs setup
struct DSB2Settings : public midi::DefaultSettings
{
  static const long BaudRate = 31250;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, DSB2Settings);


AltSoftSerial softSerial; // Used for MP3Player - RX8, TX9 (and 10 unusable)

#ifdef DEBUG
SoftwareSerial softSerialDebugger(11, 12); // Used for debugger
#endif

class Mp3Notify;
// typedef DFMiniMp3<AltSoftSerial, Mp3Notify, Mp3ChipMH2024K16SS> DfMp3;
typedef DFMiniMp3<AltSoftSerial, Mp3Notify, Mp3ChipOriginal> DfMp3;

class Mp3Notify
{
#ifdef DEBUG
  public:
    static void OnError(DfMp3& mp3, uint16_t errorCode) {
      errorCounter++;
      // see DfMp3_Error for code meaning
      switch (errorCode) {
        case 1:
          softSerialDebugger.println(F("DFPlayer: DfMp3_Error_Busy = 1 - (see chip documentation) Usually the media was not found."));
          break;
        case 2:
          softSerialDebugger.println(F("DFPlayer: DfMp3_Error_Sleeping = 2 - (see chip documentation) The chip is in sleep mode."));
          break;
        case 3:
          // Serial error ? We increase delay between 2 commands
          mp3.increaseSendSpace();
          digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
          softSerialDebugger.println(F("DFPlayer: DfMp3_Error_softSerialDebuggerWrongStack = 3 - Serial communication error (see chip documentation)"));
          break;
        case 4:
          softSerialDebugger.println(F("DFPlayer: DfMp3_Error_CheckSumNotMatch = 4 - (see chip documentation) Communications error on the hardware level. Check wiring and GND connections."));
          break;
        case 5:
          softSerialDebugger.println(F("DFPlayer: DfMp3_Error_FileIndexOut = 5 - (see chip documentation) File index out of bounds."));
          break;
        case 6:
          softSerialDebugger.println(F("DFPlayer: DfMp3_Error_FileMismatch = 6 - (see chip documentation) Can not find file."));
          break;
        case 7:
          softSerialDebugger.println(F("DFPlayer: DfMp3_Error_Advertise = 7 - (see chip documentation) In advertisement."));
          break;
        default:
          Serial.print(F("DFPlayer: Unknown error code = "));
          Serial.println(errorCode);
          break;
      }
    }

    static void OnPlayFinished(DfMp3& mp3, DfMp3_PlaySources source, uint16_t track) {
      softSerialDebugger.print(F("DFPlayer: Play finished for #"));
      softSerialDebugger.println(track);
    }
    static void OnPlaySourceOnline(DfMp3& mp3, DfMp3_PlaySources source) {
      PrintlnSourceAction(source, "online");
    }
    static void OnPlaySourceInserted(DfMp3& mp3, DfMp3_PlaySources source) {
      PrintlnSourceAction(source, "inserted");
    }
    static void OnPlaySourceRemoved(DfMp3& mp3, DfMp3_PlaySources source) {
      PrintlnSourceAction(source, "removed");
    }
    static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
    {
      if (source & DfMp3_PlaySources_Sd) { softSerialDebugger.print(F("DFPlayer: SD Card, ")); }
      if (source & DfMp3_PlaySources_Usb) { softSerialDebugger.print(F("DFPlayer: USB Disk, ")); }
      if (source & DfMp3_PlaySources_Flash) { softSerialDebugger.print(F("DFPlayer: Flash, ")); }
      softSerialDebugger.println(action);
    }
#else

  public:
    static void OnError(DfMp3& mp3, uint16_t errorCode) {
      errorCounter++;
      if (errorCode == 3) {
        // Serial error ? We increase delay between 2 commands
        mp3.increaseSendSpace();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      }
    }
    static void OnPlayFinished(DfMp3& mp3, DfMp3_PlaySources source, uint16_t track) {}
    static void OnPlaySourceOnline(DfMp3& mp3, DfMp3_PlaySources source) {}
    static void OnPlaySourceInserted(DfMp3& mp3, DfMp3_PlaySources source) {}
    static void OnPlaySourceRemoved(DfMp3& mp3, DfMp3_PlaySources source) {}
#endif
};

DfMp3 dfPlayer(softSerial);

TimeOut repeatTimeout;

void setup() {
  int result, i;
  
  pinMode(LED_BUILTIN, OUTPUT);

  #ifdef DEBUG
    softSerialDebugger.begin(115200);
    softSerialDebugger.println(F("DSB Clone - debug interface started"));
    softSerialDebugger.println("Built on " __DATE__ " at " __TIME__);
    softSerialDebugger.println(F("STARTUP: Starting MP3 player"));
  #endif
  // Init the mp3 module with a delay between commands
  dfPlayer.begin();
  dfPlayer.setSendSpace(DFPLAYER_SENDSPACE);

  // Boot procedure for some picky module: reset, wait for SD
  dfPlayer.reset();
  waitMilliseconds(500);
  // Be silent and use SD
  dfPlayer.setVolume(0);
  dfPlayer.setPlaybackSource(DfMp3_PlaySource_Sd);
  i = 0;
  do{
      waitMilliseconds(100);
      result=dfPlayer.isOnline();
      i++;
  } while (result==false && i < 20);

  #ifdef DEBUG
    softSerialDebugger.println(F("STARTUP: Calibrate send space."));
  #endif
  calibrateSendSpace();
  
  #ifdef DEBUG
    softSerialDebugger.print(F("STARTUP: Setting volume to "));
    softSerialDebugger.println(soundVolume);
  #endif
  dfPlayer.setVolume(soundVolume);
  #ifdef DEBUG
    softSerialDebugger.println(F("STARTUP: Setting EQ"));
  #endif
  dfPlayer.setEq(DfMp3_Eq_Bass);
  waitMilliseconds(500);
   
  dfPlayer.playMp3FolderTrack(1000);
  waitMilliseconds(3000);
  #ifdef DEBUG
    softSerialDebugger.println(F("STARTUP: MP3 player started"));
  #endif

  /* Callbacks !
    https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-Callbacks
    MIDI.setHandleNoteOn(handleNoteOn);

    List of MIDI messages:
    https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message
  */
  MIDI.setHandleAfterTouchPoly(handleAfterTouchPoly);
  MIDI.setHandleSystemReset(handleSystemReset);
  if (ENABLE_DSB1) {
    MIDI.setHandleControlChange(handleControlChange);
    MIDI.setHandleNoteOff(handleNoteOff);
  }
  #ifdef DEBUG
    softSerialDebugger.println(F("STARTUP: Starting MIDI interface"));
  #endif
  // Listen to every MIDI channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  
  #ifdef DEBUG
    softSerialDebugger.println(F("STARTUP: MIDI interface listening"));
    softSerialDebugger.println(F("STARTUP: System ready"));
  #endif
}

void loop() {
  // Maybe not call it on every loop ?
  dfPlayer.loop();

  // Timeout handler monitor all callback
  TimeOut::handler();

  // Just read as fast as we can to get callback working
  if (MIDI.read()) {
    // Blink LED on every message
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}


/**
   Most message are Aftertouch Polyphonic, with one exception for the system reset
     DSB1 uses channels 15 (xAE/d174), 16 (xAF/d175) (Scud Race)
     DSB1 on virtua racing start messages with channel 17 (xB0/d176) (Virtua Racing)
     DSB2 uses channel 15 (xAE/d174)
   We'll simply ignore channel information and do not filter it.
*/
void handleAfterTouchPoly(byte channel, byte note, byte pressure) {
  /*
    Play Command: it starts a BGM.
     Note is a sound type: BGM, effect, voices...
     Pressure is the track.

    # About "channel"
    No game use channel 1-14; only 15 and 16 are used.
    Commands always begin with 174 (channel 15), 175 (channel 16) or 176 (another MIDI command).
    It's supposed 174 is play and 175 is loop play.

    # About "note"
    Most games use note x10/d16 for BGM, but some like ScudRace use x6F/d111.
    We need to filter because a lot of other note are sent for sound effect. Trying to start them would kill BGM.
    DFplayer handles up to 99 folders. It means using note as a folder name is not possible for ScudRace.

    # About "pressure"
    For every game pressure = 0 means STOP but for Scud Race there is a track: the advertisement BGM.
    As a result we "rewrite" value to 255 if game is Scud Race, because track 0 can't be played on DFplayer.
    
    # Example: on Sega Rally 2
    BGM music start at 16:1 and goes to 16:14 (also, there is no 4 in test menu)
    Playing BGM 0 is a way to stop current playing song
    Note from 17:* to 26:* are sound effects and voices
    
    # Example: on Scud Race
    BGM music start at 111:1 and goes to 111:9
    Playing BGM 0 is a way to stop current playing song
  */
  if (channel < 15) {
    // Channel is not 15/16
    return;
  }
  
  if (note == 16 || note == 111) {    
    if (pressure > 0) {
      
      dfPlayer.playMp3FolderTrack(pressure);      
      lastCommand = LastCommandType::Play;

      // Activate repeat if channel 16
      if (channel == 16 || ENABLE_SPIKEOUT) {
        // a delayed setRepeatPlayCurrentTrack
        repeatTimeout.timeOut(500, setRepeat, TIMEOUT::UNDELETABLE);
        // dfPlayer.setRepeatPlayCurrentTrack(true);
      }
      
      #ifdef DEBUG
        softSerialDebugger.print(F("CMD: BGM #"));
        softSerialDebugger.println(pressure);
      #endif
    } else {
      dfPlayer.stop();
      lastCommand = LastCommandType::Stop;
      #ifdef DEBUG
        softSerialDebugger.println(F("CMD: BGM #0 STOP"));
      #endif
    }
  }

  /*
    // On Sega Rally 2 commands from d17:* to d26:* are sound effects and voices
    if (note > 16 && note < 27 && pressure > 0) {
    #ifdef DEBUG
      softSerialDebugger.print(F("CMD: sound effect #"));
      softSerialDebugger.print(note);
      softSerialDebugger.print(F(" #"));
      softSerialDebugger.println(pressure);
    #endif
    }
  */


  /*
    Volume Command: set the sound volume

    Volume commands are the same for every DSB. It uses the note 01 or 02 and pressure is volume.

    On Sega Rally 2
     Every BGM play command is preceeded by a sound volume command
     Goes from 0 to 15 in service menu; from 0 to 90 in MIDI messages (14 and 15 are both 90)
     0:0, 1:12, 2:18, 3:24, 4:30, 5:36, 6:42, 7:48, 8:54, 9:60, 10:66, 11:72, 12:78, 13:84, 14:90, 15:90
     Volume on DFplayer goes from 0 to 30; but 21 already too much powerfull so limiting from 0 to 22 is ok => we divide command by 4.
     Why 4 ? Because it's easy for a CPU to divide by 4, it's only a bitshift
  */
  if (note == 1 || note == 2) {
    // DFPlayer handles volume values from 0 to 30 , we'll go up to 22.5 by dividing input command (90 max) by 4
    // TODO: replace division by a lookup table for speed ?
    // byte TMPsoundVolume = pressure >> 2;
    byte TMPsoundVolume = soundTable[pressure];

    #ifdef DEBUG
      softSerialDebugger.print(F("CMD: Volume change. Value: "));
      softSerialDebugger.print(pressure);
      softSerialDebugger.print(F(" => Game setting: "));
      softSerialDebugger.println(TMPsoundVolume);
    #endif
    
    // New volume value is compared with old one. If it's the same we avoid sending the command again.
    if (TMPsoundVolume != soundVolume) {
      dfPlayer.setVolume(TMPsoundVolume);
      soundVolume = TMPsoundVolume;
    } else {
      #ifdef DEBUG
        softSerialDebugger.println(F("INFO: Not sent to dfPlayer because volume was already the same"));
      #endif
    }
    
    lastCommand = LastCommandType::Volume;
  }

  /*
    Fadeout Command: stop a BGM with a fadeout.

    Most DSB use note 3, but some use note 4 to start a fadeout.
    It's not clear what the pressure do. Maybe a speed of fadeout ?

    DFplayer can't do fadeout but we can fake it by changing volume.

    Example: on Sega Rally 2 it happens after intro.
  */
  if (note == 3 || note == 4) {
    #ifdef DEBUG
      softSerialDebugger.print(F("CMD: Fadeout "));
      softSerialDebugger.println(pressure);
    #endif
    fadeOut(pressure >> 2);
    lastCommand = LastCommandType::FadeOut;
  }
}

/**
   Some system reset command are issued from time on time
   It uses the MIDI system reset command.
*/
void handleSystemReset(void) {
  #ifdef DEBUG
    softSerialDebugger.println(F("CMD: System reset command received"));
  #endif
  // We do not reset MP3 player as it "cracks" in speaker and cause 600ms lag, only stop playing
  dfPlayer.stop();
  lastCommand = LastCommandType::Reset;
}


/**
   The ControlChange is used by VirtuaFighter for Fadeout:
     status = xBE/d190 (channel 15)
     number = x18/d24
     value change according to speed = x20/d32,x30/d48,x40/d64,x50/d80

   This is also used by Virtua Racing but in a bugged version: the play sound action is a control change with only one data byte (instead of two)
     status = xB0/d176
     number = x00/d00 The track to play
     no value byte !
   This implie we need to hack MIDI library so statusbyte 176 is declared as two bytes instead of three
*/
void handleControlChange(byte channel, byte number, byte value) {
  // VirtuaFighter Fadeout
  // Statusbyte 190 = channel 15 (1-16)
  if (channel == 15 && number == 24) {
    #ifdef DEBUG
      softSerialDebugger.print(F("CMD: FadeOut VirtuaFighter "));
      softSerialDebugger.println(number);
    #endif
    fadeOut(value / 4);
    lastCommand = LastCommandType::FadeOut;
  }

  // Virtua Racing play
  // Statusbyte 176 = channel 1 (1-16)
  if (channel == 1) {
    // Custom hacked command for Virtua Racing
    dfPlayer.playMp3FolderTrack(number);
    lastCommand = LastCommandType::Play;
    #ifdef DEBUG
      softSerialDebugger.print(F("CMD: ControlChange VirtuaRacing: BGM #"));
      softSerialDebugger.println(number);
    #endif
  }
}

/**
   The NoteOff is used by Virtua Racing but in a bugged version: the stop sound action is a noteoff with only one data byte (instead of two)
     status = x81/d129
     number = x00/d00 always 0
     no value byte !
   This implie we need to hack MIDI library so statusbyte 129 is declared as two bytes instead of three
*/
void handleNoteOff(byte inChannel, byte inPitch, byte inVelocity) {
  if (inChannel == 2 && inPitch == 0) {
    dfPlayer.stop();
    lastCommand = LastCommandType::Stop;
    #ifdef DEBUG
      softSerialDebugger.println(F("CMD: NoteOff VirtuaRacing : STOP sound"));
    #endif
  }
}



/**
 * Do a fade out by changing volume to 0 then sending stop.
 * Volume is restored after
 * 
 * byte delay = delay in loop
*/
void fadeOut(byte delayValue) {

  // Some games are spamming fade out command, we skip next messages
  if (lastCommand == LastCommandType::FadeOut) {
  #ifdef DEBUG
    softSerialDebugger.print(F("Fading: previous cmd was already a fadeout, no fade"));
  #endif
    return;
  }

  #ifdef DEBUG
    softSerialDebugger.print(F("Fading:"));
  #endif
  // We use a local variable so we can restore sound to previous value.
  // Some games (like scud race) don't set sound volume on every play so we need to restore after a fadeout.
  byte fadeSoundVolume = soundVolume;

  // Reduce slowly to level 1
  while (fadeSoundVolume > 1) {
    fadeSoundVolume--;
    dfPlayer.setVolume(fadeSoundVolume);
    waitMilliseconds(delayValue);
    #ifdef DEBUG
      softSerialDebugger.print(F(" "));
      softSerialDebugger.print(fadeSoundVolume);
    #endif
  }

  // At the end we stop play, wait and restore volume
  dfPlayer.stop();
  waitMilliseconds(50);
  dfPlayer.setVolume(soundVolume);
  
  #ifdef DEBUG
    softSerialDebugger.println(F(" end fadeout"));
  #endif
}

/**
 * A simple callback function to repeat current track
 */
void setRepeat()
{
  dfPlayer.setRepeatPlayCurrentTrack(true);
}

/**
 * This increase communication delay with dfPlayer until we get no error
 */
void calibrateSendSpace() {
  byte okLoop = 0;
  uint16_t status = 0;
  
  do {
    // Do the test: stop+start
    dfPlayer.stop();
    dfPlayer.playRandomTrackFromAll();
    // getStatus doesn't work on MH2024K24SS :/
    status = dfPlayer.getStatus();
    // Should be playing music without error
    if (status == 0 || status == 512 || status == 529) {
      // Increase space between 2 commands and reset loop counter
      dfPlayer.increaseSendSpace();
      okLoop = 0;
    } else {
      okLoop++;
    } 
    
  #ifdef DEBUG
    softSerialDebugger.print(F("Calibration (status/errors/ok): "));
    softSerialDebugger.printf("%d status / %d errors / %d OKloop.\n\r", status, errorCounter, okLoop);
  #endif
  // 10 consecutive success ? We are calibrated !
  } while(okLoop < 10);
  // A little bit more to be secure
  dfPlayer.increaseSendSpace();
  // Don't forget to stop at the end
  dfPlayer.stop();
  #ifdef DEBUG
    softSerialDebugger.printf("Final value: %d ms.\n\r", dfPlayer.getSendSpace());
  #endif
}


/**
 * A blocking wait function
 */
void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    dfPlayer.loop(); 
    delay(1);
  }
}
