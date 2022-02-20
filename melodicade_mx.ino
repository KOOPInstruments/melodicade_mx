const char version[] = "Build: 22-02-20 16:52";
byte deckType = 1; // Deck type: 0 = No velocity detection, 1 = Dual switch velocity detection

// To do list
// Code:  If pitch bend button is pressed with no note button currently engaged, immediately snap to full amount to easier facilitite pre-bends
// Code:  Look into creating a scrolling menu so that we can add more options like:
//          Knob mode selection
//          Velocity curve scaling
//          Bend speed/Mod speed (pitchSpeed/modSpeed)
//          Split mode splitKey number
//          Global reference tuning pitch
//          AutoSus timeout
//          Program maps and instrument names selectiom

// Program loop timer
// Keep loop times as low as possible for better velocity detection.
// Even 10µs per node * 120 nodes per deck = 1.2ms top to bottom.  Time difference
// between top/bottom buttons can be 1ms or lower depending on how hard they are hit.
// The longer it takes, the less dynamic range there is available at the high end.
// unsigned long programLoopTimer;


//----------------------------------------------------------------------------//
//                        -----  Melodicade MX  -----                         //
//          A portable 6+ octave, velocity sensitive MIDI keyboard            //
//          using Cherry MX compatible keyswitches arranged in the            //
//                   Wicki-Hayden isomprphic button layout.                   //
//                                                                            //
//                    Copyright (C) 2022 - Michael Koopman                    //
//                KOOP Instruments (koopinstruments@gmail.com)                //
//      https://www.koopinstruments.com/instrument-projects/melodicade_mx     //
//----------------------------------------------------------------------------//
//                                                                            //
//   This program is free software: you can redistribute it and/or modify     //
//   it under the terms of the GNU General Public License as published by     //
//   the Free Software Foundation, either version 3 of the License, or        //
//   (at your option) any later version.                                      //
//                                                                            //
//   This program is distributed in the hope that it will be useful,          //
//   but WITHOUT ANY WARRANTY; without even the implied warranty of           //
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
//   GNU General Public License for more details.                             //
//                                                                            //
//   You should have received a copy of the GNU General Public License        //
//   along with this program.  If not, see <https://www.gnu.org/licenses/>.   //
//                                                                            //
//----------------------------------------------------------------------------//
//
// Microcontroller Information:
// Teensy 4.1 at 600MHz with USB type MIDI
// I/O requirments:
//     44 x Digital
//      4 x Analog
//      2 x I2C
//      1 x Serial TX
//      1 x MQS
// Additional requirements:
//      1 x +3.3V for foot pedal, DIN MIDI, 10K Ohm pots, SSD1306 OLED, and KY-040 rotary encoder
//      1 x GND for DIN MIDI, 10K Ohm pots, SSD1306 OLED, synth audio out, and KY-040 rotary encoder
//
// Hardware configuration:
// 120 x Gateron Black mechanical keyswitches (matrix of 10 columns * 12 shared rows)
// 120 x LL1105AF065Q Low activation force tact switches (matrix of 10 columns * 12 shared rows)
//   1 x SSD1306 OLED display on I2C bus
//   1 x KY-040 rotary encoder with pushbutton for menu interaction
//   2 x 10K Ohm potentiometers for analog adjustment of velocity and MIDI CC volume
//   1 x USB-B panel mount jack for power and USB MIDI
//   1 x 1/4" TS panel mount jack for foot pedal
//   1 x 3.5mm TRRS panel mount jack for synth audio out
//   1 x 5-pin DIN female serial MIDI jack
//
// Device pinout:
// Name                         Teensy 4.1 Pin
//--------------------------------------------
// (tact-switch columns)
// Col 0                        0
// Col 1                        2
// Col 2                        3
// Col 3                        4
// Col 4                        5
// Col 5                        6
// Col 6                        7
// Col 7                        8
// Col 8                        9
// Col 9                        10
// (key-switch columns)
// Col 0                        11
// Col 1                        24
// Col 2                        25
// Col 3                        26
// Col 4                        27
// Col 5                        28
// Col 6                        29
// Col 7                        30
// Col 8                        31
// Col 9                        32
// (shared rows)
// Row 0                        16
// Row 1                        15
// Row 2                        14
// Row 3                        41
// Row 4                        40
// Row 5                        39
// Row 6                        38
// Row 7                        37
// Row 8                        36
// Row 9                        35
// Row 10                       34
// Row 11                       33
// --
// Rotary encoder SW            17
// Rotary encoder DT            23(A9)
// Rotary encoder CLK           22(A8)
// --
// OLED I2C Clock               19(SCL)
// OLED I2C Data                18(SDA)
// --
// Pot (top)                    21(A7)
// Pot (bottom)                 20(A6)
// --
// 1/4" TS Jack (tip)           13
// 3.5mm TRS (tip+ring)         12(MQSL)
// --
// DIN MIDI TX                  1
//
//
// Keyswitch/tactSwitch buttonNumber variable locations
// Square bracket indicates control buttons
// --------------------------------------------------
// | [000]─── 001 002 003 004 005 006 007 008 009   |
// | [020]  010 011 012 013 014 015 016 017 018 019 |
// |   └───── 021 022 023 024 025 026 027 028 029   |
// |        030 031 032 033 034 035 036 037 038 039 |
// |   ┌───── 041 042 043 044 045 046 047 048 049   |
// | [040]  050 051 052 053 054 055 056 057 058 059 |
// | [060]─── 061 062 063 064 065 066 067 068 069   |
// | [080]  070 071 072 073 074 075 076 077 078 079 |
// |   └───── 081 082 083 084 085 086 087 088 089   |
// |        090 091 092 093 094 095 096 097 098 099 |
// |   ┌───── 101 102 103 104 105 106 107 108 109   |
// | [100]  110 111 112 113 114 115 116 117 118 119 |
// --------------------------------------------------
//
// Control panel button functions:
//     MIDI Channel Up
//     MIDI Channel Down
//     --
//     Pitch bend up    (release - 8192 / half pressure - 12287  / full pressure - 16383)
//     Modulation on    (release - 0 / half pressure - 63     / full pressure - 127)
//     Pitch bend down  (release - 8192 / half pressure - 4097   / full pressure - 0)
//     --
//     Looper:
//       1st press - On & primed for input.
//       2nd press - If no note input: deactivate looper.  If yes note input: save loop and start overdub playback.
//       3rd+ press - Toggle overdub recording off/on.  If current channel is active when toggled on, clear indexes for new overdub.
//       Hold > 750ms - Deactivate looper.
//
// Foot pedal:
//     Selectable functions:
//       Sustain (default)
//       Looper
//       Modulation


//------------------------------------------------------------------------------------------------------------------------------------------------------------//
// START OF PROGRAM

// Required libraries
#include "MIDIUSB.h"                                                            // MIDIUSB v1.0.5 library by Gary Grewal
#include "Wire.h"                                                               // Wire library (Included with Arduino IDE)
#include "SSD1306Ascii.h"                                                       // SSD1306Ascii v1.3.0 library by Bill Greiman
#include "SSD1306AsciiWire.h"                                                   // SSD1306Ascii v1.3.0 library by Bill Greiman
#include "RotaryEncoder.h"                                                      // RotaryEncoder v1.5.0 library by Matthais Hertel
#include "Audio.h"                                                              // Teensy Audio library by Paul Stoffregen

// Teensy hardware restart variables for use in controllerReset() function
#define RESTART_ADDR 0xE000ED0C
#define READ_RESTART() (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))

// OLED I2C address and library configuration
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

// Analog input pins
const byte topPotPin            = A7;
const byte bottomPotPin         = A6;

// Rotary encoder pins and library configuration
RotaryEncoder encoder(A9, A8, RotaryEncoder::LatchMode::FOUR3);

// Digital input pins
const byte rotaryEncoderButtonPin = 17;
const byte footPedalPin           = 13;

// Button row pins
const byte rowPins[] = {16, 15, 14, 41, 40, 39, 38, 37, 36, 35, 34, 33};        // Row pins in order from top to bottom
const byte rowCount = sizeof(rowPins);                                          // The number of rows in the matrix

// Keyswitch column pins
const byte keyswitchColumnPins[] = {11, 24, 25, 26, 27, 28, 29, 30, 31, 32};    // Column pins in order from left to right (top perspective)
const byte columnCount = sizeof(keyswitchColumnPins);                           // The number of columns in the matrix

// Tact switch column pins
const byte tactSwitchColumnPins[] = {0, 2, 3, 4, 5, 6, 7, 8, 9, 10};            // Column pins in order from left to right (top perspective)

byte deckScanToggle;                                                            // Variable for bouncing between decks when scanning

// Variables for matrix scanning
const byte elementCount = columnCount * rowCount;                               // The total number of elements in the matrix
byte columnIndex;                                                               // Current column index for scanning
byte currentColumn = keyswitchColumnPins[columnIndex];                          // Column being scanned
byte rowIndex;                                                                  // Current row index for scanning
byte currentRow = rowPins[rowIndex];                                            // Row being scanned

// MIDI channel map (pushing percussion channel 9 to the end to simplify coding)
const byte channelMap[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 9};
int channelIndex;                                                               // Index used with channel map
byte channelResetToggle;                                                        // If set HIGH, reset MIDI channel adjustments to default on next program loop

// Instrument selection
unsigned int    instrumentIndex[16];                                            // Instrument selection per channel (0-15)
byte            synthInstrumentIndex[16];                                       // Synth instrument selection per "channel" (0-15)

// Time, debounce and rate limits for analog MIDI CC updates.  Needed to prevent buffer overrun on some devices.
const byte      debounceTime             = 50;                                  // Digital button debounce time (milliseconds)
const byte      rotaryButtonDebounceTime = 200;                                 // Rotary encoder button requires additional debounce time (milliseconds)
const byte      analogDeadzone           = 4;                                   // The amount an analog knob reading needs to change before the variable is updated
unsigned long   keyswitchDebounceTimer[elementCount];
unsigned long   tactSwitchDebounceTimer[elementCount];
unsigned long   rotaryEncoderButtonDebounceTimer;

// Rate limiting
const byte      rateLimiterTime = 25;                                           // 25ms = 40Hz
unsigned long   lastRateLimiterTimestamp;                                       // Tracking last program loop time (millisconds) to limit resource intensive processes running unnecessarily
byte            rateLimiterToggle;                                              // When this toggles HIGH, run rate limited processes
unsigned long   lastHardwareTestPrintTime;                                      // Rate limit printing of hardwareTest() info to serial monitor

// Digital button states and activation times
byte            activeKeyswitches[elementCount];
byte            activeTactSwitches[elementCount];
byte            footPedalButton;
byte            previousFootPedalButton;
byte            rotaryEncoderButton;
byte            previousRotaryEncoderButton;

// Active notes and velocity values
byte            activeNotes[elementCount];
byte            previousActiveNotes[elementCount];
unsigned long   keyswitchActivationTime[elementCount];
unsigned long   tactSwitchActivationTime[elementCount];
byte            noteVelocity[elementCount];

// Control button states
byte channelUpButton;
byte previousChannelUpButton;
byte channelDownButton;
byte previousChannelDownButton;
byte pitchUpHalfButton;
byte pitchUpFullButton;
byte modulationHalfButton;
byte modulationFullButton;
byte pitchDownHalfButton;
byte pitchDownFullButton;
byte loopButton;
byte previousLoopButton;

// Rotary encoder
signed int      rotaryEncoderPosition;                                          // Encoder position (+1 on clockwise detent, -1 on anticlockwise detent)
signed int      newPos;
byte            oledMenuToggle;                                                 // Toggle selection cursor from "option" heading (LOW) to "value" heading (HIGH)
byte            oledOption = 2;                                                 // Default cursor menu option (matches OLED row values - range 2-5)

// Device mode selection variable
byte modeSelection;
const byte NORMAL   = 0;
const byte LAYER    = 1;
const byte SPLIT    = 2;
const byte AUTOSUS  = 3;
const byte SYNTH    = 4;

// Split mode splits the deck starting at this MIDI pitch value and up, or below
const byte splitKey = 60;

// OLED
unsigned long oledScreensaverTime;                                              // Hold last input time in a variable for countdown clock
byte oledOnOff  = HIGH;                                                         // Default screen state toggle to ON
byte oledInit   = HIGH;                                                         // Default init toggle to ON

// Analog inputs
byte    topPotValue;
byte    previousTopPotValue;
byte    bottomPotValue;
byte    previousBottomPotValue;

// Note transposition
signed int transposeValue[16];                                                  // Array per channel - Max range -24 to +21 to stay within bounds of pitch values 0-127 using the Wicki-Hayden layout defined above

// Pedal
byte pedalFunction;
const byte SUSTAIN      = 0;
const byte LOOPER       = 1;
const byte MODULATION   = 2;

// Velocity (7-bit message with total range of 0-127)
byte potVelocity;                                                               // Velocity modifier tied to analog pot input
byte velocityDetectionEnabled;                                                  // Toggle for turning velocity detection on and off as needed
unsigned long   velocityDisableClock = 4294967295;

// Pitch Bend (14-bit message with total range of 0-16383)
int referencePitch   = 8192;                                                    // Default neutral pitch position; could exploit this value for global instrument tuning
int pitchOffset;                                                                // Offset amount for pitch bending.  pitchSpeed is added to this each rateLimiterToggle iteration to a max value of 8191
const int pitchSpeed = 2048;                                                    // The amount to increment/decrement pitchOffset each rateLimiterToggle iteration (must cleanly divide 8192)

// Modulation (7-bit message with total range of 0-127)
byte modOffset;                                                                 // Offset amount for modulation.  modSpeed is added to this each rateLimiterToggle iteration to a max value of 127
const int modSpeed = 16;                                                        // The amount to increment/decrement modOffset each rateLimiterToggle iteration (must cleanly divide 128)

// AutoSus timer
unsigned long autoSusTimer;
byte autoSusStatus;
int autoSusTimeout = 2000;                                                      // How long to hold AutoSus sustain before releasing in ms

// Looper
const int loopMaxChannels = 16;                                                 // Number of MIDI channels available to the looper for overdub recording
const int loopMaxIndexes = 1792;                                                // Number of note packets that can be recorded per channel
byte loopRecordingToggle;                                                       // Toggle to engage/disengage loop recording
unsigned long loopDisableClock = 4294967295;                                    // Hold looper button to start timer to disable looper. Default to max value of an unsigned long (49.7 days of milliseconds).
byte loopRecordingEnabled;                                                      // Inform the noteOn/noteOff/controlChange/pitchbendChange functions to record input or not
byte loopWaitingForInput;                                                       // Hold looper start time until the moment a note is input
byte loopInputDetected;                                                         // Inform looper that a note was input (located in digitalButtons() function)
byte loopInMemory;                                                              // Inform looper that note data has been recorded to trigger playback when needed
byte loopPlaybackEnabled;                                                       // Inform looper that it should continue reading out playback indexes on a loop
byte loopTrackEnabled[loopMaxChannels];                                         // Inform looper that a recording is active on this channel for overdub functionality
unsigned long loopStartTimestamp;                                               // Record start timestamp to calculate relative offsets for note input
unsigned long loopDuration;                                                     // Store loop duration at recording end to calculate playback loop
int loopRecordingIndex[loopMaxChannels];                                        // Current recording index per available channel
int loopPlaybackIndex[loopMaxChannels];                                         // Current playback index per available channel
unsigned long loopPacketTime[loopMaxChannels][loopMaxIndexes];                  // Recorded event time in relation to loop start
byte loopPacketType[loopMaxChannels][loopMaxIndexes];                           // Recorded event type (1 = noteOn, 2 = noteOff, 3 = controlChange, 4 = pitchBendChange)
byte loopPacketByte0[loopMaxChannels][loopMaxIndexes];                          // Recorded 1st data byte (note pitch/controlChange type/pitchBend highByte)
byte loopPacketByte1[loopMaxChannels][loopMaxIndexes];                          // Recorded 2nd data byte (note velocity/controlChange value/pitchBend lowByte)
byte loopPercentage;                                                            // Current playback percentage for OLED display
byte previousLoopPercentage;                                                    // Comparison variable to prevent unnecessary time consuming OLED updates
byte noteTracking[16][127];                                                     // Track noteOn packets that have been sent to the bus to ensure closing noteOffs are also sent on playback interruption


// Default MIDI pitch value assignment
const byte wickiHaydenLayout[elementCount] = {
00,       90,  92,  94,  96,  98, 100, 102, 104, 106,                           // '00' indicates unused node
       83,  85,  87,  89,  91,  93,  95,  97,  99, 101,
00,       78,  80,  82,  84,  86,  88,  90,  92,  94,
       71,  73,  75,  77,  79,  81,  83,  85,  87,  89,
00,       66,  68,  70,  72,  74,  76,  78,  80,  82,
       59,  61,  63,  65,  67,  69,  71,  73,  75,  77,
00,       54,  56,  58,  60,  62,  64,  66,  68,  70,
       47,  49,  51,  53,  55,  57,  59,  61,  63,  65,
00,       42,  44,  46,  48,  50,  52,  54,  56,  58,
       35,  37,  39,  41,  43,  45,  47,  49,  51,  53,
00,       30,  32,  34,  36,  38,  40,  42,  44,  46,
       23,  25,  27,  29,  31,  33,  35,  37,  39,  41
};

const byte drumLayout[elementCount] = {
00,       00,  00,  00,  00,  00,  00,  00,  00,  00,
       00,  00,  00,  00,  00,  00,  00,  00,  00,  00,
00,       00,  00,  00,  00,  28,  00,  00,  00,  00,
       00,  25,  26,  29,  30,  86,  87,  78,  79,  00,
00,       73,  74,  75,  82,  27,  83,  84,  71,  72,
       00,  77,  76,  62,  64,  63,  85,  68,  67,  00,
00,       66,  65,  41,  47,  48,  50,  57,  80,  81,
       00,  61,  60,  43,  45,  48,  49,  52,  69,  00,
00,       70,  58,  42,  46,  46,  42,  51,  59,  54,
       25,  39,  40,  37,  38,  38,  37,  40,  31,  25,
00,       44,  35,  55,  36,  35,  36,  53,  35,  56,
       00,  00,  00,  00,  00,  00,  00,  00,  00,  00
};


// GM GS/GM2/XG bank maps, program maps and instrument names (instrument name strings must be 21 characters to ensure the entire line is cleared on oledUpdate)
// There is a ton of variation between devices and SoundFonts, so here are a few of my favorites; customize to taste.

// General MIDI Level 1 (+ GM2 Percussion):
const byte drumKitMap[]   = {0, 8, 16, 24, 25, 32, 40, 48, 56};
const byte bankMap[]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const byte programMap[]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127};
const char *drumKitName[] = {"Standard Kit         ", "Room Kit             ", "Power Kit            ", "Electronic Kit       ", "TR-808 Kit           ", "Jazz Kit             ", "Brush Kit            ", "Orchestra Kit        ", "Sound FX Kit         "};
const char *programName[] = {"Acoustic Grand Piano ", "Bright Acc. Piano    ", "Electric Grand Piano ", "Honky-tonk Piano     ", "Rhodes Piano         ", "Chorused Elec. Piano ", "Harpsichord          ", "Clavinet             ", "Celesta              ", "Glockenspiel         ", "Music Box            ", "Vibraphone           ", "Marimba              ", "Xylophone            ", "Tubular Bells        ", "Dulcimer/Santur      ", "Hammond Organ        ", "Percussive Organ     ", "Rock Organ           ", "Church Organ         ", "Reed Organ           ", "French Accordion     ", "Harmonica            ", "Bandoneon            ", "Nylon-String Guitar  ", "Steel-String Guitar  ", "Jazz Guitar          ", "Clean Elec. Guitar   ", "Muted Elec. Guitar   ", "Overdriven Guitar    ", "Distortion Guitar    ", "Guitar Harmonics     ", "Acoustic Bass        ", "Fingered Bass        ", "Picked Bass          ", "Fretless Bass        ", "Slap Bass 1          ", "Slap Bass 2          ", "Synth Bass 1         ", "Synth Bass 2         ", "Violin               ", "Viola                ", "Cello                ", "Contrabass           ", "Tremolo Strings      ", "Pizzicato Strings    ", "Harp                 ", "Timpani              ", "String Ensemble      ", "Slow String Ensemble ", "Synth Strings 1      ", "Synth Strings 2      ", "Choir Aahs           ", "Voice Oohs           ", "Synth Voice          ", "Orchestra Hit        ", "Trumpet              ", "Trombone             ", "Tuba                 ", "Muted Trumpet        ", "French Horn          ", "Brass Section        ", "Synth Brass 1        ", "Synth Brass 2        ", "Soprano Sax          ", "Alto Sax             ", "Tenor Sax            ", "Baritone Sax         ", "Oboe                 ", "English Horn         ", "Bassoon              ", "Clarinet             ", "Piccolo              ", "Flute                ", "Recorder             ", "Pan Flute            ", "Blown Bottle         ", "Shakuhachi           ", "Whistle              ", "Ocarina              ", "Square Lead          ", "Saw Lead             ", "Synth Calliope       ", "Chiffer Lead         ", "Charang              ", "Solo Synth Vox       ", "5th Saw Wave         ", "Bass & Lead          ", "Fantasia Pad         ", "Warm Pad             ", "Polysynth Pad        ", "Space Voice Pad      ", "Bowed Glass Pad      ", "Metal Pad            ", "Halo Pad             ", "Sweep Pad            ", "Ice Rain             ", "Soundtrack           ", "Crystal              ", "Atmosphere           ", "Brightness           ", "Goblin               ", "Echo Drops           ", "Star Theme           ", "Sitar                ", "Banjo                ", "Shamisen             ", "Koto                 ", "Kalimba              ", "Bagpipe              ", "Fiddle               ", "Shanai               ", "Tinkle Bell          ", "Agogo                ", "Steel Drums          ", "Woodblock            ", "Taiko Drum           ", "Melodic Tom          ", "Synth Drum           ", "Reverse Cymbal       ", "Guitar Fret Noise    ", "Breath Noise         ", "Seashore             ", "Bird Tweet           ", "Telephone            ", "Helicopter           ", "Applause             ", "Gun Shot             "};

// General MIDI Level 2:
// const byte drumKitMap[]   = {0, 8, 16, 24, 25, 32, 40, 48, 56};
// const byte bankMap[]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 7, 8, 9};
// const byte programMap[]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 0, 1, 2, 3, 4, 5, 6, 7, 11, 12, 14, 16, 17, 19, 20, 21, 24, 25, 26, 27, 28, 29, 30, 31, 33, 38, 39, 40, 46, 48, 50, 52, 53, 54, 55, 56, 57, 59, 60, 61, 62, 63, 80, 81, 84, 87, 89, 91, 98, 102, 104, 107, 115, 116, 117, 118, 120, 121, 122, 123, 124, 125, 126, 127, 0, 4, 5, 6, 14, 16, 17, 19, 24, 25, 27, 28, 30, 38, 39, 48, 55, 57, 62, 63, 80, 81, 102, 118, 120, 122, 123, 124, 125, 126, 127, 4, 5, 6, 16, 24, 25, 28, 38, 39, 55, 62, 81, 122, 123, 124, 125, 126, 127, 5, 38, 81, 122, 124, 125, 126, 122, 124, 125, 126, 125, 125, 125, 125};
// const char *drumKitName[] = {"Standard Kit         ", "Room Kit             ", "Power Kit            ", "Electronic Kit       ", "TR-808 Kit           ", "Jazz Kit             ", "Brush Kit            ", "Orchestra Kit        ", "Sound FX Kit         "};
// const char *programName[] = {"Acoustic Grand Piano ", "Bright Acc. Piano    ", "Electric Grand Piano ", "Honky-tonk Piano     ", "Rhodes Piano         ", "Chorused Elec. Piano ", "Harpsichord          ", "Clavinet             ", "Celesta              ", "Glockenspiel         ", "Music Box            ", "Vibraphone           ", "Marimba              ", "Xylophone            ", "Tubular Bells        ", "Dulcimer/Santur      ", "Hammond Organ        ", "Percussive Organ     ", "Rock Organ           ", "Church Organ 1       ", "Reed Organ           ", "French Accordion     ", "Harmonica            ", "Bandoneon            ", "Nylon-String Guitar  ", "Steel-String Guitar  ", "Jazz Guitar          ", "Clean Elec. Guitar   ", "Muted Elec. Guitar   ", "Overdriven Guitar    ", "Distortion Guitar    ", "Guitar Harmonics     ", "Acoustic Bass        ", "Fingered Bass        ", "Picked Bass          ", "Fretless Bass        ", "Slap Bass 1          ", "Slap Bass 2          ", "Synth Bass 1         ", "Synth Bass 2         ", "Violin               ", "Viola                ", "Cello                ", "Contrabass           ", "Tremolo Strings      ", "Pizzicato Strings    ", "Harp                 ", "Timpani              ", "String Ensemble      ", "Slow String Ensemble ", "Synth Strings 1      ", "Synth Strings 2      ", "Choir Aahs           ", "Voice Oohs           ", "Synth Voice          ", "Orchestra Hit        ", "Trumpet              ", "Trombone             ", "Tuba                 ", "Muted Trumpet        ", "French Horn          ", "Brass Section        ", "Synth Brass 1        ", "Synth Brass 2        ", "Soprano Sax          ", "Alto Sax             ", "Tenor Sax            ", "Baritone Sax         ", "Oboe                 ", "English Horn         ", "Bassoon              ", "Clarinet             ", "Piccolo              ", "Flute                ", "Recorder             ", "Pan Flute            ", "Blown Bottle         ", "Shakuhachi           ", "Whistle              ", "Ocarina              ", "Square Lead          ", "Saw Lead             ", "Synth Calliope       ", "Chiffer Lead         ", "Charang              ", "Solo Synth Vox       ", "5th Saw Wave         ", "Bass & Lead          ", "Fantasia Pad         ", "Warm Pad             ", "Polysynth Pad        ", "Space Voice Pad      ", "Bowed Glass Pad      ", "Metal Pad            ", "Halo Pad             ", "Sweep Pad            ", "Ice Rain             ", "Soundtrack           ", "Crystal              ", "Atmosphere           ", "Brightness           ", "Goblin               ", "Echo Drops           ", "Star Theme           ", "Sitar                ", "Banjo                ", "Shamisen             ", "Koto                 ", "Kalimba              ", "Bagpipe              ", "Fiddle               ", "Shanai               ", "Tinkle Bell          ", "Agogo                ", "Steel Drums          ", "Woodblock            ", "Taiko Drum           ", "Melodic Tom          ", "Synth Drum           ", "Reverse Cymbal       ", "Guitar Fret Noise    ", "Breath Noise         ", "Seashore             ", "Bird Tweet           ", "Telephone            ", "Helicopter           ", "Applause             ", "Gun Shot             ", "Wide Acoustic Grand  ", "Wide Bright Acoustic ", "Wide Electric Grand  ", "Wide Honky-tonk      ", "Detuned Elec. Piano 1", "Detuned Elec. Piano 2", "Coupled Harpsichord  ", "Pulse Clavinet       ", "Wet Vibraphone       ", "Wide Marimba         ", "Church Bells         ", "Detuned Organ 1      ", "Detuned Organ 2      ", "Church Organ 2       ", "Puff Organ           ", "Italian Accordion    ", "Ukelele              ", "12-String Guitar     ", "Hawaiian Guitar      ", "Chorus Guitar        ", "Funk Guitar          ", "Guitar Pinch         ", "Feedback Guitar      ", "Guitar Feedback      ", "Finger Slap          ", "Synth Bass 101       ", "Synth Bass 4         ", "Slow Violin          ", "Yang Qin             ", "Orchestra Strings    ", "Synth Strings 3      ", "Choir Aahs 2         ", "Humming              ", "Analog Voice         ", "Bass Hit             ", "Dark Trumpet         ", "Trombone 2           ", "Muted Trumpet 2      ", "French Horn 2        ", "Brass Section        ", "Synth Brass 3        ", "Synth Brass 4        ", "Square Wave          ", "Saw Wave             ", "Wire Lead            ", "Delayed Lead         ", "Sine Pad             ", "Itopia               ", "Synth Mallet         ", "Echo Bell            ", "Sitar 2              ", "Taisho Koto          ", "Castanets            ", "Concert Bass Drum    ", "Melodic Tom 2        ", "808 Tom              ", "Guitar Cut Noise     ", "Flute Key Click      ", "Rain                 ", "Dog                  ", "Telephone 2          ", "Car Engine           ", "Laughing             ", "Machine Gun          ", "Dark Acoustic Grand  ", "Elec Piano 1 Variant ", "Elec Piano 2 Variant ", "Wide Harpsichord     ", "Carillon             ", "60's Organ 1         ", "Organ 5              ", "Church Organ 3       ", "Open Nylon Guitar    ", "Mandolin             ", "Mid Tone Guitar      ", "Funk Guitar 2        ", "Distorted Rtm Guitar ", "Synth Bass 3         ", "Rubber Bass          ", "60's Strings         ", "6th Hit              ", "Bright Trombone      ", "Analog Brass 1       ", "Analog Brass 2       ", "Sine Wave            ", "Doctor Solo          ", "Echo Pan             ", "Electric Percussion  ", "String Slap          ", "Thunder              ", "Horse Gallop         ", "Door Creaking        ", "Car Stop             ", "Screaming            ", "Lasergun             ", "60's Electric Piano  ", "Elec. Piano Legend   ", "Open Harpsichord     ", "Organ 4              ", "Nylon Guitar 2       ", "Steel + Body         ", "Jazz Man             ", "Clavi Bass           ", "Attack Pulse         ", "Euro Hit             ", "Jump Brass           ", "Natural Lead         ", "Wind                 ", "Bird 2               ", "Door Closing         ", "Car Pass             ", "Punch                ", "Explosion            ", "Electric Piano Phase ", "Hammer               ", "Sequenced Saw        ", "Stream               ", "Scratch              ", "Car Crash            ", "Heart Beat           ", "Bubble               ", "Wind Chimes          ", "Siren                ", "Footsteps            ", "Train                ", "Jetplane             ", "Starship             ", "Burst Noise          "};

// GeneralUser GS 1.471 SoundFont (http://www.schristiancollins.com/generaluser.php):
// const byte drumKitMap[]   = {0, 1, 8, 16, 24, 25, 26, 32, 40, 48, 56};
// const byte bankMap[]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 16, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120};
// const byte programMap[]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 38, 44, 48, 49, 52, 56, 57, 59, 60, 61, 80, 81, 98, 120, 121, 122, 123, 124, 125, 126, 127, 102, 120, 122, 123, 124, 125, 126, 127, 122, 123, 124, 125, 126, 127, 122, 123, 125, 126, 122, 124, 125, 126, 125, 125, 4, 5, 6, 14, 16, 17, 19, 21, 24, 25, 26, 27, 28, 30, 31, 38, 39, 48, 50, 61, 62, 63, 80, 81, 107, 115, 116, 117, 118, 125, 14, 125, 0, 1, 4, 5, 8, 14, 38, 39, 49, 50, 51, 61, 78, 81, 87, 88, 89, 96, 98, 100, 119, 121, 127, 0, 4, 10, 27, 38, 48, 49, 80, 81, 88, 89, 119, 122, 127, 48, 80, 81, 88, 25, 0, 1, 8, 16, 24, 25, 26, 32, 40, 48, 56};
// const char *drumKitName[] = {"Standard             ", "Standard 2           ", "Room                 ", "Power                ", "Electronic           ", "808/909              ", "Dance                ", "Jazz                 ", "Brush                ", "Orchestral           ", "SFX                  "};
// const char *programName[] = {"Stereo Grand         ", "Bright Grand         ", "Electric Grand       ", "Honky-Tonk           ", "Tine Electric Piano  ", "FM Electric Piano    ", "Harpsichord          ", "Clavinet             ", "Celeste              ", "Glockenspiel         ", "Music Box            ", "Vibraphone           ", "Marimba              ", "Xylophone            ", "Tubular Bells        ", "Dulcimer             ", "Tonewheel Organ      ", "Percussive Organ     ", "Rock Organ           ", "Pipe Organ           ", "Reed Organ           ", "Accordian            ", "Harmonica            ", "Bandoneon            ", "Nylon Guitar         ", "Steel Guitar         ", "Jazz Guitar          ", "Clean Guitar         ", "Muted Guitar         ", "Overdrive Guitar     ", "Distortion Guitar    ", "Guitar Harmonics     ", "Acoustic Bass        ", "Finger Bass          ", "Pick Bass            ", "Fretless Bass        ", "Slap Bass 1          ", "Slap Bass 2          ", "Synth Bass 1         ", "Synth Bass 2         ", "Violin               ", "Viola                ", "Cello                ", "Double Bass          ", "Stereo Strings Trem  ", "Pizzicato Strings    ", "Orchestral Harp      ", "Timpani              ", "Stereo Strings Fast  ", "Stereo Strings Slow  ", "Synth Strings 1      ", "Synth Strings 2      ", "Concert Choir        ", "Voice Oohs           ", "Synth Voice          ", "Orchestra Hit        ", "Trumpet              ", "Trombone             ", "Tuba                 ", "Muted Trumpet        ", "French Horns         ", "Brass Section        ", "Synth Brass 1        ", "Synth Brass 2        ", "Soprano Sax          ", "Alto Sax             ", "Tenor Sax            ", "Baritone Sax         ", "Oboe                 ", "English Horn         ", "Bassoon              ", "Clarinet             ", "Piccolo              ", "Flute                ", "Recorder             ", "Pan Flute            ", "Bottle Blow          ", "Shakuhachi           ", "Irish Tin Whistle    ", "Ocarina              ", "Square Lead          ", "Saw Lead             ", "Synth Calliope       ", "Chiffer Lead         ", "Charang              ", "Solo Vox             ", "5th Saw Wave         ", "Bass & Lead          ", "Fantasia             ", "Warm Pad             ", "Polysynth            ", "Space Voice          ", "Bowed Glass          ", "Metal Pad            ", "Halo Pad             ", "Sweep Pad            ", "Ice Rain             ", "Soundtrack           ", "Crystal              ", "Atmosphere           ", "Brightness           ", "Goblin               ", "Echo Drops           ", "Star Theme           ", "Sitar                ", "Banjo                ", "Shamisen             ", "Koto                 ", "Kalimba              ", "Bagpipes             ", "Fiddle               ", "Shenai               ", "Tinker Bell          ", "Agogo                ", "Steel Drums          ", "Wood Block           ", "Taiko Drum           ", "Melodic Tom          ", "Synth Drum           ", "Reverse Cymbal       ", "Fret Noise           ", "Breath Noise         ", "Seashore             ", "Birds                ", "Telephone 1          ", "Helicopter           ", "Applause             ", "Gun Shot             ", "Synth Bass 101       ", "Mono Strings Trem    ", "Mono Strings Fast    ", "Mono Strings Slow    ", "Concert Choir Mono   ", "Trumpet 2            ", "Trombone 2           ", "Muted Trumpet 2      ", "Solo French Horn     ", "Brass Section Mono   ", "Square Wave          ", "Saw Wave             ", "Synth Mallet         ", "Cut Noise            ", "Fl. Key Click        ", "Rain                 ", "Dog                  ", "Telephone 2          ", "Car-Engine           ", "Laughing             ", "Machine Gun          ", "Echo Pan             ", "String Slap          ", "Thunder              ", "Horse Gallop         ", "Door Creaking        ", "Car-Stop             ", "Scream               ", "Lasergun             ", "Howling Winds        ", "Bird 2               ", "Door                 ", "Car-Pass             ", "Punch                ", "Explosion            ", "Stream               ", "Scratch              ", "Car-Crash            ", "Heart Beat           ", "Bubbles              ", "Windchime            ", "Siren                ", "Footsteps            ", "Train                ", "Jet Plane            ", "Chorused Tine EP     ", "Chorused FM EP       ", "Coupled Harpsichord  ", "Church Bells         ", "Detuned Tnwl. Organ  ", "Detuned Perc. Organ  ", "Pipe Organ 2         ", "Italian Accordian    ", "Ukulele              ", "12-String Guitar     ", "Hawaiian Guitar      ", "Chorused Clean Gt.   ", "Funk Guitar          ", "Feedback Guitar      ", "Guitar Feedback      ", "Synth Bass 3         ", "Synth Bass 4         ", "Orchestra Pad        ", "Synth Strings 3      ", "Brass Section 2      ", "Synth Brass 3        ", "Synth Brass 4        ", "Sine Wave            ", "Doctor Solo          ", "Taisho Koto          ", "Castanets            ", "Concert Bass Drum    ", "Melodic Tom 2        ", "808 Tom              ", "Starship             ", "Carillon             ", "Burst Noise          ", "Piano & Str.-Fade    ", "Piano & Str.-Sus     ", "Tine & FM EPs        ", "Piano & FM EP        ", "Tinkling Bells       ", "Bell Tower           ", "Techno Bass          ", "Pulse Bass           ", "Stereo Strings Velo  ", "Synth Strings 4      ", "Synth Strings 5      ", "Brass Section 3      ", "Whistlin'            ", "Sawtooth Stab        ", "Doctor's Solo        ", "Harpsi Pad           ", "Solar Wind           ", "Mystery Pad          ", "Synth Chime          ", "Bright Saw Stack     ", "Cymbal Crash         ", "Filter Snap          ", "Interference         ", "Bell Piano           ", "Bell Tine EP         ", "Christmas Bells      ", "Clean Guitar 2       ", "Mean Saw Bass        ", "Full Orchestra       ", "Mono Strings Velo    ", "Square Lead 2        ", "Saw Lead 2           ", "Fantasia 2           ", "Solar Wind 2         ", "Tambourine           ", "White Noise Wave     ", "Shooting Star        ", "Woodwind Choir       ", "Square Lead 3        ", "Saw Lead 3           ", "Night Vision         ", "Mandolin             ", "Standard Drums       ", "Standard 2 Drums     ", "Room Drums           ", "Power Drums          ", "Electronic Drums     ", "808 909 Drums        ", "Dance Drums          ", "Jazz Drums           ", "Brush Drums          ", "Orchestral Perc.     ", "SFX Kit              "};

// Timbres of Heaven 3.94 SoundFont (http://midkar.com/soundfonts/):
// const byte drumKitMap[]   = {0, 1, 8, 16, 24, 25, 26, 27, 32, 40, 48, 50, 56, 99, 100, 101, 102, 103, 104, 105, 106, 107, 127};
// const byte bankMap[]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 14, 14, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 18, 18, 18, 19, 24, 24, 24, 27, 27, 30, 30, 30, 32, 32, 32, 33, 40, 40, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 90, 90, 99, 101,101,101, 126, 126};
// const byte programMap[]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 1, 5, 10, 16, 17, 21, 22, 24, 25, 26, 27, 28, 30, 33, 34, 36, 37, 38, 39, 46, 53, 57, 66, 73, 77, 90, 92, 98, 109, 120, 121, 122, 123, 124, 125, 126, 127, 5, 24, 25, 26, 34, 36, 38, 46, 102, 119, 120, 121, 122, 123, 124, 125, 126, 127, 5, 34, 38, 39, 46, 90, 122, 123, 124, 125, 126, 127, 4, 5, 25, 27, 46, 90, 122, 123, 124, 125, 126, 122, 124, 125, 126, 120, 122, 123, 125, 127, 122, 125, 6, 14, 21, 24, 25, 26, 27, 28, 30, 31, 34, 36, 37, 38, 39, 40, 48, 50, 61, 62, 65, 68, 80, 81, 105, 115, 116, 117, 118, 125, 5, 14, 27, 30, 36, 37, 65, 112, 125, 38, 101, 0, 1, 5, 7, 18, 24, 25, 26, 27, 30, 38, 46, 4, 18, 27, 98, 27, 0, 4, 80, 16, 80, 0, 1, 2, 24, 25, 52, 52, 16, 24, 0, 1, 3, 5, 16, 32, 33, 34, 35, 36, 37, 48, 49, 50, 54, 55, 64, 65, 66, 67, 68, 69, 70, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 96, 97, 98, 99, 100, 101, 112, 113, 114, 115, 38, 39, 38, 0, 1, 2, 0, 1};
// const char *drumKitName[] = {"Don's Std Kit        ", "Std Kit 2            ", "Room Drum Kit        ", "Power Drums          ", "Electronic Kit       ", "TR-808 Elec. Kit     ", "TR-909 Elec. Kit     ", "Dance Kit            ", "Jazz Drum Kit        ", "Brush Kit            ", "Orchestra Kit        ", "Symphony Kit         ", "GM2 SFX Kit          ", "PSR-520 Std kit      ", "Don's XG Std Kit     ", "XG Room Kit          ", "XG Rock Kit          ", "XG Electronic Kit    ", "XG Analog Kit        ", "XG Jazz Kit          ", "XG Brush Kit         ", "XG Symphony Kit      ", "CM-64 32L (SC-55)    "};
// const char *programName[] = {"Concert Grand        ", "Bright Grand         ", "Electric Grand       ", "Honky Tonk           ", "Don's Rhodes MKV EP  ", "Chorused Rhodes EP   ", "Don's Harpsichord    ", "Don's Clavinet       ", "Don's Celesta        ", "Glockenspiel         ", "Music Box            ", "Vibraphone           ", "Marimba              ", "Xylophone            ", "Tubular Bells        ", "Dulcimer (Santur)    ", "Wave Organ           ", "Perc. Organ Chopper  ", "Rock Organ           ", "Church Organ         ", "Reed Organ           ", "Accordion            ", "HarmonicaVibrato     ", "Bandoneon            ", "Solo Nylon Guit      ", "Steel String Guitar  ", "Jazz Guitar          ", "Fender Strat         ", "Muted Elec. Guitar   ", "Ult. Overdriven Gt.  ", "Ult. Distortion Gt.  ", "Guitar Harmonics     ", "Acoustic Bass        ", "Bass 305             ", "Don's Picked Bass    ", "Fretless Bass        ", "Don's Slap Bass 1    ", "Don's Slap Bass 2    ", "Synth Bass 1         ", "Synth Bass 2         ", "Don's Solo Violin    ", "Viola                ", "Cello                ", "Contrabass           ", "Tremolo Strings      ", "Pizzicato Section    ", "Clavinova Harp       ", "Timpani              ", "Don's Strings        ", "Slow Strings         ", "Synth Strings 1      ", "Synth Strings 2      ", "Choir Aahs           ", "Voice Oohs           ", "Synth Voice          ", "Orchestra Hit        ", "Trumpet              ", "Roly's Trombone      ", "Don's Tuba           ", "Don's Muted Trumpet  ", "French Horns         ", "Brass Section        ", "Don's Synth Brass 1  ", "Synth Brass 2        ", "Soprano Sax          ", "Alto Sax             ", "Tenor sax            ", "Baritone Sax         ", "Don's Oboe           ", "English Horn         ", "Bassoon              ", "Clarinet             ", "Piccolo              ", "Flute V2             ", "Recorder             ", "Pan Flute            ", "Bottle Chiff         ", "Don's Shakuhachi     ", "Whistle              ", "Ocarina (Breathy)    ", "Square Lead          ", "Saw Wave             ", "Synth Calliope       ", "Don's Chiffer Lead   ", "Charang              ", "Solo Synth Voice     ", "5th Saw Wave         ", "Bass & Lead          ", "Don's Fantasia       ", "Warm Strings         ", "Poly Synth           ", "Space Voice          ", "Bowed Glass          ", "Metal Pad            ", "Halo pad             ", "Sweep Pad            ", "Don's Ice Rain       ", "Soundtrack           ", "Crystal              ", "Atmosphere           ", "Brightness           ", "Goblin (Sci-Fi)      ", "Echo Drops           ", "Don's Star Theme     ", "Sitar                ", "Don's Banjo          ", "Shamisen             ", "Koto                 ", "Kalimba              ", "Bagpipes             ", "Don's Fiddle         ", "Shannai              ", "Tinkle Bell          ", "Agogo                ", "Steel Drums          ", "Woodblock            ", "Taiko Drum           ", "Melodic Tom          ", "Synth Drum           ", "Reverse Cymbal       ", "Guitar String Noise  ", "Breath Noise         ", "Sea Shore            ", "Bird Tweets          ", "Telephone            ", "Helicopter           ", "Don's Applause       ", "Gun Shot 357 Magnum  ", "Glory Piano          ", "Crystal Rhodes EP    ", "Chromatinium         ", "Fury Organ 101       ", "Ice Organ            ", "HarmonyPhone         ", "Harmonica No Vib     ", "Chor. Nylon Guitar   ", "Muted Steel Guitar   ", "Jazz Amp V2          ", "Clean Machine        ", "Muted Dist. Guitar   ", "Destroyer Guitar     ", "Flatwound Bass       ", "Jazz Bass            ", "Don's Pop Bass 1     ", "Don's Pop Bass2      ", "Synth Bass 101       ", "Synth Bass 201       ", "Nylon Harp           ", "Soul Oohs            ", "Don's Cornet         ", "Tenor Sax Soft       ", "Don's Flute          ", "Don's Shakuhachi     ", "Moog Rez Pad         ", "Soft Bell Pad        ", "Synth Mallet         ", "Don's BagPipes       ", "Gtr. Cut Noise       ", "Fl. Key Click        ", "Rain                 ", "Dog                  ", "Telephone 2          ", "Car-Engine           ", "GS Laughs            ", "Machine Gun          ", "Crystal Dreams       ", "Nylon Velo Guitar    ", "Don's Rover Guitar   ", "Don's Jazz Man Gtr   ", "Picked Bass 3        ", "Nose Bass            ", "Pulse Bass           ", "Guzheng Harp         ", "Echo Pan             ", "Gong!!!              ", "String Slap          ", "Fem. Breaths         ", "Don's Thunder        ", "Horse-Gallop         ", "Door Creaking        ", "Car-Stop             ", "Screaming            ", "Lasergun             ", "Soft Chor. Rhodes    ", "Picked Bass 4        ", "Orbit Dualrezz       ", "Sequenced Bass       ", "Yang Qin             ", "JP8 Swell Pad        ", "Wind                 ", "Bird  2              ", "Door Slam            ", "Car-Pass             ", "Punch                ", "Explosion            ", "DX7 Chorused         ", "CP-80 Chorused       ", "Don's Dobro Guitar   ", "Twnagster Clean      ", "Muted Nylon Harp     ", "Avatar Swell-1       ", "Stream               ", "Seagulls             ", "Scratch              ", "Car-Crash            ", "Heart Beat           ", "Bubble               ", "Wind Chimes          ", "Siren                ", "Footsteps            ", "Pick Scrape          ", "Rainstick            ", "SONAR                ", "Train                ", "Laser Drum           ", "Waterfall            ", "Jetplane             ", "Coupled Harpsichord  ", "Church Bell          ", "Organetto Superiore  ", "Ukulele              ", "12-String Guitar     ", "Hawaiian Guitar      ", "Chorused Strat       ", "Don's Funk Guitar    ", "Feedback Guitar      ", "Guitar Feedback      ", "Muted Picked Bass    ", "Reso Slap Bass       ", "FM Slap Bass         ", "Synth Bass 3         ", "Acid Bass 2          ", "Slow Violin          ", "Orchestral Pad       ", "Synth Strings 3      ", "Brass Section 2      ", "Synth Brass 3        ", "Hyper Alto Sax       ", "Oboe Exp.            ", "Sine Wave            ", "Dr. Solo             ", "Don's Sarod          ", "Castanets            ", "Concert Bass Drum    ", "Melo Tom 2           ", "Don's 808 Tom        ", "StarShip             ", "CrystalPhone         ", "Carillon             ", "Boss Guitar          ", "Feedback Guitar 2    ", "Unison Slap Bass     ", "FM Slap Bass 2       ", "Growl Sax            ", "Don's Gender         ", "Burst Noise          ", "Rubber Bass          ", "UFO FX               ", "Dry Grand            ", "Dance Piano          ", "Chromophone          ", "Super Clavinet       ", "B3 Slow Rotor        ", "CP Flamenco          ", "Mandolin             ", "Pedal Steele Guitar  ", "Gibson ES-335        ", "Power Guitar         ", "Synth Bass 301       ", "FM Harp              ", "Don's Wurlitzer EP   ", "Rock Organ 2         ", "Don's Les Paul       ", "Bell Harp            ", "Skynyrd LP           ", "Dream Piano          ", "Don's 60's E. Piano  ", "BuzzPad              ", "Hybrid Organ         ", "Electrum Pad         ", "TB-303-1             ", "TB-303-2             ", "TB-303-3             ", "Nylon Guitar 2       ", "Steel Guitar 2       ", "Choir Aahs 2         ", "Choir+ Strings       ", "Don's Lanz Organ     ", "Don's Lute           ", "XG Cutting Noise     ", "XG Cut Noise 2       ", "XG String Slap       ", "XG Pick Scrape       ", "XG Flt. Key Click    ", "XG Rain              ", "XG Thunder           ", "XG Wind              ", "XG Stream            ", "XG Bubbles           ", "XG Feed              ", "XG Dog Bark          ", "XG Horse-Gallop      ", "XG Bird  2           ", "XG Ghost Laughter    ", "XG SFX Maou          ", "DTMF Dial Sound      ", "XG Door Squeak       ", "XG Door Slam         ", "XG Scratch Push      ", "XG Scratch Pull      ", "XG Wind Chimes       ", "XG Telephone 2       ", "XG Car-Engine        ", "XG Car-Stop          ", "XG Car-Pass          ", "XG Car-Crash         ", "XG Siren             ", "XG Train             ", "XG Jetplane          ", "XG StarShip          ", "XG Burst Noise       ", "XG Coaster           ", "XG Submarine         ", "XG Laughing          ", "XG Scream            ", "XG Punch             ", "XG Heart Beat        ", "XG Footsteps         ", "XG Applause 2        ", "XG Machine Gun       ", "XG Lasergun          ", "XG Explosion         ", "XG FireWork          ", "FM Synth Bass 1      ", "FM Synth Bass 2      ", "Wire Bass            ", "C64 Pulse            ", "C64 Saw              ", "C64 Triangle         ", "XG SFX Kit 1         ", "XG SFX Kit 2         "};

// Arachno SoundFont 1.0 SoundFont (http://www.arachnosoft.com/main/soundfont.php):
// const byte drumKitMap[]   = {0, 8, 16, 24, 25, 32, 40, 48, 49, 127};
// const byte bankMap[]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// const byte programMap[]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127};
// const char *drumKitName[] = {"Standard Drum Kit    ", "Room Drum Kit        ", "Power Drum Kit       ", "Electronic Drum Kit  ", "TR-808/909 Drum Kit  ", "Jazz Drum Kit        ", "Brush Drum Kit       ", "Orchestral Drum Kit  ", "Fix Room Drum Kit    ", "MT-32 Drum Kit       "};
// const char *programName[] = {"Grand Piano          ", "Bright Piano         ", "Rock Piano           ", "Honky-Tonk Piano     ", "Electric Piano       ", "Crystal Piano        ", "Harpsichord          ", "Clavinet             ", "Celesta              ", "Glockenspiel         ", "Music Box            ", "Vibraphone           ", "Marimba              ", "Xylophone            ", "Tubular Bells        ", "Dulcimer (Santur)    ", "DrawBar Organ        ", "Percussive Organ     ", "Rock Organ           ", "Church Organ         ", "Reed Organ           ", "Accordion            ", "Harmonica            ", "Bandoneon            ", "Nylon Guitar         ", "Steel String Guitar  ", "Jazz Guitar          ", "Clean Guitar         ", "Muted Guitar         ", "Overdrive Guitar     ", "Distortion Guitar    ", "Guitar Harmonics     ", "Acoustic Bass        ", "Fingered Bass        ", "Picked Bass          ", "Fretless Bass        ", "Slap Bass 1          ", "Slap Bass 2          ", "Synth Bass 1         ", "Synth Bass 2         ", "Violin               ", "Viola                ", "Cello                ", "ContraBass           ", "Tremolo Strings      ", "Pizzicato Strings    ", "Orchestral Harp      ", "Timpani              ", "Strings Ensemble 1   ", "Strings Ensemble 2   ", "Synth Strings 1      ", "Synth Strings 2      ", "Choir Aahs           ", "Voice Oohs           ", "Synth Voice          ", "Orchestra Hit        ", "Trumpet              ", "Trombone             ", "Tuba                 ", "Muted Trumpet        ", "French Horns         ", "Brass Section        ", "Synth Brass 1        ", "Synth Brass 2        ", "Soprano Sax          ", "Alto Sax             ", "Tenor Sax            ", "Baritone Sax         ", "Oboe                 ", "English Horns        ", "Bassoon              ", "Clarinet             ", "Piccolo              ", "Flute                ", "Recorder             ", "Pan Flute            ", "Blown Bottle         ", "Shakuhachi           ", "Whistle              ", "Ocarina              ", "Square Wave          ", "Saw Wave             ", "Synth Calliope       ", "Chiffer Lead         ", "Charang              ", "Solo Voice           ", "5th Saw Wave         ", "Bass & Lead          ", "Fantasia (New Age)   ", "Warm Pad             ", "Poly Synth           ", "Space Voice          ", "Bowed Glass          ", "Metal Pad            ", "Halo Pad             ", "Sweep Pad            ", "Ice Rain             ", "Sound Track          ", "Crystal              ", "Atmosphere           ", "Brightness           ", "Goblin               ", "Echo Drops           ", "Star Theme           ", "Sitar                ", "Banjo                ", "Shamisen             ", "Koto                 ", "Kalimba              ", "Bag Pipe             ", "Fiddle               ", "Shannai              ", "Tinkle Bell          ", "Agogo                ", "Steel Drums          ", "Wood Block           ", "Taiko Drum           ", "Melodic Tom          ", "Synth Drum           ", "Reverse Cymbal       ", "Guitar Fret Noise    ", "Breath Noise         ", "Sea Shore            ", "Bird Tweets          ", "Telephone            ", "Helicopter           ", "Applause             ", "Gun Shot             "};

// FluidSynth FluidR3_GM SoundFont (https://www.fluidsynth.org):
// const byte drumKitMap[]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 24, 25, 32, 33, 34, 35, 36, 40, 41, 42, 48};
// const byte bankMap[]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 16};
// const byte programMap[]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 4, 5, 6, 14, 16, 17, 19, 21, 24, 25, 26, 28, 30, 31, 38, 39, 40, 48, 50, 61, 62, 63, 80, 107, 115, 116, 117, 118, 125, 25};
// const char *drumKitName[] = {"Standard             ", "Standard 1           ", "Standard 2           ", "Standard 3           ", "Standard 4           ", "Standard 5           ", "Standard 6           ", "Standard 7           ", "Room                 ", "Room 1               ", "Room 2               ", "Room 3               ", "Room 4               ", "Room 5               ", "Room 6               ", "Room 7               ", "Power                ", "Power 1              ", "Power 2              ", "Power 3              ", "Electronic           ", "TR-808               ", "Jazz                 ", "Jazz 1               ", "Jazz 2               ", "Jazz 3               ", "Jazz 4               ", "Brush                ", "Brush 1              ", "Brush 2              ", "Orchestra Kit        "};
// const char *programName[] = {"Yamaha Grand Piano   ", "Bright Yamaha Grand  ", "Electric Piano       ", "Honky Tonk           ", "Rhodes EP            ", "Legend EP 2          ", "Harpsichord          ", "Clavinet             ", "Celesta              ", "Glockenspiel         ", "Music Box            ", "Vibraphone           ", "Marimba              ", "Xylophone            ", "Tubular Bells        ", "Dulcimer             ", "DrawbarOrgan         ", "Percussive Organ     ", "Rock Organ           ", "Church Organ         ", "Reed Organ           ", "Accordion            ", "Harmonica            ", "Bandoneon            ", "Nylon String Guitar  ", "Steel String Guitar  ", "Jazz Guitar          ", "Clean Guitar         ", "Palm Muted Guitar    ", "Overdrive Guitar     ", "Distortion Guitar    ", "Guitar Harmonics     ", "Acoustic Bass        ", "Fingered Bass        ", "Picked Bass          ", "Fretless Bass        ", "Slap Bass            ", "Pop Bass             ", "Synth Bass 1         ", "Synth Bass 2         ", "Violin               ", "Viola                ", "Cello                ", "Contrabass           ", "Tremolo              ", "Pizzicato Section    ", "Harp                 ", "Timpani              ", "Strings              ", "Slow Strings         ", "Synth Strings 1      ", "Synth Strings 2      ", "Ahh Choir            ", "Ohh Voices           ", "Synth Voice          ", "Orchestra Hit        ", "Trumpet              ", "Trombone             ", "Tuba                 ", "Muted Trumpet        ", "French Horns         ", "Brass Section        ", "Synth Brass 1        ", "Synth Brass 2        ", "Soprano Sax          ", "Alto Sax             ", "Tenor Sax            ", "Baritone Sax         ", "Oboe                 ", "English Horn         ", "Bassoon              ", "Clarinet             ", "Piccolo              ", "Flute                ", "Recorder             ", "Pan Flute            ", "Bottle Chiff         ", "Shakuhachi           ", "Whistle              ", "Ocarina              ", "Square Lead          ", "Saw Wave             ", "Calliope Lead        ", "Chiffer Lead         ", "Charang              ", "Solo Vox             ", "Fifth Sawtooth Wave  ", "Bass & Lead          ", "Fantasia             ", "Warm Pad             ", "Polysynth            ", "Space Voice          ", "Bowed Glass          ", "Metal Pad            ", "Halo Pad             ", "Sweep Pad            ", "Ice Rain             ", "Soundtrack           ", "Crystal              ", "Atmosphere           ", "Brightness           ", "Goblin               ", "Echo Drops           ", "Star Theme           ", "Sitar                ", "Banjo                ", "Shamisen             ", "Koto                 ", "Kalimba              ", "BagPipe              ", "Fiddle               ", "Shenai               ", "Tinker Bell          ", "Agogo                ", "Steel Drums          ", "Woodblock            ", "Taiko Drum           ", "Melodic Tom          ", "Synth Drum           ", "Reverse Cymbal       ", "Fret Noise           ", "Breath Noise         ", "Sea Shore            ", "Bird Tweet           ", "Telephone            ", "Helicopter           ", "Applause             ", "Gun Shot             ", "Detuned EP 1         ", "Detuned EP 2         ", "Coupled Harpsichord  ", "Church Bell          ", "Detuned Organ 1      ", "Detuned Organ 2      ", "Church Organ 2       ", "Italian Accordion    ", "Ukulele              ", "12 String Guitar     ", "Hawaiian Guitar      ", "Funk Guitar          ", "Feedback Guitar      ", "Guitar Feedback      ", "Synth Bass 3         ", "Synth Bass 4         ", "Slow Violin          ", "Orchestral Pad       ", "Synth Strings 3      ", "Brass 2              ", "Synth Brass 3        ", "Synth Brass 4        ", "Sine Wave            ", "Taisho Koto          ", "Castanets            ", "Concert Bass Drum    ", "Melo Tom 2           ", "808 Tom              ", "Burst Noise          ", "Mandolin             "};


// Onboard PSG Synth
const int synthDrumKitMap[] = {WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD, WAVEFORM_SAMPLE_HOLD};
const int synthProgramMap[] = {WAVEFORM_SINE, WAVEFORM_SINE, WAVEFORM_SAWTOOTH, WAVEFORM_SAWTOOTH, WAVEFORM_SQUARE, WAVEFORM_SQUARE, WAVEFORM_TRIANGLE, WAVEFORM_TRIANGLE, WAVEFORM_TRIANGLE_VARIABLE, WAVEFORM_TRIANGLE_VARIABLE, WAVEFORM_PULSE, WAVEFORM_PULSE};
const char *synthDrumKitName[] = {"Noise (short)        ", "Noise (long)         ", "Noise (short)        ", "Noise (long)         ", "Noise (short)        ", "Noise (long)         ", "Noise (short)        ", "Noise (long)         ", "Noise (short)        ", "Noise (long)         ", "Noise (short)        ", "Noise (long)         "};
const char *synthProgramName[] = {"Sine Wave (short)    ", "Sine Wave (long)     ", "Sawtooth Wave (short)", "Sawtooth Wave (long) ", "Square Wave (short)  ", "Square Wave (long)   ", "Triangle Wave (short)", "Triangle Wave (long) ", "Tri Wave Var. (short)", "Tri Wave Var. (long) ", "Pulse Wave (short)   ", "Pulse Wave (long)    "};
const int synthEnvSustainMap[] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,};

const float midiPitchFrequencyMap[] = {
     8.176,     8.662,     9.177,     9.723,    10.301,    10.913,    11.562,    12.250,    12.978,    13.750,    14.568,    15.434,        // C-1 to B-1
    16.352,    17.324,    18.354,    19.445,    20.602,    21.827,    23.125,    24.500,    25.957,    27.500,    29.135,    30.868,        //  C0 to B0
    32.703,    34.648,    36.708,    38.891,    41.203,    43.654,    46.249,    48.999,    51.913,    55.000,    58.270,    61.735,        //  C1 to B1
    65.406,    69.296,    73.416,    77.782,    82.407,    87.307,    92.499,    97.999,   103.826,   110.000,   116.541,   123.471,        //  C2 to B2
   130.813,   138.591,   146.832,   155.563,   164.814,   174.614,   184.997,   195.998,   207.652,   220.000,   233.082,   246.942,        //  C3 to B3
   261.626,   277.183,   293.665,   311.127,   329.628,   349.228,   369.994,   391.995,   415.305,   440.000,   466.164,   493.883,        //  C4 to B4
   523.251,   554.365,   587.330,   622.254,   659.255,   698.456,   739.989,   783.991,   830.609,   880.000,   932.328,   987.767,        //  C5 to B5
  1046.502,  1108.731,  1174.659,  1244.508,  1318.510,  1396.913,  1479.978,  1567.982,  1661.219,  1760.000,  1864.655,  1975.533,        //  C6 to B6
  2093.005,  2217.461,  2349.318,  2489.016,  2637.020,  2793.826,  2959.955,  3135.963,  3322.438,  3520.000,  3729.310,  3951.066,        //  C7 to B7
  4186.009,  4434.922,  4698.636,  4978.032,  5274.041,  5587.652,  5919.911,  6271.927,  6644.875,  7040.000,  7458.620,  7902.133,        //  C8 to B8
  8372.018,  8869.844,  9397.273,  9956.063, 10548.082, 11175.303, 11839.822, 12543.854,                                                    //  C9 to G9
};

// Mapping function for floats
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float waveformAmplitude;                    // Passing velocity level to synth as the amplitude
byte synthModToggle;

byte waveformPitch[16][8] = {
{255, 255, 255, 255, 255, 255, 255, 255},   // Default to 255 as an "unused" indicator as it's outside the range of pitch values 0-127
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
{255, 255, 255, 255, 255, 255, 255, 255},
};


// Teensy Audio library configuration (visualizer: https://www.pjrc.com/teensy/gui/index.html)
// 128 waveforms to 128 envelopes (8 note polyphony per channel x 16) routed to 32 mixers to 8 mixers to 2 mixers to 1 mixer to audio out (MQS)
AudioSynthWaveform      waveformCh[16][8];
AudioEffectEnvelope     envelopeCh[16][8];
AudioMixer4             firstMixer[32];
AudioMixer4             secondMixer[8];
AudioMixer4             thirdMixer[2];
AudioMixer4             outMixer;
AudioConnection         ch00WaveformToEnvelope0(waveformCh[0][0], 0, envelopeCh[0][0], 0);
AudioConnection         ch00WaveformToEnvelope1(waveformCh[0][1], 0, envelopeCh[0][1], 0);
AudioConnection         ch00WaveformToEnvelope2(waveformCh[0][2], 0, envelopeCh[0][2], 0);
AudioConnection         ch00WaveformToEnvelope3(waveformCh[0][3], 0, envelopeCh[0][3], 0);
AudioConnection         ch00WaveformToEnvelope4(waveformCh[0][4], 0, envelopeCh[0][4], 0);
AudioConnection         ch00WaveformToEnvelope5(waveformCh[0][5], 0, envelopeCh[0][5], 0);
AudioConnection         ch00WaveformToEnvelope6(waveformCh[0][6], 0, envelopeCh[0][6], 0);
AudioConnection         ch00WaveformToEnvelope7(waveformCh[0][7], 0, envelopeCh[0][7], 0);
AudioConnection         ch01WaveformToEnvelope0(waveformCh[1][0], 0, envelopeCh[1][0], 0);
AudioConnection         ch01WaveformToEnvelope1(waveformCh[1][1], 0, envelopeCh[1][1], 0);
AudioConnection         ch01WaveformToEnvelope2(waveformCh[1][2], 0, envelopeCh[1][2], 0);
AudioConnection         ch01WaveformToEnvelope3(waveformCh[1][3], 0, envelopeCh[1][3], 0);
AudioConnection         ch01WaveformToEnvelope4(waveformCh[1][4], 0, envelopeCh[1][4], 0);
AudioConnection         ch01WaveformToEnvelope5(waveformCh[1][5], 0, envelopeCh[1][5], 0);
AudioConnection         ch01WaveformToEnvelope6(waveformCh[1][6], 0, envelopeCh[1][6], 0);
AudioConnection         ch01WaveformToEnvelope7(waveformCh[1][7], 0, envelopeCh[1][7], 0);
AudioConnection         ch02WaveformToEnvelope0(waveformCh[2][0], 0, envelopeCh[2][0], 0);
AudioConnection         ch02WaveformToEnvelope1(waveformCh[2][1], 0, envelopeCh[2][1], 0);
AudioConnection         ch02WaveformToEnvelope2(waveformCh[2][2], 0, envelopeCh[2][2], 0);
AudioConnection         ch02WaveformToEnvelope3(waveformCh[2][3], 0, envelopeCh[2][3], 0);
AudioConnection         ch02WaveformToEnvelope4(waveformCh[2][4], 0, envelopeCh[2][4], 0);
AudioConnection         ch02WaveformToEnvelope5(waveformCh[2][5], 0, envelopeCh[2][5], 0);
AudioConnection         ch02WaveformToEnvelope6(waveformCh[2][6], 0, envelopeCh[2][6], 0);
AudioConnection         ch02WaveformToEnvelope7(waveformCh[2][7], 0, envelopeCh[2][7], 0);
AudioConnection         ch03WaveformToEnvelope0(waveformCh[3][0], 0, envelopeCh[3][0], 0);
AudioConnection         ch03WaveformToEnvelope1(waveformCh[3][1], 0, envelopeCh[3][1], 0);
AudioConnection         ch03WaveformToEnvelope2(waveformCh[3][2], 0, envelopeCh[3][2], 0);
AudioConnection         ch03WaveformToEnvelope3(waveformCh[3][3], 0, envelopeCh[3][3], 0);
AudioConnection         ch03WaveformToEnvelope4(waveformCh[3][4], 0, envelopeCh[3][4], 0);
AudioConnection         ch03WaveformToEnvelope5(waveformCh[3][5], 0, envelopeCh[3][5], 0);
AudioConnection         ch03WaveformToEnvelope6(waveformCh[3][6], 0, envelopeCh[3][6], 0);
AudioConnection         ch03WaveformToEnvelope7(waveformCh[3][7], 0, envelopeCh[3][7], 0);
AudioConnection         ch04WaveformToEnvelope0(waveformCh[4][0], 0, envelopeCh[4][0], 0);
AudioConnection         ch04WaveformToEnvelope1(waveformCh[4][1], 0, envelopeCh[4][1], 0);
AudioConnection         ch04WaveformToEnvelope2(waveformCh[4][2], 0, envelopeCh[4][2], 0);
AudioConnection         ch04WaveformToEnvelope3(waveformCh[4][3], 0, envelopeCh[4][3], 0);
AudioConnection         ch04WaveformToEnvelope4(waveformCh[4][4], 0, envelopeCh[4][4], 0);
AudioConnection         ch04WaveformToEnvelope5(waveformCh[4][5], 0, envelopeCh[4][5], 0);
AudioConnection         ch04WaveformToEnvelope6(waveformCh[4][6], 0, envelopeCh[4][6], 0);
AudioConnection         ch04WaveformToEnvelope7(waveformCh[4][7], 0, envelopeCh[4][7], 0);
AudioConnection         ch05WaveformToEnvelope0(waveformCh[5][0], 0, envelopeCh[5][0], 0);
AudioConnection         ch05WaveformToEnvelope1(waveformCh[5][1], 0, envelopeCh[5][1], 0);
AudioConnection         ch05WaveformToEnvelope2(waveformCh[5][2], 0, envelopeCh[5][2], 0);
AudioConnection         ch05WaveformToEnvelope3(waveformCh[5][3], 0, envelopeCh[5][3], 0);
AudioConnection         ch05WaveformToEnvelope4(waveformCh[5][4], 0, envelopeCh[5][4], 0);
AudioConnection         ch05WaveformToEnvelope5(waveformCh[5][5], 0, envelopeCh[5][5], 0);
AudioConnection         ch05WaveformToEnvelope6(waveformCh[5][6], 0, envelopeCh[5][6], 0);
AudioConnection         ch05WaveformToEnvelope7(waveformCh[5][7], 0, envelopeCh[5][7], 0);
AudioConnection         ch06WaveformToEnvelope0(waveformCh[6][0], 0, envelopeCh[6][0], 0);
AudioConnection         ch06WaveformToEnvelope1(waveformCh[6][1], 0, envelopeCh[6][1], 0);
AudioConnection         ch06WaveformToEnvelope2(waveformCh[6][2], 0, envelopeCh[6][2], 0);
AudioConnection         ch06WaveformToEnvelope3(waveformCh[6][3], 0, envelopeCh[6][3], 0);
AudioConnection         ch06WaveformToEnvelope4(waveformCh[6][4], 0, envelopeCh[6][4], 0);
AudioConnection         ch06WaveformToEnvelope5(waveformCh[6][5], 0, envelopeCh[6][5], 0);
AudioConnection         ch06WaveformToEnvelope6(waveformCh[6][6], 0, envelopeCh[6][6], 0);
AudioConnection         ch06WaveformToEnvelope7(waveformCh[6][7], 0, envelopeCh[6][7], 0);
AudioConnection         ch07WaveformToEnvelope0(waveformCh[7][0], 0, envelopeCh[7][0], 0);
AudioConnection         ch07WaveformToEnvelope1(waveformCh[7][1], 0, envelopeCh[7][1], 0);
AudioConnection         ch07WaveformToEnvelope2(waveformCh[7][2], 0, envelopeCh[7][2], 0);
AudioConnection         ch07WaveformToEnvelope3(waveformCh[7][3], 0, envelopeCh[7][3], 0);
AudioConnection         ch07WaveformToEnvelope4(waveformCh[7][4], 0, envelopeCh[7][4], 0);
AudioConnection         ch07WaveformToEnvelope5(waveformCh[7][5], 0, envelopeCh[7][5], 0);
AudioConnection         ch07WaveformToEnvelope6(waveformCh[7][6], 0, envelopeCh[7][6], 0);
AudioConnection         ch07WaveformToEnvelope7(waveformCh[7][7], 0, envelopeCh[7][7], 0);
AudioConnection         ch08WaveformToEnvelope0(waveformCh[8][0], 0, envelopeCh[8][0], 0);
AudioConnection         ch08WaveformToEnvelope1(waveformCh[8][1], 0, envelopeCh[8][1], 0);
AudioConnection         ch08WaveformToEnvelope2(waveformCh[8][2], 0, envelopeCh[8][2], 0);
AudioConnection         ch08WaveformToEnvelope3(waveformCh[8][3], 0, envelopeCh[8][3], 0);
AudioConnection         ch08WaveformToEnvelope4(waveformCh[8][4], 0, envelopeCh[8][4], 0);
AudioConnection         ch08WaveformToEnvelope5(waveformCh[8][5], 0, envelopeCh[8][5], 0);
AudioConnection         ch08WaveformToEnvelope6(waveformCh[8][6], 0, envelopeCh[8][6], 0);
AudioConnection         ch08WaveformToEnvelope7(waveformCh[8][7], 0, envelopeCh[8][7], 0);
AudioConnection         ch09WaveformToEnvelope0(waveformCh[9][0], 0, envelopeCh[9][0], 0);
AudioConnection         ch09WaveformToEnvelope1(waveformCh[9][1], 0, envelopeCh[9][1], 0);
AudioConnection         ch09WaveformToEnvelope2(waveformCh[9][2], 0, envelopeCh[9][2], 0);
AudioConnection         ch09WaveformToEnvelope3(waveformCh[9][3], 0, envelopeCh[9][3], 0);
AudioConnection         ch09WaveformToEnvelope4(waveformCh[9][4], 0, envelopeCh[9][4], 0);
AudioConnection         ch09WaveformToEnvelope5(waveformCh[9][5], 0, envelopeCh[9][5], 0);
AudioConnection         ch09WaveformToEnvelope6(waveformCh[9][6], 0, envelopeCh[9][6], 0);
AudioConnection         ch09WaveformToEnvelope7(waveformCh[9][7], 0, envelopeCh[9][7], 0);
AudioConnection         ch10WaveformToEnvelope0(waveformCh[10][0], 0, envelopeCh[10][0], 0);
AudioConnection         ch10WaveformToEnvelope1(waveformCh[10][1], 0, envelopeCh[10][1], 0);
AudioConnection         ch10WaveformToEnvelope2(waveformCh[10][2], 0, envelopeCh[10][2], 0);
AudioConnection         ch10WaveformToEnvelope3(waveformCh[10][3], 0, envelopeCh[10][3], 0);
AudioConnection         ch10WaveformToEnvelope4(waveformCh[10][4], 0, envelopeCh[10][4], 0);
AudioConnection         ch10WaveformToEnvelope5(waveformCh[10][5], 0, envelopeCh[10][5], 0);
AudioConnection         ch10WaveformToEnvelope6(waveformCh[10][6], 0, envelopeCh[10][6], 0);
AudioConnection         ch10WaveformToEnvelope7(waveformCh[10][7], 0, envelopeCh[10][7], 0);
AudioConnection         ch11WaveformToEnvelope0(waveformCh[11][0], 0, envelopeCh[11][0], 0);
AudioConnection         ch11WaveformToEnvelope1(waveformCh[11][1], 0, envelopeCh[11][1], 0);
AudioConnection         ch11WaveformToEnvelope2(waveformCh[11][2], 0, envelopeCh[11][2], 0);
AudioConnection         ch11WaveformToEnvelope3(waveformCh[11][3], 0, envelopeCh[11][3], 0);
AudioConnection         ch11WaveformToEnvelope4(waveformCh[11][4], 0, envelopeCh[11][4], 0);
AudioConnection         ch11WaveformToEnvelope5(waveformCh[11][5], 0, envelopeCh[11][5], 0);
AudioConnection         ch11WaveformToEnvelope6(waveformCh[11][6], 0, envelopeCh[11][6], 0);
AudioConnection         ch11WaveformToEnvelope7(waveformCh[11][7], 0, envelopeCh[11][7], 0);
AudioConnection         ch12WaveformToEnvelope0(waveformCh[12][0], 0, envelopeCh[12][0], 0);
AudioConnection         ch12WaveformToEnvelope1(waveformCh[12][1], 0, envelopeCh[12][1], 0);
AudioConnection         ch12WaveformToEnvelope2(waveformCh[12][2], 0, envelopeCh[12][2], 0);
AudioConnection         ch12WaveformToEnvelope3(waveformCh[12][3], 0, envelopeCh[12][3], 0);
AudioConnection         ch12WaveformToEnvelope4(waveformCh[12][4], 0, envelopeCh[12][4], 0);
AudioConnection         ch12WaveformToEnvelope5(waveformCh[12][5], 0, envelopeCh[12][5], 0);
AudioConnection         ch12WaveformToEnvelope6(waveformCh[12][6], 0, envelopeCh[12][6], 0);
AudioConnection         ch12WaveformToEnvelope7(waveformCh[12][7], 0, envelopeCh[12][7], 0);
AudioConnection         ch13WaveformToEnvelope0(waveformCh[13][0], 0, envelopeCh[13][0], 0);
AudioConnection         ch13WaveformToEnvelope1(waveformCh[13][1], 0, envelopeCh[13][1], 0);
AudioConnection         ch13WaveformToEnvelope2(waveformCh[13][2], 0, envelopeCh[13][2], 0);
AudioConnection         ch13WaveformToEnvelope3(waveformCh[13][3], 0, envelopeCh[13][3], 0);
AudioConnection         ch13WaveformToEnvelope4(waveformCh[13][4], 0, envelopeCh[13][4], 0);
AudioConnection         ch13WaveformToEnvelope5(waveformCh[13][5], 0, envelopeCh[13][5], 0);
AudioConnection         ch13WaveformToEnvelope6(waveformCh[13][6], 0, envelopeCh[13][6], 0);
AudioConnection         ch13WaveformToEnvelope7(waveformCh[13][7], 0, envelopeCh[13][7], 0);
AudioConnection         ch14WaveformToEnvelope0(waveformCh[14][0], 0, envelopeCh[14][0], 0);
AudioConnection         ch14WaveformToEnvelope1(waveformCh[14][1], 0, envelopeCh[14][1], 0);
AudioConnection         ch14WaveformToEnvelope2(waveformCh[14][2], 0, envelopeCh[14][2], 0);
AudioConnection         ch14WaveformToEnvelope3(waveformCh[14][3], 0, envelopeCh[14][3], 0);
AudioConnection         ch14WaveformToEnvelope4(waveformCh[14][4], 0, envelopeCh[14][4], 0);
AudioConnection         ch14WaveformToEnvelope5(waveformCh[14][5], 0, envelopeCh[14][5], 0);
AudioConnection         ch14WaveformToEnvelope6(waveformCh[14][6], 0, envelopeCh[14][6], 0);
AudioConnection         ch14WaveformToEnvelope7(waveformCh[14][7], 0, envelopeCh[14][7], 0);
AudioConnection         ch15WaveformToEnvelope0(waveformCh[15][0], 0, envelopeCh[15][0], 0);
AudioConnection         ch15WaveformToEnvelope1(waveformCh[15][1], 0, envelopeCh[15][1], 0);
AudioConnection         ch15WaveformToEnvelope2(waveformCh[15][2], 0, envelopeCh[15][2], 0);
AudioConnection         ch15WaveformToEnvelope3(waveformCh[15][3], 0, envelopeCh[15][3], 0);
AudioConnection         ch15WaveformToEnvelope4(waveformCh[15][4], 0, envelopeCh[15][4], 0);
AudioConnection         ch15WaveformToEnvelope5(waveformCh[15][5], 0, envelopeCh[15][5], 0);
AudioConnection         ch15WaveformToEnvelope6(waveformCh[15][6], 0, envelopeCh[15][6], 0);
AudioConnection         ch15WaveformToEnvelope7(waveformCh[15][7], 0, envelopeCh[15][7], 0);
AudioConnection         ch00envelopeToMixer0(envelopeCh[0][0], 0, firstMixer[0], 0);
AudioConnection         ch00envelopeToMixer1(envelopeCh[0][1], 0, firstMixer[0], 1);
AudioConnection         ch00envelopeToMixer2(envelopeCh[0][2], 0, firstMixer[0], 2);
AudioConnection         ch00envelopeToMixer3(envelopeCh[0][3], 0, firstMixer[0], 3);
AudioConnection         ch00envelopeToMixer4(envelopeCh[0][4], 0, firstMixer[1], 0);
AudioConnection         ch00envelopeToMixer5(envelopeCh[0][5], 0, firstMixer[1], 1);
AudioConnection         ch00envelopeToMixer6(envelopeCh[0][6], 0, firstMixer[1], 2);
AudioConnection         ch00envelopeToMixer7(envelopeCh[0][7], 0, firstMixer[1], 3);
AudioConnection         ch01envelopeToMixer0(envelopeCh[1][0], 0, firstMixer[2], 0);
AudioConnection         ch01envelopeToMixer1(envelopeCh[1][1], 0, firstMixer[2], 1);
AudioConnection         ch01envelopeToMixer2(envelopeCh[1][2], 0, firstMixer[2], 2);
AudioConnection         ch01envelopeToMixer3(envelopeCh[1][3], 0, firstMixer[2], 3);
AudioConnection         ch01envelopeToMixer4(envelopeCh[1][4], 0, firstMixer[3], 0);
AudioConnection         ch01envelopeToMixer5(envelopeCh[1][5], 0, firstMixer[3], 1);
AudioConnection         ch01envelopeToMixer6(envelopeCh[1][6], 0, firstMixer[3], 2);
AudioConnection         ch01envelopeToMixer7(envelopeCh[1][7], 0, firstMixer[3], 3);
AudioConnection         ch02envelopeToMixer0(envelopeCh[2][0], 0, firstMixer[4], 0);
AudioConnection         ch02envelopeToMixer1(envelopeCh[2][1], 0, firstMixer[4], 1);
AudioConnection         ch02envelopeToMixer2(envelopeCh[2][2], 0, firstMixer[4], 2);
AudioConnection         ch02envelopeToMixer3(envelopeCh[2][3], 0, firstMixer[4], 3);
AudioConnection         ch02envelopeToMixer4(envelopeCh[2][4], 0, firstMixer[5], 0);
AudioConnection         ch02envelopeToMixer5(envelopeCh[2][5], 0, firstMixer[5], 1);
AudioConnection         ch02envelopeToMixer6(envelopeCh[2][6], 0, firstMixer[5], 2);
AudioConnection         ch02envelopeToMixer7(envelopeCh[2][7], 0, firstMixer[5], 3);
AudioConnection         ch03envelopeToMixer0(envelopeCh[3][0], 0, firstMixer[6], 0);
AudioConnection         ch03envelopeToMixer1(envelopeCh[3][1], 0, firstMixer[6], 1);
AudioConnection         ch03envelopeToMixer2(envelopeCh[3][2], 0, firstMixer[6], 2);
AudioConnection         ch03envelopeToMixer3(envelopeCh[3][3], 0, firstMixer[6], 3);
AudioConnection         ch03envelopeToMixer4(envelopeCh[3][4], 0, firstMixer[7], 0);
AudioConnection         ch03envelopeToMixer5(envelopeCh[3][5], 0, firstMixer[7], 1);
AudioConnection         ch03envelopeToMixer6(envelopeCh[3][6], 0, firstMixer[7], 2);
AudioConnection         ch03envelopeToMixer7(envelopeCh[3][7], 0, firstMixer[7], 3);
AudioConnection         ch04envelopeToMixer0(envelopeCh[4][0], 0, firstMixer[8], 0);
AudioConnection         ch04envelopeToMixer1(envelopeCh[4][1], 0, firstMixer[8], 1);
AudioConnection         ch04envelopeToMixer2(envelopeCh[4][2], 0, firstMixer[8], 2);
AudioConnection         ch04envelopeToMixer3(envelopeCh[4][3], 0, firstMixer[8], 3);
AudioConnection         ch04envelopeToMixer4(envelopeCh[4][4], 0, firstMixer[9], 0);
AudioConnection         ch04envelopeToMixer5(envelopeCh[4][5], 0, firstMixer[9], 1);
AudioConnection         ch04envelopeToMixer6(envelopeCh[4][6], 0, firstMixer[9], 2);
AudioConnection         ch04envelopeToMixer7(envelopeCh[4][7], 0, firstMixer[9], 3);
AudioConnection         ch05envelopeToMixer0(envelopeCh[5][0], 0, firstMixer[10], 0);
AudioConnection         ch05envelopeToMixer1(envelopeCh[5][1], 0, firstMixer[10], 1);
AudioConnection         ch05envelopeToMixer2(envelopeCh[5][2], 0, firstMixer[10], 2);
AudioConnection         ch05envelopeToMixer3(envelopeCh[5][3], 0, firstMixer[10], 3);
AudioConnection         ch05envelopeToMixer4(envelopeCh[5][4], 0, firstMixer[11], 0);
AudioConnection         ch05envelopeToMixer5(envelopeCh[5][5], 0, firstMixer[11], 1);
AudioConnection         ch05envelopeToMixer6(envelopeCh[5][6], 0, firstMixer[11], 2);
AudioConnection         ch05envelopeToMixer7(envelopeCh[5][7], 0, firstMixer[11], 3);
AudioConnection         ch06envelopeToMixer0(envelopeCh[6][0], 0, firstMixer[12], 0);
AudioConnection         ch06envelopeToMixer1(envelopeCh[6][1], 0, firstMixer[12], 1);
AudioConnection         ch06envelopeToMixer2(envelopeCh[6][2], 0, firstMixer[12], 2);
AudioConnection         ch06envelopeToMixer3(envelopeCh[6][3], 0, firstMixer[12], 3);
AudioConnection         ch06envelopeToMixer4(envelopeCh[6][4], 0, firstMixer[13], 0);
AudioConnection         ch06envelopeToMixer5(envelopeCh[6][5], 0, firstMixer[13], 1);
AudioConnection         ch06envelopeToMixer6(envelopeCh[6][6], 0, firstMixer[13], 2);
AudioConnection         ch06envelopeToMixer7(envelopeCh[6][7], 0, firstMixer[13], 3);
AudioConnection         ch07envelopeToMixer0(envelopeCh[7][0], 0, firstMixer[14], 0);
AudioConnection         ch07envelopeToMixer1(envelopeCh[7][1], 0, firstMixer[14], 1);
AudioConnection         ch07envelopeToMixer2(envelopeCh[7][2], 0, firstMixer[14], 2);
AudioConnection         ch07envelopeToMixer3(envelopeCh[7][3], 0, firstMixer[14], 3);
AudioConnection         ch07envelopeToMixer4(envelopeCh[7][4], 0, firstMixer[15], 0);
AudioConnection         ch07envelopeToMixer5(envelopeCh[7][5], 0, firstMixer[15], 1);
AudioConnection         ch07envelopeToMixer6(envelopeCh[7][6], 0, firstMixer[15], 2);
AudioConnection         ch07envelopeToMixer7(envelopeCh[7][7], 0, firstMixer[15], 3);
AudioConnection         ch08envelopeToMixer0(envelopeCh[8][0], 0, firstMixer[16], 0);
AudioConnection         ch08envelopeToMixer1(envelopeCh[8][1], 0, firstMixer[16], 1);
AudioConnection         ch08envelopeToMixer2(envelopeCh[8][2], 0, firstMixer[16], 2);
AudioConnection         ch08envelopeToMixer3(envelopeCh[8][3], 0, firstMixer[16], 3);
AudioConnection         ch08envelopeToMixer4(envelopeCh[8][4], 0, firstMixer[17], 0);
AudioConnection         ch08envelopeToMixer5(envelopeCh[8][5], 0, firstMixer[17], 1);
AudioConnection         ch08envelopeToMixer6(envelopeCh[8][6], 0, firstMixer[17], 2);
AudioConnection         ch08envelopeToMixer7(envelopeCh[8][7], 0, firstMixer[17], 3);
AudioConnection         ch09envelopeToMixer0(envelopeCh[9][0], 0, firstMixer[18], 0);
AudioConnection         ch09envelopeToMixer1(envelopeCh[9][1], 0, firstMixer[18], 1);
AudioConnection         ch09envelopeToMixer2(envelopeCh[9][2], 0, firstMixer[18], 2);
AudioConnection         ch09envelopeToMixer3(envelopeCh[9][3], 0, firstMixer[18], 3);
AudioConnection         ch09envelopeToMixer4(envelopeCh[9][4], 0, firstMixer[19], 0);
AudioConnection         ch09envelopeToMixer5(envelopeCh[9][5], 0, firstMixer[19], 1);
AudioConnection         ch09envelopeToMixer6(envelopeCh[9][6], 0, firstMixer[19], 2);
AudioConnection         ch09envelopeToMixer7(envelopeCh[9][7], 0, firstMixer[19], 3);
AudioConnection         ch10envelopeToMixer0(envelopeCh[10][0], 0, firstMixer[20], 0);
AudioConnection         ch10envelopeToMixer1(envelopeCh[10][1], 0, firstMixer[20], 1);
AudioConnection         ch10envelopeToMixer2(envelopeCh[10][2], 0, firstMixer[20], 2);
AudioConnection         ch10envelopeToMixer3(envelopeCh[10][3], 0, firstMixer[20], 3);
AudioConnection         ch10envelopeToMixer4(envelopeCh[10][4], 0, firstMixer[21], 0);
AudioConnection         ch10envelopeToMixer5(envelopeCh[10][5], 0, firstMixer[21], 1);
AudioConnection         ch10envelopeToMixer6(envelopeCh[10][6], 0, firstMixer[21], 2);
AudioConnection         ch10envelopeToMixer7(envelopeCh[10][7], 0, firstMixer[21], 3);
AudioConnection         ch11envelopeToMixer0(envelopeCh[11][0], 0, firstMixer[22], 0);
AudioConnection         ch11envelopeToMixer1(envelopeCh[11][1], 0, firstMixer[22], 1);
AudioConnection         ch11envelopeToMixer2(envelopeCh[11][2], 0, firstMixer[22], 2);
AudioConnection         ch11envelopeToMixer3(envelopeCh[11][3], 0, firstMixer[22], 3);
AudioConnection         ch11envelopeToMixer4(envelopeCh[11][4], 0, firstMixer[23], 0);
AudioConnection         ch11envelopeToMixer5(envelopeCh[11][5], 0, firstMixer[23], 1);
AudioConnection         ch11envelopeToMixer6(envelopeCh[11][6], 0, firstMixer[23], 2);
AudioConnection         ch11envelopeToMixer7(envelopeCh[11][7], 0, firstMixer[23], 3);
AudioConnection         ch12envelopeToMixer0(envelopeCh[12][0], 0, firstMixer[24], 0);
AudioConnection         ch12envelopeToMixer1(envelopeCh[12][1], 0, firstMixer[24], 1);
AudioConnection         ch12envelopeToMixer2(envelopeCh[12][2], 0, firstMixer[24], 2);
AudioConnection         ch12envelopeToMixer3(envelopeCh[12][3], 0, firstMixer[24], 3);
AudioConnection         ch12envelopeToMixer4(envelopeCh[12][4], 0, firstMixer[25], 0);
AudioConnection         ch12envelopeToMixer5(envelopeCh[12][5], 0, firstMixer[25], 1);
AudioConnection         ch12envelopeToMixer6(envelopeCh[12][6], 0, firstMixer[25], 2);
AudioConnection         ch12envelopeToMixer7(envelopeCh[12][7], 0, firstMixer[25], 3);
AudioConnection         ch13envelopeToMixer0(envelopeCh[13][0], 0, firstMixer[26], 0);
AudioConnection         ch13envelopeToMixer1(envelopeCh[13][1], 0, firstMixer[26], 1);
AudioConnection         ch13envelopeToMixer2(envelopeCh[13][2], 0, firstMixer[26], 2);
AudioConnection         ch13envelopeToMixer3(envelopeCh[13][3], 0, firstMixer[26], 3);
AudioConnection         ch13envelopeToMixer4(envelopeCh[13][4], 0, firstMixer[27], 0);
AudioConnection         ch13envelopeToMixer5(envelopeCh[13][5], 0, firstMixer[27], 1);
AudioConnection         ch13envelopeToMixer6(envelopeCh[13][6], 0, firstMixer[27], 2);
AudioConnection         ch13envelopeToMixer7(envelopeCh[13][7], 0, firstMixer[27], 3);
AudioConnection         ch14envelopeToMixer0(envelopeCh[14][0], 0, firstMixer[28], 0);
AudioConnection         ch14envelopeToMixer1(envelopeCh[14][1], 0, firstMixer[28], 1);
AudioConnection         ch14envelopeToMixer2(envelopeCh[14][2], 0, firstMixer[28], 2);
AudioConnection         ch14envelopeToMixer3(envelopeCh[14][3], 0, firstMixer[28], 3);
AudioConnection         ch14envelopeToMixer4(envelopeCh[14][4], 0, firstMixer[29], 0);
AudioConnection         ch14envelopeToMixer5(envelopeCh[14][5], 0, firstMixer[29], 1);
AudioConnection         ch14envelopeToMixer6(envelopeCh[14][6], 0, firstMixer[29], 2);
AudioConnection         ch14envelopeToMixer7(envelopeCh[14][7], 0, firstMixer[29], 3);
AudioConnection         ch15envelopeToMixer0(envelopeCh[15][0], 0, firstMixer[30], 0);
AudioConnection         ch15envelopeToMixer1(envelopeCh[15][1], 0, firstMixer[30], 1);
AudioConnection         ch15envelopeToMixer2(envelopeCh[15][2], 0, firstMixer[30], 2);
AudioConnection         ch15envelopeToMixer3(envelopeCh[15][3], 0, firstMixer[30], 3);
AudioConnection         ch15envelopeToMixer4(envelopeCh[15][4], 0, firstMixer[31], 0);
AudioConnection         ch15envelopeToMixer5(envelopeCh[15][5], 0, firstMixer[31], 1);
AudioConnection         ch15envelopeToMixer6(envelopeCh[15][6], 0, firstMixer[31], 2);
AudioConnection         ch15envelopeToMixer7(envelopeCh[15][7], 0, firstMixer[31], 3);
AudioConnection         firstMixerToSecondMixer00(firstMixer[0], 0, secondMixer[0], 0);
AudioConnection         firstMixerToSecondMixer01(firstMixer[1], 0, secondMixer[0], 1);
AudioConnection         firstMixerToSecondMixer02(firstMixer[2], 0, secondMixer[0], 2);
AudioConnection         firstMixerToSecondMixer03(firstMixer[3], 0, secondMixer[0], 3);
AudioConnection         firstMixerToSecondMixer04(firstMixer[4], 0, secondMixer[1], 0);
AudioConnection         firstMixerToSecondMixer05(firstMixer[5], 0, secondMixer[1], 1);
AudioConnection         firstMixerToSecondMixer06(firstMixer[6], 0, secondMixer[1], 2);
AudioConnection         firstMixerToSecondMixer07(firstMixer[7], 0, secondMixer[1], 3);
AudioConnection         firstMixerToSecondMixer08(firstMixer[8], 0, secondMixer[2], 0);
AudioConnection         firstMixerToSecondMixer09(firstMixer[9], 0, secondMixer[2], 1);
AudioConnection         firstMixerToSecondMixer10(firstMixer[10], 0, secondMixer[2], 2);
AudioConnection         firstMixerToSecondMixer11(firstMixer[11], 0, secondMixer[2], 3);
AudioConnection         firstMixerToSecondMixer12(firstMixer[12], 0, secondMixer[3], 0);
AudioConnection         firstMixerToSecondMixer13(firstMixer[13], 0, secondMixer[3], 1);
AudioConnection         firstMixerToSecondMixer14(firstMixer[14], 0, secondMixer[3], 2);
AudioConnection         firstMixerToSecondMixer15(firstMixer[15], 0, secondMixer[3], 3);
AudioConnection         firstMixerToSecondMixer16(firstMixer[16], 0, secondMixer[4], 0);
AudioConnection         firstMixerToSecondMixer17(firstMixer[17], 0, secondMixer[4], 1);
AudioConnection         firstMixerToSecondMixer18(firstMixer[18], 0, secondMixer[4], 2);
AudioConnection         firstMixerToSecondMixer19(firstMixer[19], 0, secondMixer[4], 3);
AudioConnection         firstMixerToSecondMixer20(firstMixer[20], 0, secondMixer[5], 0);
AudioConnection         firstMixerToSecondMixer21(firstMixer[21], 0, secondMixer[5], 1);
AudioConnection         firstMixerToSecondMixer22(firstMixer[22], 0, secondMixer[5], 2);
AudioConnection         firstMixerToSecondMixer23(firstMixer[23], 0, secondMixer[5], 3);
AudioConnection         firstMixerToSecondMixer24(firstMixer[24], 0, secondMixer[6], 0);
AudioConnection         firstMixerToSecondMixer25(firstMixer[25], 0, secondMixer[6], 1);
AudioConnection         firstMixerToSecondMixer26(firstMixer[26], 0, secondMixer[6], 2);
AudioConnection         firstMixerToSecondMixer27(firstMixer[27], 0, secondMixer[6], 3);
AudioConnection         firstMixerToSecondMixer28(firstMixer[28], 0, secondMixer[7], 0);
AudioConnection         firstMixerToSecondMixer29(firstMixer[29], 0, secondMixer[7], 1);
AudioConnection         firstMixerToSecondMixer30(firstMixer[30], 0, secondMixer[7], 2);
AudioConnection         firstMixerToSecondMixer31(firstMixer[31], 0, secondMixer[7], 3);
AudioConnection         secondMixerToThirdMixer0(secondMixer[0], 0, thirdMixer[0], 0);
AudioConnection         secondMixerToThirdMixer1(secondMixer[1], 0, thirdMixer[0], 1);
AudioConnection         secondMixerToThirdMixer2(secondMixer[2], 0, thirdMixer[0], 2);
AudioConnection         secondMixerToThirdMixer3(secondMixer[3], 0, thirdMixer[0], 3);
AudioConnection         secondMixerToThirdMixer4(secondMixer[4], 0, thirdMixer[1], 0);
AudioConnection         secondMixerToThirdMixer5(secondMixer[5], 0, thirdMixer[1], 1);
AudioConnection         secondMixerToThirdMixer6(secondMixer[6], 0, thirdMixer[1], 2);
AudioConnection         secondMixerToThirdMixer7(secondMixer[7], 0, thirdMixer[1], 3);
AudioConnection         thirdMixerToOutMixer0(thirdMixer[0], 0, outMixer, 0);
AudioConnection         thirdMixerToOutMixer1(thirdMixer[1], 0, outMixer, 1);
AudioOutputMQS          mqs;
AudioConnection         outMixerToMqs0(outMixer, 0, mqs, 0);

//------------------------------------------------------------------------------------------------------------------------------------------------------------//
// START OF SETUP SECTION
void setup()
{

    if (deckType == 1)
    {
        velocityDetectionEnabled = 1;
    }

    AudioMemory(20);

    // // 440Hz sine wave test
    // waveformCh[0][0].begin(0.3, 440, WAVEFORM_SINE);
    // envelopeCh[0][0].noteOn();

    // // Test of maximum normal output:
    // for (int i = 0; i < 6; i++)
    // {
    //     waveformCh[0][i].begin(0.3 , 0, WAVEFORM_SINE);
    //     envelopeCh[0][i].noteOn();
    // }

    // waveformCh[0][0].frequency(130.81); // C3
    // waveformCh[0][1].frequency(261.63); // C4
    // waveformCh[0][2].frequency(329.69); // E4
    // waveformCh[0][3].frequency(391.96); // G4
    // waveformCh[0][4].frequency(523.25); // C5
    // waveformCh[0][5].frequency(659.26); // E5

    // Enable serial for MIDI DIN connector
    Serial1.begin(31250, SERIAL_8N1);                                           // Serial MIDI specification: 31250 baud, 8 data bits, no parity, 1 stop bit

    // Set pinModes for the digital button matrix
    for (int pinNumber = 0; pinNumber < columnCount; pinNumber++)               // For each keyswitch column pin...
    {
        pinMode(keyswitchColumnPins[pinNumber], OUTPUT);                        // set the pinMode to OUTPUT...
        digitalWrite(keyswitchColumnPins[pinNumber], HIGH);                     // and default to HIGH.  Individual columns will toggle LOW while being scanned.
    }
    for (int pinNumber = 0; pinNumber < columnCount; pinNumber++)               // For each tact switch column pin...
    {
        pinMode(tactSwitchColumnPins[pinNumber], OUTPUT);                       // set the pinMode to OUTPUT ...
        digitalWrite(tactSwitchColumnPins[pinNumber], HIGH);                    // and default to HIGH.  Individual columns will toggle LOW while being scanned.
    }
    for (int pinNumber = 0; pinNumber < rowCount; pinNumber++)                  // For each row pin...
    {
        pinMode(rowPins[pinNumber], INPUT_PULLUP);                              // Set the pinMode to INPUT_PULLUP.  If this line goes LOW while scanning,
    }                                                                           // then we know the key under the associated column is being held.

    // Set pinModes for analog inputs
    pinMode(topPotPin, INPUT);
    pinMode(bottomPotPin, INPUT);

    // Set pinMode for rotary encoder button
    pinMode(rotaryEncoderButtonPin, INPUT_PULLUP);

    // Set pinMode for foot pedal
    pinMode(footPedalPin, INPUT_PULLDOWN);

    // OLED setup
    Wire.begin();                                                               // Init I2C
    Wire.setClock(400000L);                                                     // Fast mode
    oled.begin(&Adafruit128x64, I2C_ADDRESS);                                   // OLED type and address
    oled.setFont(font5x7);                                                      // Set the font type (https://github.com/greiman/SSD1306Ascii/blob/master/doc/5x7fonts.pdf)
    oled.setContrast(0);                                                        // Lower the contrast (set to 0 as my cheap OLED only ranges from way-too-bright to acceptable brightness level)

    // Display program build info and logo
    oled.clear();
    // Print version info
    for (int i = 0; i < 21; i++)                                                // Line 0
    {
        oled.setCursor(i*6,0);                                                  // Default character-set is 5 pixels wide plus a 1 pixel gap between letters
        oled.print(version[i]);                                                 // Print version info (variable definition at document head for ease of access)
    }

    for (int i = 0; i < 26; i++)
    {
        oled.setCursor(i*5,1);                                                  // Print every 5 pixels instead of 6 to create a solid line
        oled.print("-");
    }

    // Print logo
    const char startupAnimation2[]   = "|-      --";                  // Unicode DELETE character (U+007F) renders as a solid box using "font5x7"
    const char startupAnimation3[]   = "|-         --";
    const char startupAnimation4[]   = "|-        --";
    const char startupAnimation5[]   = "|-         ---";
    const char startupAnimation6[]   = "|-       ---";
    const char startupAnimation7[]   = "INSTRUMENTS";

    for (int i = 0; i < 21; i++)                                                // Lines 2-6
    {
        oled.setCursor(i*6,2);
        oled.print(startupAnimation2[i]);
        oled.setCursor(i*6,3);
        oled.print(startupAnimation3[i]);
        oled.setCursor(i*6,4);
        oled.print(startupAnimation4[i]);
        oled.setCursor(i*6,5);
        oled.print(startupAnimation5[i]);
        oled.setCursor(i*6,6);
        oled.print(startupAnimation6[i]);
        delay(25);
    }
    for (int i = 0; i < 11; i++)                                                // Line 7
    {
        oled.setCursor(6+(i*10.9),7);                                           // Increase the letter separation to better square up with logo above
        oled.print(startupAnimation7[i]);
    }

    delay(2000);                                                                // Delay for a bit so that info can be read

    // Call the OLED display update function to init the screen for program operation
    oledUpdate();
}
// END OF SETUP SECTION
//------------------------------------------------------------------------------------------------------------------------------------------------------------//

//------------------------------------------------------------------------------------------------------------------------------------------------------------//
// START OF LOOP SECTION
void loop()
{

    // Update the rate limiting variable
    if (millis() - lastRateLimiterTimestamp >= rateLimiterTime)
    {
        lastRateLimiterTimestamp = millis();
        rateLimiterToggle = HIGH;
    }

    // Print program loop times for diagnostics
    // if (rateLimiterToggle == HIGH) { Serial.println(micros() - programLoopTimer); }
    // programLoopTimer = micros();

    // OLED screensaver
    if ((millis() - oledScreensaverTime) > 120000 && oledOnOff == HIGH)         // Turn off OLED display if no input for n milliseconds to prevent burn-in
    {
        oled.ssd1306WriteCmd(SSD1306_DISPLAYOFF);                               // Send the display off command
        oledOnOff = LOW;                                                        // Input detection is located in the digitalButtons() function
    }

    // Digital button related tasks
    digitalButtons();

    // Foot pedal related tasks
    footPedal();

    // Analog pot related tasks
    analogPots();

    // Channel select related tasks
    channelSelect();

    // Rotary encoder menu related tasks
    rotaryEncoder();

    // Hardware test - Comment this function out when not being used.
    // Make sure to also comment out the tact-switch velocity "timeout" feature in the digitalButtons() function.
    // Search for "hardwareTest()" to find the comment with instructions.
    // hardwareTest();

    // MIDI control change related tasks (pitch bend/modulation)
    if (rateLimiterToggle == HIGH) { digitalMidiCC(); }    // Rate limiting to cut down on looper packets that need to be recorded

    // Looper related tasks
    looper();

    // Controller reset related tasks
    controllerReset();

    // Send notes to the MIDI bus
    playNotes();

    // Reset input locking variables for the next program loop
    if (channelUpButton         == LOW) { previousChannelUpButton       = LOW; }
    if (channelDownButton       == LOW) { previousChannelDownButton     = LOW; }
    if (loopButton              == LOW) { previousLoopButton            = LOW; }
    if (rotaryEncoderButton     == LOW) { previousRotaryEncoderButton   = LOW; velocityDisableClock = 4294967295; }
    if (footPedalButton         == LOW) { previousFootPedalButton       = LOW; }

    // Reset the state variable indicating that a recordable input was pressed to kick off loop recording
    loopInputDetected = LOW;

    // Reset rate limiter toggle
    rateLimiterToggle = LOW;
}

// END OF LOOP SECTION
//------------------------------------------------------------------------------------------------------------------------------------------------------------//

void oledUpdate()
{
    // Initialization
    if (oledInit == HIGH)
    {
        oled.clear();
        // Print static information only once to speed up subsequent updates
        // Line 0
            // Used by MIDI program name
        // Line 1
            for (int i = 0; i < 26; i++)
            {
                oled.setCursor(i*5,1);
                oled.print("-");
            }
        // Line 2
            oled.setCursor(0,2);
            oled.print(" Bank/Prog.:");
        // Line 3
            oled.setCursor(0,3);
            oled.print(" Device Mode:");
        // Line 4
            oled.setCursor(0,4);
            oled.print(" Pedal Mode:");
        // Line 5
            oled.setCursor(0,5);
            oled.print(" Transpose:");
        // Line 6
            for (int i = 0; i < 26; i++)
            {
                oled.setCursor(i*5,6);
                oled.print("-");
            }
        // Line 7
            oled.setCursor(0,7);
            oled.print("Loop:      CH:   /   ");

        oledInit = LOW;                                                                     // Disable init for subsequent loops until called again
        oledUpdate();
    }

    // Line 0 - MIDI program/synth instrument name
        if (channelIndex != 15 && modeSelection != SYNTH)                                   // If drums channel is NOT selected
        {
            oled.setCursor(0,0);
            oled.print(programName[instrumentIndex[channelMap[channelIndex]]]);
        }
        if (channelIndex == 15 && modeSelection != SYNTH)                                   // If drums channel IS selected
        {
            oled.setCursor(0,0);
            oled.print(drumKitName[instrumentIndex[channelMap[channelIndex]]]);
        }
        if (channelIndex != 15 && modeSelection == SYNTH)                                   // If synth mode drums channel is NOT selected
        {
            oled.setCursor(0,0);
            oled.print(synthProgramName[synthInstrumentIndex[channelMap[channelIndex]]]);
        }
        if (channelIndex == 15 && modeSelection == SYNTH)                                   // If synth mode drums channel IS selected
        {
            oled.setCursor(0,0);
            oled.print(synthDrumKitName[synthInstrumentIndex[channelMap[channelIndex]]]);
        }
    // Line 1
        // --------------------
    // Line 2 - MIDI bank/program/synth instrument numeric value
        if (channelIndex != 15 && modeSelection != SYNTH)
        {
            oled.setCursor(84, 2);
            oled.printf("%03d", bankMap[instrumentIndex[channelMap[channelIndex]]]);
            oled.print("/");
            oled.printf("%03d", programMap[instrumentIndex[channelMap[channelIndex]]]);
        }
        if (channelIndex == 15 && modeSelection != SYNTH)
        {
            oled.setCursor(84, 2);
            oled.print("128/");                                                             // Drums are always bank 128
            oled.printf("%03d", drumKitMap[instrumentIndex[channelMap[channelIndex]]]);
        }
        if (modeSelection == SYNTH)
        {
            oled.setCursor(84, 2);
            oled.print("---/");                                                             // Synth has no banks, just display dashes
            oled.printf("%03d", synthInstrumentIndex[channelMap[channelIndex]]);
        }
    // Line 3 - Device mode selection
        oled.setCursor(84, 3);
        if (modeSelection == NORMAL)    { oled.print("Normal "); }
        if (modeSelection == LAYER)     { oled.print("Layer  "); }
        if (modeSelection == SPLIT)     { oled.print("Split  "); }
        if (modeSelection == AUTOSUS)   { oled.print("AutoSus"); }
        if (modeSelection == SYNTH)     { oled.print("Synth  "); }
    // Line 4 - Pedal function
        oled.setCursor(84, 4);
        if (pedalFunction == SUSTAIN)       { oled.print("Sustain"); }
        if (pedalFunction == LOOPER)        { oled.print("Looper "); }
        if (pedalFunction == MODULATION)    { oled.print("Mod CC "); }
    // Line 5 - Tranposition offset
        oled.setCursor(84, 5);
        if (transposeValue[channelMap[channelIndex]] == 0)  { oled.print("---"); }
        if (transposeValue[channelMap[channelIndex]] > 0)   { oled.print("+"); oled.printf("%02d", transposeValue[channelMap[channelIndex]]); }
        if (transposeValue[channelMap[channelIndex]] < 0)   { oled.printf("%03d", transposeValue[channelMap[channelIndex]]); }
    // Line 6
        // --------------------
    // Line 7 - Looper and channel selection
        // Loop
            if (loopTrackEnabled[channelMap[channelIndex]] == 1)                // Overdub loop lock indicators
            {
                oled.setCursor(30, 7);
                oled.print("<");
                oled.setCursor(54, 7);
                oled.print(">");
            }
            else if (loopTrackEnabled[channelMap[channelIndex]] == 0 && loopRecordingEnabled == HIGH)  // If recording is enabled and the current channel is available for recording
            {
                oled.setCursor(30, 7);
                oled.print(" REC ");
            }
            else                                                                // Clear if inactive
            {
                oled.setCursor(30, 7);
                oled.print(" ");
                oled.setCursor(54, 7);
                oled.print(" ");
            }

            if (loopPlaybackEnabled == LOW)
            {
                oled.setCursor(36, 7);
                oled.print("---");
            }

        // Channel
            if (loopTrackEnabled[channelMap[channelIndex]] == 1)                // Overdub channel lock indicators
            {
                oled.setCursor(84, 7);
                oled.print("<");
                oled.setCursor(120, 7);
                oled.print(">");
            }
            else                                                                // Clear if inactive
            {
                oled.setCursor(84, 7);
                oled.print(" ");
                oled.setCursor(120, 7);
                oled.print(" ");
            }

            oled.setCursor(90, 7);                                              // Primary channel (nn/--)
            if (modeSelection != LAYER && modeSelection != SPLIT)
            {
                oled.printf("%02d", channelMap[channelIndex]);                  // Print active channel selection
            }
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
             oled.printf("%02d", channelMap[channelIndex]);                     // Print active channel selection
            }
            oled.setCursor(108, 7);                                             // Secondary channel (--/nn)
            if (modeSelection != LAYER && modeSelection != SPLIT)
            {
                oled.print("--");
            }
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if (channelIndex % 2 == 0)                                      // If modulo is 0, i.e. an even number
                {
                    oled.printf("%02d", channelMap[channelIndex+1]);
                }
                if (channelIndex % 2 == 1)                                      // If modulo is 1, i.e. an odd number
                {
                    oled.printf("%02d", channelMap[channelIndex-1]);
                }
            }


    // Selection Cursor
        // Option column
        for (int i = 2; i < 6; i++)                                             // Clear any existing cursor data in the option menu column to prepare for update
        {
            oled.setCursor(0,i);
            oled.print(" ");
        }
        oled.setCursor(0,oledOption);                                           // Set cursor position to current menu option...
        oled.print(">");                                                        // and print the selection cursor.
        // Value column
        if (oledMenuToggle == LOW)                                              // If option menu column is active, clear any existing cursor data in the value menu column
        {
            for (int i = 2; i < 6; i++)
            {
                oled.setCursor(78,i);
                oled.print(" ");
            }
        }
        if (oledMenuToggle == HIGH)                                             // If value menu column is active, print value selection cursor
        {
            oled.setCursor(0,oledOption);
            oled.print("-");
            oled.setCursor(78,oledOption);
            oled.print(">");
        }
}


void digitalButtons()
{
    // Scan each row in a column, then increment the column index and start down the rows again.  Increment indexes on next loop, or reset when limits are reached.
    // Scanning of each node is broken out into separate program loops, as it can take pins several microseconds to fully change state,
    // and that time is far better utilized by executing program code than on unnecessary delay statements.
    if (rowIndex >= rowCount)                                                                   // If row index limit is reached
    {
        rowIndex = 0;                                                                           // Wrap around to 0
        digitalWrite(currentColumn, HIGH);                                                      // Set the now finished column pin back to HIGH and move on to the next column pin.
        columnIndex = columnIndex + 1;                                                          // Increment column index in preparation for next scan cycle
        if (columnIndex >= columnCount)                                                         // If column index limit has also been reached (the whole deck should have been scanned at this point)
        {
            columnIndex = 0;                                                                    // Wrap around to 0
            if (velocityDetectionEnabled == 1) { deckScanToggle = !deckScanToggle; }            // Toggle over to scanning of the tact switch deck if velocity detection is enabled
        }
        if (deckScanToggle == LOW) currentColumn = keyswitchColumnPins[columnIndex];            // Depending on active deck, hold the currently selected keyswitch column pin in a variable...
        else if (deckScanToggle == HIGH) currentColumn = tactSwitchColumnPins[columnIndex];     // or hold the currently selected tact switch column pin in a variable.
        digitalWrite(currentColumn, LOW);                                                       // Set the pin state to LOW turning it into a temporary ground.
        delayMicroseconds(1);                                                                   // Give the pin a bit more time to change states (ghost readings on the first row otherwise)
    }

    // Keyswitch deck
    if (deckScanToggle == LOW)
    {
        byte buttonNumber = columnIndex + (rowIndex * columnCount);                             // Assign this location in the matrix a unique number for array storage later.
        currentRow = rowPins[rowIndex];                                                         // Hold the currently selected row pin in a variable.
        byte buttonState = !digitalRead(currentRow);                                            // Take reading, invert due to INPUT_PULLUP, and store the currently selected pin state.
        if (buttonState == HIGH && activeKeyswitches[buttonNumber] == LOW && (millis() - keyswitchDebounceTimer[buttonNumber]) > debounceTime)   // If the polled button is active and passes debounce...
        {
            activeKeyswitches[buttonNumber] = HIGH;                                             // store the button state...
            keyswitchDebounceTimer[buttonNumber] = millis();                                    // and save the last button change time for later debounce comparison.
            keyswitchActivationTime[buttonNumber] = micros();                                   // Record button activation time for velocity calculation
            if (velocityDetectionEnabled == 0 && buttonNumber != 0 && buttonNumber != 20 && buttonNumber != 40 && buttonNumber != 60 && buttonNumber != 80 && buttonNumber != 100)   // If velocity detection is disabled and active button is not in the control panel
            {
                activeNotes[buttonNumber] = HIGH;                                               // Record an active note immediately on keyswitch activation.
                noteVelocity[buttonNumber] = 127 - potVelocity;                                 // Applying a modifier rather than setting it directly, as this pot also serves as a modifier for the velocity sensing deck
                loopInputDetected = HIGH;                                                       // Activate the loopInputDetected variable for looper
            }

            // Control panel buttons
            if (buttonNumber == 0)   { channelUpButton = HIGH; }                                // If a control panel button was pressed, set the state to HIGH (active)
            if (buttonNumber == 20)  { channelDownButton = HIGH; }
            if (buttonNumber == 40)  { pitchUpHalfButton = HIGH; }
            if (buttonNumber == 60)  { modulationHalfButton = HIGH; }
            if (buttonNumber == 80)  { pitchDownHalfButton = HIGH; }
            if (buttonNumber == 100) { loopButton = HIGH; }

            // OLED screensaver activity monitor
            oledScreensaverTime = millis();                                                     // Reset the OLED screensaver countdown clock
            if (oledOnOff == LOW)                                                               // If the OLED screensaver is currently active (meaning display is off)...
            {
                oled.ssd1306WriteCmd(SSD1306_DISPLAYON);                                        // turn the display back on.
                oledOnOff = HIGH;                                                               // Toggle the locking variable
            }

        }
        else if (buttonState == LOW && activeKeyswitches[buttonNumber] == HIGH && (millis() - keyswitchDebounceTimer[buttonNumber]) > debounceTime)  // If the polled button is inactive and passes debounce...
        {
            activeKeyswitches[buttonNumber] = LOW;                                              // store the button state...
            keyswitchDebounceTimer[buttonNumber] = millis();                                    // and save the last button press time for later debounce comparison.
            activeNotes[buttonNumber] = LOW;                                                    // Record an inactive note

            // Control panel buttons
            if (buttonNumber == 0)   { channelUpButton = LOW; }
            if (buttonNumber == 20)  { channelDownButton = LOW; }
            if (buttonNumber == 40)  { pitchUpHalfButton = LOW; }
            if (buttonNumber == 60)  { modulationHalfButton = LOW;}
            if (buttonNumber == 80)  { pitchDownHalfButton = LOW; }
            if (buttonNumber == 100) { loopButton = LOW; }
        }
    }

    // Tact Switch Deck
    else
    {
        byte buttonNumber = columnIndex + (rowIndex * columnCount);                             // Assign this location in the matrix a unique number for array storage later.

        if (activeKeyswitches[buttonNumber] == HIGH)                                            // Only scan the bottom tact switch if the associated upper keyswitch is active to increase effeciency
        {

// When running hardwareTest() comment out this block so that the tact-switch values always read true
// /*
            // If the top switch has been active longer than 30000µs, give up on velocity detection and send the lowest desired velocity
            if (activeTactSwitches[buttonNumber] == LOW && micros() - keyswitchActivationTime[buttonNumber] > 30000
                && buttonNumber != 0 && buttonNumber != 20 && buttonNumber != 40 && buttonNumber != 60 && buttonNumber != 80 && buttonNumber != 100)    // Omit the control panel keys due to their dual position functionaltiy
            {
                activeTactSwitches[buttonNumber] = HIGH;                                        // Override and set tact switch high
                tactSwitchDebounceTimer[buttonNumber] = millis();                               // Update debounce timer
                keyswitchActivationTime[buttonNumber] = 0;                                      // Clear velocity calculation timer
                activeNotes[buttonNumber] = HIGH;                                               // Record an active note
                noteVelocity[buttonNumber] = constrain(76 - (potVelocity / 2), 0, 127);                            // Record a "Soft" velocity level
                loopInputDetected = HIGH;                                                       // Activate the loopInputDetected variable for looper
            }
// */

            currentRow = rowPins[rowIndex];                                                     // Hold the currently selected row pin in a variable.
            byte buttonState = !digitalRead(currentRow);                                        // Take reading, invert due to INPUT_PULLUP, and store the currently selected pin state.

            if (buttonState == HIGH && activeTactSwitches[buttonNumber] == LOW && (millis() - tactSwitchDebounceTimer[buttonNumber]) > debounceTime)    // If the polled button is active and passes debounce...
            {
                activeTactSwitches[buttonNumber] = HIGH;                                        // store the button state...
                tactSwitchDebounceTimer[buttonNumber] = millis();                               // and save the last button change time for later debounce comparison.
                tactSwitchActivationTime[buttonNumber] = micros();                              // Record button activation time for velocity calculation
                if (buttonNumber != 0 && buttonNumber != 20 && buttonNumber != 40 && buttonNumber != 60 && buttonNumber != 80 && buttonNumber != 100)
                {
                    activeNotes[buttonNumber] = HIGH;
                    loopInputDetected = HIGH;

                    // Calculate velocity
                    unsigned long buttonTimeDifference = (tactSwitchActivationTime[buttonNumber] - keyswitchActivationTime[buttonNumber]);
                    if      (buttonTimeDifference <=  1800) { buttonTimeDifference =  1800; }
                    else if (buttonTimeDifference >= 30000) { buttonTimeDifference = 30000; }

                    // Print values to serial monitor for calibration
                    Serial.println(buttonTimeDifference);

                    // Defining a crude velocity curve
                    // "Slam" velocity
                    if      (buttonTimeDifference >= 1800 && buttonTimeDifference <=  5800) { noteVelocity[buttonNumber] = map(buttonTimeDifference, 1800, 5800, 127, 111) - (potVelocity / 2); }
                    // "Hard" velocity
                    else if (buttonTimeDifference >= 5801 && buttonTimeDifference <=  26000) { noteVelocity[buttonNumber] = map(buttonTimeDifference, 5801, 26000, 110, 94) - (potVelocity / 2); }
                    // "Normal" velocity
                    else if (buttonTimeDifference >= 26001 && buttonTimeDifference <= 50000) { noteVelocity[buttonNumber] = map(buttonTimeDifference, 26001, 50000, 93, 77) - constrain((potVelocity / 2), 0, 127); }
                    // "Soft" velocity (above) is anything longer than our threshold
                }

                if      (buttonNumber == 40)  { pitchUpFullButton = HIGH; }
                else if (buttonNumber == 60)  { modulationFullButton = HIGH; }
                else if (buttonNumber == 80)  { pitchDownFullButton = HIGH; }

            }

            else if (buttonState == LOW && activeTactSwitches[buttonNumber] == HIGH && (millis() - tactSwitchDebounceTimer[buttonNumber]) > debounceTime)
            {
                if      (buttonNumber == 40)  { pitchUpFullButton = LOW; activeTactSwitches[buttonNumber] = LOW;}
                else if (buttonNumber == 60)  { modulationFullButton = LOW; activeTactSwitches[buttonNumber] = LOW;}
                else if (buttonNumber == 80)  { pitchDownFullButton = LOW; activeTactSwitches[buttonNumber] = LOW;}
                tactSwitchDebounceTimer[buttonNumber] = millis();
            }

        }

        else if (activeKeyswitches[buttonNumber] == LOW)                                        // Only unlatch the bottom button after the top has been released, as the full cycle is required for velocity calculation
        {
            activeTactSwitches[buttonNumber] = LOW;
        }
    }
    rowIndex = rowIndex + 1;                                                                    // Increment row index in preparation for next program loop
}


void footPedal()
{
    if (rateLimiterToggle == HIGH) { footPedalButton = digitalRead(footPedalPin); }
    if (footPedalButton == HIGH && previousFootPedalButton == LOW)
    {
        previousFootPedalButton = footPedalButton;
        if      (pedalFunction == SUSTAIN)      { controlChange(channelMap[channelIndex], 64, 127); }
        else if (pedalFunction == LOOPER)       { loopButton = HIGH; }
        else if (pedalFunction == MODULATION)   { controlChange(channelMap[channelIndex], 1, 127); }
    }
    else if (footPedalButton == LOW && previousFootPedalButton == HIGH)
    {
        previousFootPedalButton = footPedalButton;
        if      (pedalFunction == SUSTAIN)      { controlChange(channelMap[channelIndex], 64, 0); }
        else if (pedalFunction == LOOPER)       { loopButton = LOW; }
        else if (pedalFunction == MODULATION)   { controlChange(channelMap[channelIndex], 1, 0); }
    }
}


void analogPots()
{
    if (rateLimiterToggle == HIGH)
    {
      topPotValue = map(analogRead(topPotPin), 0, 1023, 0, 127);
      bottomPotValue = map(analogRead(bottomPotPin), 0, 1023, 0, 127);

    }
    if (topPotValue != previousTopPotValue)                                                 // No deadzone on this, as tiny random changes are inconsequential, maybe even desireable?
    {
      potVelocity = map(topPotValue, 0, 127, 127, 0);
      previousTopPotValue = topPotValue;

    }

    if (bottomPotValue > (previousBottomPotValue + analogDeadzone) || bottomPotValue < (previousBottomPotValue - analogDeadzone))
    {
      controlChange(channelMap[channelIndex], 7, bottomPotValue);
      previousBottomPotValue = bottomPotValue;
    }
}


void channelSelect()
{
    // Channel Up
    if (channelUpButton == HIGH && previousChannelUpButton == LOW && ((channelIndex <= 15 && (modeSelection != LAYER && modeSelection != SPLIT)) || (channelIndex <= 13 && (modeSelection == LAYER || modeSelection == SPLIT))) )   // Confine paired indexes to 0-13 (14 is odd one out, 15 is percussion)
    {
        previousChannelUpButton = HIGH;
        for (int i = 0; i < elementCount; i++)                                              // For all buttons in the deck
        {
            if (i != 0 && i != 20 && i != 40 && i != 60 && i != 80 && i != 100)             // that aren't part of the control panel
            {
                activeNotes[i] = LOW;                                                       // Pop active notes to prevent note hangs when changing channels
            }
        }
        playNotes();                                                                        // Send note data to MIDI bus

        pitchBendChange(channelMap[channelIndex], referencePitch & 0x7F, referencePitch >> 7);
        // controlChange(channelMap[channelIndex], 1, 0);
        // controlChange(channelMap[channelIndex], 64, 0);
        if (modeSelection == LAYER || modeSelection == SPLIT)
        {
            if      (channelIndex % 2 == 0)
            {
                pitchBendChange(channelMap[channelIndex + 1], referencePitch & 0x7F, referencePitch >> 7);
                controlChange(channelMap[channelIndex + 1], 1, 0);
                controlChange(channelMap[channelIndex + 1], 64, 0);
            }
            else if (channelIndex % 2 == 1)
            {
                pitchBendChange(channelMap[channelIndex - 1], referencePitch & 0x7F, referencePitch >> 7);
                controlChange(channelMap[channelIndex - 1], 1, 0);
                controlChange(channelMap[channelIndex - 1], 64, 0);
            }
        }

        if (loopRecordingEnabled == HIGH && loopPlaybackEnabled == LOW)                     // Channel up/down also activates the looper button on channel change to allow for quick overdubbing
        {
            loopRecordingToggle = HIGH;
            looper();
        }

        channelIndex = channelIndex + 1;                                                    // Increment channel index
        if      (channelIndex == 16 && (modeSelection != LAYER && modeSelection != SPLIT)) channelIndex = 0;            // If index limit is reached, roll over back to 0
        else if (channelIndex == 14 && (modeSelection == LAYER || modeSelection == SPLIT)) channelIndex = 0;

        if (deckType == 1 && channelIndex == 15)                                            // If percussion channel is selected, disable velocity detection (the added latency on light hits makes playing frustrating)
        {
            velocityDetectionEnabled = 0;
            deckScanToggle = LOW;                                                           // Force deckScanToggle LOW in case we were in a scan when this button was hit
        }
        else if (deckType == 1 && channelIndex != 15)                                       // If leaving percussion channel, turn velocity detection back on
        {
            velocityDetectionEnabled = 1;
        }
        oledUpdate();                                                                       // Update the OLED with the new data
    }

    // Channel Down
    else if (channelDownButton == HIGH && previousChannelDownButton == LOW && channelIndex >= 0)
    {
        previousChannelDownButton = HIGH;
        for (int i = 0; i < elementCount; i++)
        {
            if (i != 0 && i != 20 && i != 40 && i != 60 && i != 80 && i != 100)
            {
                activeNotes[i] = LOW;
            }
        }
        playNotes();

        pitchBendChange(channelMap[channelIndex], referencePitch & 0x7F, referencePitch >> 7);
        // controlChange(channelMap[channelIndex], 1, 0);
        // controlChange(channelMap[channelIndex], 64, 0);
        if (modeSelection == LAYER || modeSelection == SPLIT)
        {
            if      (channelIndex % 2 == 0)
            {
                pitchBendChange(channelMap[channelIndex + 1], referencePitch & 0x7F, referencePitch >> 7);
                controlChange(channelMap[channelIndex + 1], 1, 0);
                controlChange(channelMap[channelIndex + 1], 64, 0);
            }
            else if (channelIndex % 2 == 1)
            {
                pitchBendChange(channelMap[channelIndex - 1], referencePitch & 0x7F, referencePitch >> 7);
                controlChange(channelMap[channelIndex - 1], 1, 0);
                controlChange(channelMap[channelIndex - 1], 64, 0);
            }
        }

        if (loopRecordingEnabled == HIGH && loopPlaybackEnabled == LOW)
        {
            loopRecordingToggle = HIGH;
            looper();
        }

        channelIndex = channelIndex - 1;                                                    // Decrement channel index
        if      (channelIndex == -1 && (modeSelection != LAYER && modeSelection != SPLIT)) channelIndex = 15;
        else if (channelIndex == -1 && (modeSelection == LAYER || modeSelection == SPLIT)) channelIndex = 13;

        if (deckType == 1 && channelIndex == 15)
        {
            velocityDetectionEnabled = 0;
            deckScanToggle = LOW;
        }
        else if (deckType == 1 && channelIndex != 15)
        {
            velocityDetectionEnabled = 1;
        }
        oledUpdate();

    }

    // Channel Reset
    else if (channelResetToggle == HIGH)
    {
        channelResetToggle = LOW;
        for (int i = 0; i < elementCount; i++)
        {
            if (i != 0 && i != 20 && i != 40 && i != 60 && i != 80 && i != 100)
            {
                activeNotes[i] = LOW;
            }
        }
        playNotes();

        pitchBendChange(channelMap[channelIndex], referencePitch & 0x7F, referencePitch >> 7);
        // controlChange(channelMap[channelIndex], 1, 0);
        // controlChange(channelMap[channelIndex], 64, 0);
        if (modeSelection == LAYER || modeSelection == SPLIT)
        {
            if      (channelIndex % 2 == 0)
            {
                pitchBendChange(channelMap[channelIndex + 1], referencePitch & 0x7F, referencePitch >> 7);
                controlChange(channelMap[channelIndex + 1], 1, 0);
                controlChange(channelMap[channelIndex + 1], 64, 0);
            }
            else if (channelIndex % 2 == 1)
            {
                pitchBendChange(channelMap[channelIndex - 1], referencePitch & 0x7F, referencePitch >> 7);
                controlChange(channelMap[channelIndex - 1], 1, 0);
                controlChange(channelMap[channelIndex - 1], 64, 0);
            }
        }

        channelIndex = 0;                                                                   // Reset back to default channel 0

        if (deckType == 1 && channelIndex == 15)
        {
            velocityDetectionEnabled = 0;
            deckScanToggle = LOW;
        }
        else if (deckType == 1 && channelIndex != 15)
        {
            velocityDetectionEnabled = 1;
        }
        oledUpdate();
    }
}


void rotaryEncoder()
{
    // Read rotary encoder pin states
    encoder.tick();

    if (rateLimiterToggle == HIGH)
    {
        rotaryEncoderButton = !digitalRead(rotaryEncoderButtonPin);
        newPos = encoder.getPosition();
    }

    // Rotary Encoder Button
    if (rotaryEncoderButton == HIGH && previousRotaryEncoderButton == LOW && (millis() - rotaryEncoderButtonDebounceTimer) > rotaryButtonDebounceTime)  // If rotary encoder button is active and passes debounce
    {
        previousRotaryEncoderButton = HIGH;
        rotaryEncoderButtonDebounceTimer = millis();
        velocityDisableClock = millis() + 1000;                                             // If the button is not released before 1000ms, velocity detection will be toggled
        if (oledMenuToggle == LOW)
        {
            oledMenuToggle = HIGH;                                                          // Toggle to "value" column selected
        }
        else
        {
            oledMenuToggle = LOW;                                                           // Toggle to "option" column selected
        }
        oledUpdate();
    }

    if (velocityDisableClock < millis())
    {
        velocityDisableClock = 4294967295;                                                  // Set velocityDisableClock back to the max value for an unsigned long (49.7 days of milliseconds from power on)
        if (velocityDetectionEnabled == 0)
        {
            velocityDetectionEnabled = 1;
            // Velocity Detection Enabled Message
            oled.clear();
            oled.setCursor(0,2);
            oled.print("");
            oled.setCursor(0,3);
            oled.print("                   ");
            oled.setCursor(0,4);
            oled.print("Velocity  Detection");
            oled.setCursor(0,5);
            oled.print("      ENABLED      ");
            oled.setCursor(0,6);
            oled.print("                   ");
            oled.setCursor(0,7);
            oled.print("");
            // Flash amber top rows
            for (int i = 0; i < 4; i++)                                                         // Blink amber top rows
            {
                oled.setCursor(0,0);
                oled.print("");
                oled.setCursor(0,1);
                oled.print("");
                delay(200);
                oled.setCursor(0,0);
                oled.print("                     ");
                oled.setCursor(0,1);
                oled.print("                     ");
                delay(200);
            }
        }
        else if (velocityDetectionEnabled == 1)
        {
            velocityDetectionEnabled = 0;
            deckScanToggle = LOW;                                                           // Force deckScanToggle LOW in case we were in a scan when this button was hit
            // Velocity Detection Disabled Message
            oled.clear();
            oled.setCursor(0,2);
            oled.print("");
            oled.setCursor(0,3);
            oled.print("                   ");
            oled.setCursor(0,4);
            oled.print("Velocity  Detection");
            oled.setCursor(0,5);
            oled.print("     DISABLED      ");
            oled.setCursor(0,6);
            oled.print("                   ");
            oled.setCursor(0,7);
            oled.print("");
            // Flash amber top rows
            for (int i = 0; i < 4; i++)                                                         // Blink amber top rows
            {
                oled.setCursor(0,0);
                oled.print("");
                oled.setCursor(0,1);
                oled.print("");
                delay(200);
                oled.setCursor(0,0);
                oled.print("                     ");
                oled.setCursor(0,1);
                oled.print("                     ");
                delay(200);
            }
        }
        oledInit = HIGH;                                                            // Reinitialize the OLED display
        oledUpdate();
    }


    // Rotary Encoder Knob
    if (oledMenuToggle == LOW)  // If "option" column selected
    {
        // Clockwise
        if (rotaryEncoderPosition < newPos)
        {
            rotaryEncoderPosition = newPos;
            oledOption = oledOption + 1;
            if (oledOption == 6)                                                            // If oledOption becomes greater than 5, roll over to 2
            {
                oledOption = 2;
            }
            oledUpdate();
        }
        // Anti-Clockwise
        else if (rotaryEncoderPosition > newPos)
        {
            rotaryEncoderPosition = newPos;
            oledOption = oledOption - 1;
            if (oledOption == 1)                                                            // If oledOption becomes less than 2, roll over to 5
            {
                oledOption = 5;
            }
            oledUpdate();
        }
    }
    else    // If "value" column selected
    {
        // Clockwise
        if (rotaryEncoderPosition < newPos)
        {
            rotaryEncoderPosition = newPos;
            // Program selection
            if (oledOption == 2)
            {
                if (channelIndex != 15 && modeSelection != SYNTH && instrumentIndex[channelMap[channelIndex]] < sizeof(programMap) - 1)         // If drum mode is not enabled
                {
                    instrumentIndex[channelMap[channelIndex]] = instrumentIndex[channelMap[channelIndex]] + 1;
                    controlChange(channelMap[channelIndex], 0, bankMap[instrumentIndex[channelMap[channelIndex]]]);
                    programChange(channelMap[channelIndex], programMap[instrumentIndex[channelMap[channelIndex]]]);
                }
                else if (channelIndex != 15 && modeSelection != SYNTH && instrumentIndex[channelMap[channelIndex]] == sizeof(programMap) - 1)   // If drum mode is not enabled
                {
                    instrumentIndex[channelMap[channelIndex]] = 0;
                    controlChange(channelMap[channelIndex], 0, bankMap[instrumentIndex[channelMap[channelIndex]]]);
                    programChange(channelMap[channelIndex], programMap[instrumentIndex[channelMap[channelIndex]]]);
                }
                else if (channelIndex == 15 && modeSelection != SYNTH && instrumentIndex[channelMap[channelIndex]] < sizeof(drumKitMap) - 1)    // If drum mode is enabled
                {
                    instrumentIndex[channelMap[channelIndex]] = instrumentIndex[channelMap[channelIndex]] + 1;
                    programChange(channelMap[channelIndex], drumKitMap[instrumentIndex[channelMap[channelIndex]]]);
                }
                else if (modeSelection == SYNTH && synthInstrumentIndex[channelMap[channelIndex]] < 11)                                         // If synth mode is enabled
                {
                    synthInstrumentIndex[channelMap[channelIndex]] = synthInstrumentIndex[channelMap[channelIndex]] + 1;
                    // Change waveforms
                    for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
                    {
                        if (channelIndex != 15)                                         // If not drums channel index
                        {
                            waveformCh[channelMap[channelIndex]][myWaveform].begin(waveformAmplitude, 440, synthProgramMap[synthInstrumentIndex[channelMap[channelIndex]]]);
                            envelopeCh[channelMap[channelIndex]][myWaveform].attack(10.5);
                            envelopeCh[channelMap[channelIndex]][myWaveform].hold(100);
                            envelopeCh[channelMap[channelIndex]][myWaveform].decay(300);
                            envelopeCh[channelMap[channelIndex]][myWaveform].sustain(synthEnvSustainMap[synthInstrumentIndex[channelMap[channelIndex]]]);
                            envelopeCh[channelMap[channelIndex]][myWaveform].release(300);
                        }
                        else
                        {
                            waveformCh[channelMap[channelIndex]][myWaveform].begin(waveformAmplitude, 440, synthDrumKitMap[synthInstrumentIndex[channelMap[channelIndex]]]);
                            envelopeCh[channelMap[channelIndex]][myWaveform].attack(10.5);
                            envelopeCh[channelMap[channelIndex]][myWaveform].hold(2.5);
                            envelopeCh[channelMap[channelIndex]][myWaveform].decay(35);
                            envelopeCh[channelMap[channelIndex]][myWaveform].sustain(synthEnvSustainMap[synthInstrumentIndex[channelMap[channelIndex]]]);
                            envelopeCh[channelMap[channelIndex]][myWaveform].release(300);
                        }
                    }
                }
            }
            // Device mode selection
            else if (oledOption == 3 && modeSelection < SYNTH)                              // Mode selection - keep in bounds of range 0 (NORMAL) to 4 (SYNTH)
            {

                if (modeSelection == AUTOSUS)                                               // If leaving AutoSus mode...
                {
                    for (int i = 0; i < 15; i++)
                    {
                        controlChange(channelMap[i], 64, 0);                                // disable sustain.
                        autoSusStatus = LOW;
                    }
                }

                channelResetToggle = HIGH;
                channelSelect();
                modeSelection = modeSelection + 1;                                          // Increment modeSelection


                if (modeSelection == SYNTH)                                                 // Synth mode
                {
                    // Init waveforms
                    for (byte myChannelIndex = 0; myChannelIndex < 16; myChannelIndex++)
                    {
                        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
                        {
                            if (myChannelIndex != 15)                                       // If not drums
                            {
                                waveformCh[channelMap[myChannelIndex]][myWaveform].begin(waveformAmplitude, 440, synthProgramMap[synthInstrumentIndex[channelMap[myChannelIndex]]]);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].attack(10.5);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].hold(100);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].decay(300);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].sustain(synthEnvSustainMap[synthInstrumentIndex[channelMap[myChannelIndex]]]);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].release(300);
                            }
                            else
                            {
                                waveformCh[channelMap[myChannelIndex]][myWaveform].begin(waveformAmplitude, 440, synthDrumKitMap[synthInstrumentIndex[channelMap[myChannelIndex]]]);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].attack(10.5);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].hold(2.5);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].decay(35);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].sustain(synthEnvSustainMap[synthInstrumentIndex[channelMap[myChannelIndex]]]);
                                envelopeCh[channelMap[myChannelIndex]][myWaveform].release(300);
                            }
                        }
                    }
                }
            }
            // Pedal mode selection
            else if (oledOption == 4 && pedalFunction < MODULATION)
            {
                if (pedalFunction == SUSTAIN)                                               // If leaving sustain mode...
                {
                    for (int i = 0; i < 15; i++)
                    {
                        controlChange(channelMap[i], 64, 0);                                // Disable sustain
                    }
                }
                pedalFunction = pedalFunction + 1;
            }
            // Transposition selection
            else if (oledOption == 5 && transposeValue[channelMap[channelIndex]] < 12)
            {
                for (int i = 0; i < elementCount; i++)
                {
                    if (i != 0 && i != 20 && i != 40 && i != 60 && i != 80 && i != 100)     // All notes not belonging to the control panel
                    {
                        activeNotes[i] = LOW;                                               // Pop any active notes to prevent hangs when changing value
                    }
                }
                playNotes();
                transposeValue[channelMap[channelIndex]] = transposeValue[channelMap[channelIndex]] + 1;
            }
            oledUpdate();
        }
        // Anti-Clockwise
        if (rotaryEncoderPosition > newPos)
        {
            rotaryEncoderPosition = newPos;
            // Program selection
            if (oledOption == 2)
            {
                if (channelIndex != 15 && modeSelection != SYNTH && instrumentIndex[channelMap[channelIndex]] > 0)
                {
                    instrumentIndex[channelMap[channelIndex]] = instrumentIndex[channelMap[channelIndex]] - 1;
                    controlChange(channelMap[channelIndex], 0, bankMap[instrumentIndex[channelMap[channelIndex]]]);
                    programChange(channelMap[channelIndex], programMap[instrumentIndex[channelMap[channelIndex]]]);
                }
                else if (channelIndex != 15 && modeSelection != SYNTH && instrumentIndex[channelMap[channelIndex]] == 0)
                {
                    instrumentIndex[channelMap[channelIndex]] = 127;
                    controlChange(channelMap[channelIndex], 0, bankMap[instrumentIndex[channelMap[channelIndex]]]);
                    programChange(channelMap[channelIndex], programMap[instrumentIndex[channelMap[channelIndex]]]);
                }
                else if (channelIndex == 15 && modeSelection != SYNTH && instrumentIndex[channelMap[channelIndex]] > 0)
                {
                    instrumentIndex[channelMap[channelIndex]] = instrumentIndex[channelMap[channelIndex]] - 1;
                    programChange(channelMap[channelIndex], drumKitMap[instrumentIndex[channelMap[channelIndex]]]);
                }
                else if (modeSelection == SYNTH && synthInstrumentIndex[channelMap[channelIndex]] > 0)
                {
                    synthInstrumentIndex[channelMap[channelIndex]] = synthInstrumentIndex[channelMap[channelIndex]] - 1;
                    // Change waveforms
                    for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
                    {
                        if (channelIndex != 15)                                         // If not drums
                        {
                            waveformCh[channelMap[channelIndex]][myWaveform].begin(waveformAmplitude, 440, synthProgramMap[synthInstrumentIndex[channelMap[channelIndex]]]);
                            envelopeCh[channelMap[channelIndex]][myWaveform].attack(10.5);
                            envelopeCh[channelMap[channelIndex]][myWaveform].hold(100);
                            envelopeCh[channelMap[channelIndex]][myWaveform].decay(300);
                            envelopeCh[channelMap[channelIndex]][myWaveform].sustain(synthEnvSustainMap[synthInstrumentIndex[channelMap[channelIndex]]]);
                            envelopeCh[channelMap[channelIndex]][myWaveform].release(300);
                        }
                        else
                        {
                            waveformCh[channelMap[channelIndex]][myWaveform].begin(waveformAmplitude, 440, synthDrumKitMap[synthInstrumentIndex[channelMap[channelIndex]]]);
                            envelopeCh[channelMap[channelIndex]][myWaveform].attack(10.5);
                            envelopeCh[channelMap[channelIndex]][myWaveform].hold(2.5);
                            envelopeCh[channelMap[channelIndex]][myWaveform].decay(35);
                            envelopeCh[channelMap[channelIndex]][myWaveform].sustain(synthEnvSustainMap[synthInstrumentIndex[channelMap[channelIndex]]]);
                            envelopeCh[channelMap[channelIndex]][myWaveform].release(300);
                        }
                    }
                }
            }
            // Device mode selection
            else if (oledOption == 3 && modeSelection > NORMAL)
            {

                if (modeSelection == SYNTH)
                {

                    // Disable waveform generation
                    for (byte myChannel = 0; myChannel < 16; myChannel++)
                    {
                        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
                        {
                            waveformCh[myChannel][myWaveform].amplitude(0);
                        }
                    }

                }
                else if (modeSelection == AUTOSUS)
                {
                    for (int i = 0; i < 15; i++)
                    {
                        controlChange(channelMap[i], 64, 0);
                        autoSusStatus = LOW;
                    }
                }

                channelResetToggle = HIGH;
                channelSelect();
                modeSelection = modeSelection - 1;                                          // Decrement modeSelection

            }
            // Pedal mode selection
            else if (oledOption == 4 && pedalFunction > SUSTAIN)
            {
                if (pedalFunction == MODULATION)
                {
                    for (int i = 0; i < 15; i++)
                    {
                        controlChange(channelMap[i], 1, 0);
                    }
                }
                pedalFunction = pedalFunction - 1;
            }
            // Transposition selection
            else if (oledOption == 5 && transposeValue[channelMap[channelIndex]] > -12)
            {
                for (int i = 0; i < elementCount; i++)
                {
                    if (i != 0 && i != 20 && i != 40 && i != 60 && i != 80 && i != 100)
                    {
                        activeNotes[i] = LOW;
                    }
                }
                playNotes();
                transposeValue[channelMap[channelIndex]] = transposeValue[channelMap[channelIndex]] - 1;
            }
            oledUpdate();
        }
    }
}


void digitalMidiCC()
{
    // Pitch Bend - 14 bit value range 0-16383 with 8192 as the "mid-point", however there is no true middle value as it's evenly divisible.  This is accounted for below.
    // pitchOffset values:  -8192 (-whole step) -4096 (-half step) or 4096(+half step) 8192 (+whole step)
    // Absolute values   -whole step: 0     -half step: 4096    neutral: 8192   +half step: 12288   +whole step: 16363 (-1)
    // Pitch bend value is broken into it's component least significant (LSB) and most significant bytes (MSB) for saving to the looper's packet recorder by doing
    // a bitwise AND with 0x7F (127 dec or 01111111) to clear high byte for LSB and bitshift right 7 positions to derive MSB.
    // Realistically we only need MSB as 128 values is still overkill for a one second bend, but slower bends (or the addition of global pitch adjustment) would benefit from full range.

    // Return to neutral
    if (pitchUpHalfButton == LOW && pitchUpFullButton == LOW && pitchDownHalfButton == LOW && pitchDownFullButton == LOW)
    {
        // From above
        if (pitchOffset > 0)
        {
            pitchOffset = pitchOffset - pitchSpeed;   // Decrement pitchOffset by the amount of pitchSpeed for this program loop
            pitchBendChange(channelMap[channelIndex], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7);
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
            }
        }
        // From below
        else if (pitchOffset < 0)
        {
            pitchOffset = pitchOffset + pitchSpeed;   // Increment pitchOffset by the amount of pitchSpeed for this program loop
            pitchBendChange(channelMap[channelIndex], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7);
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
            }
        }
    }

    // Velocity on
    if (velocityDetectionEnabled == 1)
    {
        // Bend up from neutral to +half-step or down from +full-step to +half-step
        if (pitchUpHalfButton == HIGH && pitchUpFullButton == LOW && pitchDownHalfButton == LOW)
        {
            if (pitchOffset > 4096)
            {
                pitchOffset = pitchOffset - pitchSpeed;
                pitchBendChange(channelMap[channelIndex], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                    else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                }
            }
            else if (pitchOffset < 4096)
            {
                pitchOffset = pitchOffset + pitchSpeed;
                pitchBendChange(channelMap[channelIndex], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                    else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                }
            }
        }
        // Bend up to +full-step
        else if (pitchUpFullButton == HIGH && pitchDownHalfButton == LOW)
        {
            if (pitchOffset < 8192)
            {
                pitchOffset = pitchOffset + pitchSpeed;
                pitchBendChange(channelMap[channelIndex], ((referencePitch - 1) + pitchOffset) & 0x7F, ((referencePitch - 1) + pitchOffset) >> 7);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], ((referencePitch - 1) + pitchOffset) & 0x7F, ((referencePitch - 1) + pitchOffset) >> 7); }
                    else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], ((referencePitch - 1) + pitchOffset) & 0x7F, ((referencePitch - 1) + pitchOffset) >> 7); }
                }
            }
        }
        // Bend down from neutral to -half-step or up from -full-step to -half-step
        else if (pitchUpHalfButton == LOW && pitchDownHalfButton == HIGH && pitchDownFullButton == LOW)
        {
            if (pitchOffset > -4096)
            {
                pitchOffset = pitchOffset - pitchSpeed;
                pitchBendChange(channelMap[channelIndex], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                    else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                }
            }
            else if (pitchOffset < -4096)
            {
                pitchOffset = pitchOffset + pitchSpeed;
                pitchBendChange(channelMap[channelIndex], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                    else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                }
            }
        }
        // Bend down to -full-step
        else if (pitchUpHalfButton == LOW && pitchDownFullButton == HIGH)
        {
            if (pitchOffset > -8192)
            {
                pitchOffset = pitchOffset - pitchSpeed;
                pitchBendChange(channelMap[channelIndex], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                    else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                }
            }
        }
    }

    // Velocity off
    else
    {
        // Bend up to half-step
        if (pitchUpHalfButton == HIGH && pitchDownHalfButton == LOW)
        {
            if (pitchOffset < 4096)
            {
                pitchOffset = pitchOffset + pitchSpeed;
                pitchBendChange(channelMap[channelIndex], ((referencePitch - 1) + pitchOffset) & 0x7F, ((referencePitch - 1) + pitchOffset) >> 7);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], ((referencePitch - 1) + pitchOffset) & 0x7F, ((referencePitch - 1) + pitchOffset) >> 7); }
                    else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], ((referencePitch - 1) + pitchOffset) & 0x7F, ((referencePitch - 1) + pitchOffset) >> 7); }
                }
            }
        }
        // Bend down to half-step
        else if (pitchUpHalfButton == LOW && pitchDownHalfButton == HIGH)
        {
            if (pitchOffset > -4096)
            {
                pitchOffset = pitchOffset - pitchSpeed;
                pitchBendChange(channelMap[channelIndex], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { pitchBendChange(channelMap[channelIndex + 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                    else if (channelIndex % 2 == 1) { pitchBendChange(channelMap[channelIndex - 1], (referencePitch + pitchOffset) & 0x7F, (referencePitch + pitchOffset) >> 7); }
                }
            }
        }
    }

    // Modulation
    // Return to neutral
    if (modulationHalfButton == LOW)
    {
        if (modOffset > 0)
        {
            modOffset = modOffset - modSpeed;
            controlChange(channelMap[channelIndex], 1, (0 + modOffset));
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if      (channelIndex % 2 == 0) { controlChange(channelMap[channelIndex + 1], 1, (0 + modOffset)); }
                else if (channelIndex % 2 == 1) { controlChange(channelMap[channelIndex - 1], 1, (0 + modOffset)); }
            }
        }
    }

    // Velocity on
    if (velocityDetectionEnabled == 1)
    {
        if (modulationHalfButton == HIGH && modulationFullButton == LOW)
        {
            if (modOffset > 64)
            {
                modOffset = modOffset - modSpeed;
                controlChange(channelMap[channelIndex], 1, (0 + modOffset));
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { controlChange(channelMap[channelIndex + 1], 1, (0 + modOffset)); }
                    else if (channelIndex % 2 == 1) { controlChange(channelMap[channelIndex - 1], 1, (0 + modOffset)); }
                }
            }
            else if (modOffset < 64)
            {
                modOffset = modOffset + modSpeed;
                controlChange(channelMap[channelIndex], 1, (0 + modOffset));
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { controlChange(channelMap[channelIndex + 1], 1, (0 + modOffset)); }
                    else if (channelIndex % 2 == 1) { controlChange(channelMap[channelIndex - 1], 1, (0 + modOffset)); }
                }
            }

        }
        else if (modulationFullButton == HIGH)
        {
            if (modOffset < 128)
            {
                modOffset = modOffset + modSpeed;
                controlChange(channelMap[channelIndex], 1, (0 + modOffset) - 1);    // 127 max
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { controlChange(channelMap[channelIndex + 1], 1, (0 + modOffset) - 1); }
                    else if (channelIndex % 2 == 1) { controlChange(channelMap[channelIndex - 1], 1, (0 + modOffset) - 1); }
                }
            }
        }
    }
    // Velocity off
    else
    {
        if (modulationHalfButton == HIGH)
        {
            if (modOffset < 128)
            {
                modOffset = modOffset + modSpeed;
                controlChange(channelMap[channelIndex], 1, (0 + modOffset) - 1);
                if (modeSelection == LAYER || modeSelection == SPLIT)
                {
                    if      (channelIndex % 2 == 0) { controlChange(channelMap[channelIndex + 1], 1, (0 + modOffset) - 1); }
                    else if (channelIndex % 2 == 1) { controlChange(channelMap[channelIndex - 1], 1, (0 + modOffset) - 1); }
                }
            }
        }
    }

    if (modeSelection == SYNTH && modOffset != 0)
    {
        if (synthModToggle == LOW)
        {
            for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
            {
                waveformCh[channelMap[channelIndex]][myWaveform].frequency(midiPitchFrequencyMap[waveformPitch[channelMap[channelIndex]][myWaveform]] * 0.5);
            }
            synthModToggle = HIGH;
        }
        else if (synthModToggle == HIGH)
        {
            for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
            {
                waveformCh[channelMap[channelIndex]][myWaveform].frequency(midiPitchFrequencyMap[waveformPitch[channelMap[channelIndex]][myWaveform]]);
            }
            synthModToggle = LOW;
        }
    }
    else if (modeSelection == SYNTH && synthModToggle == HIGH)
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
            waveformCh[channelMap[channelIndex]][myWaveform].frequency(midiPitchFrequencyMap[waveformPitch[channelMap[channelIndex]][myWaveform]]);
        }
        synthModToggle = LOW;
    }


    if (modeSelection == AUTOSUS)
    {
        for (int i = 0; i < elementCount; i++)
        {
            if (activeNotes[i] == HIGH)
            {
                autoSusTimer = millis();
                if (autoSusStatus == LOW)
                {
                    autoSusStatus = HIGH;
                    controlChange(channelMap[channelIndex], 64, 127);
                }
            }
        }

        if ((millis() - autoSusTimer) > 2000 && autoSusStatus == HIGH)
        {
            autoSusStatus = LOW;
            if (loopTrackEnabled[channelMap[channelIndex]] == 0) { controlChange(channelMap[channelIndex], 64, 0); }
        }
    }

}


void looper()
{
    // Looper button/pedal is held down to force a reset
    if (loopButton == HIGH && previousLoopButton == LOW)
    {
        previousLoopButton = HIGH;                                                          // Lock input
        loopDisableClock = millis() + 750;                                                  // Set reset time.  If the button is not released before then, we will reset.
        loopRecordingToggle = HIGH;
    }
    else if (loopButton == LOW)                                                             // If the button is released
    {
        loopDisableClock = 4294967295;                                                      // Set loopDisableClock back to the max value for an unsigned long (49.7 days of milliseconds from power on)
    }

    if (loopDisableClock < millis())
    {
        loopDisableClock = 4294967295;                                                      // Set loopDisableClock back to the max value for an unsigned long (49.7 days of milliseconds from power on)
        for (int myChannel = 0; myChannel < 16; myChannel++)                                // For all channels
        {
            if (loopTrackEnabled[myChannel] == 1)
            {
                for (int myPitch = 0; myPitch < 127; myPitch++)                             // For all pitches per channel
                {
                    if (noteTracking[myChannel][myPitch] == 1)                              // If there is an unclosed noteOn...
                    {
                        noteOff(myChannel, myPitch, 0);                                     // Sending a closing noteOff.
                        noteTracking[myChannel][myPitch] = 0;
                    }
                }
                pitchBendChange(myChannel, referencePitch & 0x7F, referencePitch >> 7);     // Reset pitchBend
                controlChange(myChannel, 1, 0);                                             // Disable modulation
                controlChange(myChannel, 64, 0);                                            // Disable sustain
            }
        }
        loopWaitingForInput = LOW;
        loopRecordingEnabled = LOW;                                                         // Disable loop packet recording
        loopPlaybackEnabled = LOW;                                                          // Disable loop playback if active
        loopInMemory = LOW;                                                                 // Mark loop free for writing
        loopDuration = 0;                                                                   // Clear loop duration
        memset(loopRecordingIndex, 0, sizeof(loopRecordingIndex));                          // Reset recording index
        memset(loopPlaybackIndex, 0, sizeof(loopPlaybackIndex));                            // Reset playback index
        memset(loopPacketTime, 0, sizeof(loopPacketTime));                                  // Flatten recorded packet arrays
        memset(loopPacketType, 0, sizeof(loopPacketType));
        memset(loopPacketByte0, 0, sizeof(loopPacketByte0));
        memset(loopPacketByte1, 0, sizeof(loopPacketByte1));
        memset(loopTrackEnabled, 0, sizeof(loopTrackEnabled));
        for (int i = 0; i < 26; i++)                                                        // Restore line 6 to the default horizontal line
        {
            oled.setCursor(i*5,6);
            oled.print("-");
        }
        oledUpdate();
    }



    // Index overflow
    if (loopRecordingEnabled == HIGH && loopInMemory == HIGH)                               // Only check this while the looper is recording active note inputs
    {
        for (byte myChannel = 0; myChannel < loopMaxChannels; myChannel++)
        {
            if (loopRecordingIndex[myChannel] >= loopMaxIndexes)                            // If an index limit is reached
            {
                loopWaitingForInput = LOW;
                loopRecordingEnabled = LOW;                                                 // Disable loop packet recording
                loopPlaybackEnabled = LOW;                                                  // Disable loop playback if active
                loopInMemory = LOW;                                                         // Mark loop free for writing
                loopDuration = 0;                                                           // Clear loop duration
                memset(loopRecordingIndex, 0, sizeof(loopRecordingIndex));                  // Reset recording index
                memset(loopPlaybackIndex, 0, sizeof(loopPlaybackIndex));                    // Reset playback index
                memset(loopPacketTime, 0, sizeof(loopPacketTime));                          // Flatten recorded packet arrays
                memset(loopPacketType, 0, sizeof(loopPacketType));
                memset(loopPacketByte0, 0, sizeof(loopPacketByte0));
                memset(loopPacketByte1, 0, sizeof(loopPacketByte1));
                memset(loopTrackEnabled, 0, sizeof(loopTrackEnabled));
                // OLED Message
                oled.clear();
                oled.setCursor(0,2);
                oled.print("");
                oled.setCursor(0,3);
                oled.print("                   ");
                oled.setCursor(0,4);
                oled.print("   LOOPER  INDEX   ");
                oled.setCursor(0,5);
                oled.print("     OVERFLOW!     ");
                oled.setCursor(0,6);
                oled.print("                   ");
                oled.setCursor(0,7);
                oled.print("");
                // Flash amber top rows
                for (int i = 0; i < 4; i++)                                                 // Blink amber top rows
                {
                    oled.setCursor(0,0);
                    oled.print("");
                    oled.setCursor(0,1);
                    oled.print("");
                    delay(200);
                    oled.setCursor(0,0);
                    oled.print("                     ");
                    oled.setCursor(0,1);
                    oled.print("                     ");
                    delay(200);
                }
                oledInit = HIGH;                                                            // Reinitialize the OLED display
                oledUpdate();
            }
        }
    }

    // Looper is activated - Initialize variables, stop playback if applicable, and wait for input
    if (loopRecordingToggle == HIGH && loopWaitingForInput == LOW && loopInMemory == LOW)
    {
        loopRecordingToggle = LOW;
        loopWaitingForInput = HIGH;
        loopRecordingEnabled = HIGH;                                                        // Enable loop packet recording in the noteOn/noteOff/pitchBendChange/controlChange functions
        loopPlaybackEnabled = LOW;                                                          // Disable loop playback if active
        loopDuration = 0;                                                                   // Clear loop duration
        memset(loopRecordingIndex, 0, sizeof(loopRecordingIndex));                          // Reset recording index
        memset(loopPlaybackIndex, 0, sizeof(loopPlaybackIndex));                            // Reset playback index
        memset(loopPacketTime, 0, sizeof(loopPacketTime));                                  // Flatten recorded packet arrays
        memset(loopPacketType, 0, sizeof(loopPacketType));
        memset(loopPacketByte0, 0, sizeof(loopPacketByte0));
        memset(loopPacketByte1, 0, sizeof(loopPacketByte1));
        memset(loopTrackEnabled, 0, sizeof(loopTrackEnabled));
        oled.setCursor(36, 7);                                                              // Indicate looper is waiting for input
        oled.print("???");
    }

    // Looper is deactivated with no notes having been pressed - Disengage looper
    if (loopRecordingToggle == HIGH && loopWaitingForInput == HIGH && loopInMemory == LOW)
    {
        loopRecordingToggle = LOW;
        loopWaitingForInput = LOW;
        loopRecordingEnabled = LOW;
        oled.setCursor(36, 7);                                                              // Indicate looper is disabled
        oled.print("---");
    }

    // Input has been detected - Enable note recording
    if (loopRecordingEnabled == HIGH && loopInMemory == LOW && loopInputDetected == HIGH)
    {
        loopStartTimestamp = (millis() - 1);                                                // Set loopStartTimestamp to 1ms before the first recorded note packet
        loopWaitingForInput = LOW;
        loopInMemory = HIGH;

        // If a MIDI CC button was being held at loop input start, record the CC as first packet
        if (modulationFullButton == HIGH)
        {
            controlChange(channelMap[channelIndex], 1, 127);
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if      (channelIndex % 2 == 0)
                {
                    controlChange(channelMap[channelIndex + 1], 1, 127);
                }
                else if (channelIndex % 2 == 1)
                {
                    controlChange(channelMap[channelIndex - 1], 1, 127);
                }
            }
        }
        else if (modulationHalfButton == HIGH && velocityDetectionEnabled == 1)
        {
            controlChange(channelMap[channelIndex], 1, 64);
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if      (channelIndex % 2 == 0)
                {
                    controlChange(channelMap[channelIndex + 1], 1, 64);
                }
                else if (channelIndex % 2 == 1)
                {
                    controlChange(channelMap[channelIndex - 1], 1, 64);
                }
            }
        }
        else if (modulationHalfButton == HIGH && velocityDetectionEnabled == 0)
        {
            controlChange(channelMap[channelIndex], 1, 127);
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if      (channelIndex % 2 == 0)
                {
                    controlChange(channelMap[channelIndex + 1], 1, 127);
                }
                else if (channelIndex % 2 == 1)
                {
                    controlChange(channelMap[channelIndex - 1], 1, 127);
                }
            }
        }
        if (footPedalButton == HIGH && pedalFunction == SUSTAIN)
        {
            controlChange(channelMap[channelIndex], 64, 127);
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if      (channelIndex % 2 == 0)
                {
                    controlChange(channelMap[channelIndex + 1], 64, 127);
                }
                else if (channelIndex % 2 == 1)
                {
                    controlChange(channelMap[channelIndex - 1], 64, 127);
                }
            }
        }
        else if (footPedalButton == HIGH && pedalFunction == MODULATION)
        {
            controlChange(channelMap[channelIndex], 1, 127);
            if (modeSelection == LAYER || modeSelection == SPLIT)
            {
                if      (channelIndex % 2 == 0)
                {
                    controlChange(channelMap[channelIndex + 1], 1, 127);
                }
                else if (channelIndex % 2 == 1)
                {
                    controlChange(channelMap[channelIndex - 1], 1, 127);
                }
            }
        }

        oled.setCursor(36, 7);                                                                  // Indicate recording is active
        oled.print("REC");
    }

    // Looper is deactivated and there is note data in memory - Finalize loop in preparation for playback.
    if (loopRecordingToggle == HIGH && loopInMemory == HIGH && loopPlaybackEnabled == LOW)
    {
        loopRecordingToggle = LOW;

        pitchBendChange(channelMap[channelIndex], referencePitch & 0x7F, referencePitch >> 7);  // Reset pitchBend
        controlChange(channelMap[channelIndex], 1, 0);                                          // Disable modulation

        loopDuration = (millis() - loopStartTimestamp);
        loopStartTimestamp = millis();
        loopPlaybackEnabled = HIGH;

        loopTrackEnabled[channelMap[channelIndex]] = 1;

        if (modeSelection == LAYER || modeSelection == SPLIT)
        {
            if      (channelIndex % 2 == 0)
            {
                loopTrackEnabled[channelMap[channelIndex + 1]] = 1;
            }
            else if (channelIndex % 2 == 1)
            {
                loopTrackEnabled[channelMap[channelIndex - 1]] = 1;
            }
        }

        oledUpdate();

        for (int i = 0; i < 26; i++)                                                            // Reset progress bar
        {
            oled.setCursor(i*5,6);
            oled.print(" ");
        }
        oled.setCursor(0, 6);
        oled.print("[----+----+----+----]");
    }


    // Play the loop repeatedly
    if (loopPlaybackEnabled == HIGH && loopInMemory == HIGH)
    {
        unsigned long playbackTime = millis();
            if ((loopStartTimestamp + loopDuration) >= playbackTime)
            {
                for (byte myChannel = 0; myChannel < loopMaxChannels; myChannel++)
                {
                    int myIndex = loopPlaybackIndex[myChannel];
                    if (loopPacketTime[myChannel][loopPlaybackIndex[myChannel]] <= (playbackTime - loopStartTimestamp) && loopTrackEnabled[myChannel] == 1 && loopPacketType[myChannel][myIndex] != 0)
                    {
                        if (loopPacketType[myChannel][myIndex] == 1)
                        {
                            loopNoteOn(myChannel, loopPacketByte0[myChannel][myIndex], loopPacketByte1[myChannel][myIndex]);
                        }
                        else if (loopPacketType[myChannel][myIndex] == 2)
                        {
                            loopNoteOff(myChannel, loopPacketByte0[myChannel][myIndex], loopPacketByte1[myChannel][myIndex]);
                        }
                        else if (loopPacketType[myChannel][myIndex] == 3)
                        {
                            loopControlChange(myChannel, loopPacketByte0[myChannel][myIndex], loopPacketByte1[myChannel][myIndex]);
                        }
                        else if (loopPacketType[myChannel][myIndex] == 4)
                        {
                            loopPitchBendChange(myChannel, loopPacketByte0[myChannel][myIndex], loopPacketByte1[myChannel][myIndex]);
                        }
                        if (loopPlaybackIndex[myChannel] < loopRecordingIndex[myChannel])
                        {
                            loopPlaybackIndex[myChannel] = loopPlaybackIndex[myChannel] + 1;
                        }
                    }
                }
                if (rateLimiterToggle == HIGH) {
                    loopPercentage = ((millis() - loopStartTimestamp) * 100) / loopDuration;
                    if (loopPercentage != previousLoopPercentage && loopPercentage != 100)
                    {
                        oled.setCursor(0, 6);
                        oled.print("[----+----+----+----]");
                        oled.setCursor(map(loopPercentage, 0, 100, 6, 120), 6);             // Print progress bar to line 6 of the OLED
                        oled.print("");
                        if (loopRecordingEnabled == LOW)                                    // Only print percentage if recording/overdubbing is inactive on channel
                        {
                            oled.setCursor(36, 7);
                            oled.printf("%02d", loopPercentage);
                            oled.print("%");
                        }
                        previousLoopPercentage = loopPercentage;
                    }
                }
            }
            else
            {
                if (loopRecordingEnabled == HIGH)
                {
                    for (int myChannel = 0; myChannel < 16; myChannel++)                                // For all channels
                    {
                        if (loopTrackEnabled[myChannel] == 1)
                        {
                            for (int myPitch = 0; myPitch < 127; myPitch++)                             // For all pitches per channel
                            {
                                if (noteTracking[myChannel][myPitch] == 1)                              // If there is an unclosed noteOn...
                                {
                                    noteOff(myChannel, myPitch, 0);                                     // Sending a closing noteOff.
                                    noteTracking[myChannel][myPitch] = 0;
                                }
                            }
                            pitchBendChange(myChannel, referencePitch & 0x7F, referencePitch >> 7);     // Reset pitchBend
                            controlChange(myChannel, 1, 0);                                             // Disable modulation
                        }
                    }
                }
                for (byte myChannel = 0; myChannel < loopMaxChannels; myChannel++)
                {
                    if (loopRecordingIndex[channelMap[channelIndex]] > 0)
                    {
                        loopTrackEnabled[channelMap[channelIndex]] = 1;
                    }
                }
                loopStartTimestamp = millis();
                memset(loopPlaybackIndex, 0, sizeof(loopPlaybackIndex));
                oledUpdate();
                for (int i = 0; i < 24; i++)                                                // Reset progress bar
                {
                    oled.setCursor(i*6,6);
                    oled.print(" ");
                }
                oled.setCursor(0, 6);
                oled.print("[----+----+----+----]");
            }
    }

    // Recording is toggled during loop playback
    if (loopRecordingToggle == HIGH && loopWaitingForInput == LOW && loopInMemory == HIGH)
    {
        loopRecordingToggle = LOW;
        if (loopRecordingEnabled == HIGH)
        {
            loopRecordingEnabled = LOW;
        }
        else if (loopRecordingEnabled == LOW)
        {
            for (int myChannel = 0; myChannel < 16; myChannel++)                                // For all channels
            {
                if (loopTrackEnabled[myChannel] == 1)
                {
                    for (int myPitch = 0; myPitch < 127; myPitch++)                             // For all pitches per channel
                    {
                        if (noteTracking[myChannel][myPitch] == 1)                              // If there is an unclosed noteOn...
                        {
                            noteOff(myChannel, myPitch, 0);                                     // Sending a closing noteOff.
                            noteTracking[myChannel][myPitch] = 0;
                        }
                    }
                    pitchBendChange(myChannel, referencePitch & 0x7F, referencePitch >> 7);     // Reset pitchBend
                    controlChange(myChannel, 1, 0);                                             // Disable modulation
                    controlChange(myChannel, 64, 0);                                            // Disable sustain
                }
            }
            loopRecordingEnabled = HIGH;
            loopTrackEnabled[channelMap[channelIndex]] = 0;
            loopRecordingIndex[channelMap[channelIndex]] = 0;
            loopPlaybackIndex[channelMap[channelIndex]] = 0;
            for (int myIndex = 0; myIndex < loopMaxIndexes; myIndex++)
            {
                loopPacketTime[channelMap[channelIndex]][myIndex] = 0;
                loopPacketType[channelMap[channelIndex]][myIndex] = 0;
                loopPacketByte0[channelMap[channelIndex]][myIndex] = 0;
                loopPacketByte1[channelMap[channelIndex]][myIndex] = 0;
            }
            oledUpdate();
            oled.setCursor(36, 7);                                                              // Indicate recording is active
            oled.print("REC");
        }
    }

}


void controllerReset()
{
    if (channelUpButton == HIGH && channelDownButton == HIGH && loopButton == HIGH)         // If the reset command is input
    {
        for (int myChannel = 0; myChannel < 16; myChannel++)                                // For MIDI channels 0 through 15
        {
            controlChange(myChannel, 120, 0);                                               // All Sound Off (MIDI CC #120) - Mute all currently sounding notes
            controlChange(myChannel, 0, 0);                                                 // Reset bank on all channels to 0
            programChange(myChannel, 0);                                                    // Reset program on all channels to 0
            controlChange(myChannel, 64, 0);                                                // Disable sustain on all channels (MIDI CC #64)
            pitchBendChange(myChannel, referencePitch & 0x7F, referencePitch >> 7);         // Reset pitchBend
            controlChange(myChannel, 1, 0);                                                 // Disable modulation

        }
        // OLED Controller Reset Message
        oled.clear();
        oled.setCursor(0,2);
        oled.print("");
        oled.setCursor(0,3);
        oled.print("                   ");
        oled.setCursor(0,4);
        oled.print("     CONTROLLER    ");
        oled.setCursor(0,5);
        oled.print("       RESET!      ");
        oled.setCursor(0,6);
        oled.print("                   ");
        oled.setCursor(0,7);
        oled.print("");
        // Flash amber top rows
        for (int i = 0; i < 4; i++)                                                         // Blink amber top rows
        {
            oled.setCursor(0,0);
            oled.print("");
            oled.setCursor(0,1);
            oled.print("");
            delay(200);
            oled.setCursor(0,0);
            oled.print("                     ");
            oled.setCursor(0,1);
            oled.print("                     ");
            delay(200);
        }
        WRITE_RESTART(0x5FA0004);                                                           // Write value to memory location to trigger a restart
    }
}


void playNotes()
{
    for (int i = 0; i < elementCount; i++)                                                  // For all note buttons
    {
        if (activeNotes[i] != previousActiveNotes[i])                                       // If a change is detected
        {
            if (channelIndex == 15 && modeSelection != SYNTH)                               // If percussion channel is active
            {
                if (activeNotes[i] == HIGH)
                {
                    noteOn(channelMap[channelIndex], drumLayout[i], noteVelocity[i]);       // Send a noteOn using the Drums layout
                    previousActiveNotes[i] = HIGH;
                }
                else
                {
                    noteOff(channelMap[channelIndex], drumLayout[i], 0);                    // Send a noteOff using the Drums layout
                    previousActiveNotes[i] = LOW;
                }
            }
            else                                                                            // If percussion channel isn't active
            {
                if (activeNotes[i] == HIGH)                                                 // If the button is active
                {
                    if (modeSelection == LAYER)                                             // If Layer mode is active
                    {
                        if (channelIndex % 2 == 0)                                          // If channel index is an even number, play this note and the channel above
                        {
                            noteOn(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], noteVelocity[i]);
                            noteOn(channelMap[channelIndex + 1], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex + 1]], noteVelocity[i]);
                            previousActiveNotes[i] = HIGH;
                        }
                        else if (channelIndex % 2 == 1)                                     // If channel index is an odd number, play this note and the channel below
                        {
                            noteOn(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], noteVelocity[i]);
                            noteOn(channelMap[channelIndex - 1], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex - 1]], noteVelocity[i]);
                            previousActiveNotes[i] = HIGH;
                        }
                    }
                    else if (modeSelection == SPLIT)
                    {
                        if (channelIndex % 2 == 0)
                        {
                            if (wickiHaydenLayout[i] <  splitKey) { noteOn(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], noteVelocity[i]); }
                            if (wickiHaydenLayout[i] >= splitKey) { noteOn(channelMap[channelIndex + 1], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex + 1]], noteVelocity[i]); }
                            previousActiveNotes[i] = HIGH;
                        }
                        else if (channelIndex % 2 == 1)
                        {
                            if (wickiHaydenLayout[i] <  splitKey) { noteOn(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], noteVelocity[i]); }
                            if (wickiHaydenLayout[i] >= splitKey) { noteOn(channelMap[channelIndex - 1], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex - 1]], noteVelocity[i]); }
                            previousActiveNotes[i] = HIGH;
                        }
                    }

                    /*
                    else if (modeSelection == AUTOSUS)                                      // If AutoSus mode is enabled, blip sustain on current channel
                    {
                        controlChange(channelMap[channelIndex], 64, 0);
                        controlChange(channelMap[channelIndex], 64, 127);
                        noteOn(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], noteVelocity[i]);
                        previousActiveNotes[i] = HIGH;
                    }
                    */

                    else
                    {
                        noteOn(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], noteVelocity[i]);
                        previousActiveNotes[i] = HIGH;
                    }
                }
                else                                                                        // If the button is inactive
                {
                    if (modeSelection == LAYER)
                    {
                        if (channelIndex % 2 == 0)
                        {
                            noteOff(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], 0);
                            noteOff(channelMap[channelIndex + 1], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex + 1]], 0);
                            previousActiveNotes[i] = LOW;
                        }
                        else if (channelIndex % 2 == 1)
                        {
                            noteOff(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], 0);
                            noteOff(channelMap[channelIndex - 1], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex - 1]], 0);
                            previousActiveNotes[i] = LOW;
                        }
                    }
                    else if (modeSelection == SPLIT)
                    {
                        if (channelIndex % 2 == 0)
                        {
                            if (wickiHaydenLayout[i] <  splitKey) { noteOff(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], 0); }
                            if (wickiHaydenLayout[i] >= splitKey) { noteOff(channelMap[channelIndex + 1], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex + 1]], 0); }
                            previousActiveNotes[i] = LOW;
                        }
                        else if (channelIndex % 2 == 1)
                        {
                            if (wickiHaydenLayout[i] <  splitKey) { noteOff(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], 0); }
                            if (wickiHaydenLayout[i] >= splitKey) { noteOff(channelMap[channelIndex - 1], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex - 1]], 0); }
                            previousActiveNotes[i] = LOW;
                        }
                    }
                    else
                    {
                        noteOff(channelMap[channelIndex], wickiHaydenLayout[i] + transposeValue[channelMap[channelIndex]], 0);
                        previousActiveNotes[i] = LOW;
                    }
                }
            }
        }
    }
    
}


void hardwareTest()
{
    if (lastHardwareTestPrintTime < millis() - 250)
    {
        lastHardwareTestPrintTime = millis();

        for (int i = 0; i < 30; i++)
        {
            Serial.print(i);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 0; i < 30; i++)
        {
            Serial.print(activeKeyswitches[i]);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 0; i < 30; i++)
        {
            Serial.print(activeTactSwitches[i]);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 30; i < 60; i++)
        {
            Serial.print(i);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 30; i < 60; i++)
        {
            Serial.print(activeKeyswitches[i]);
            Serial.print("\t");
        }
        Serial.println();

        for (int i = 30; i < 60; i++)
        {
            Serial.print(activeTactSwitches[i]);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 60; i < 90; i++)
        {
            Serial.print(i);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 60; i < 90; i++)
        {
            Serial.print(activeKeyswitches[i]);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 60; i < 90; i++)
        {
            Serial.print(activeTactSwitches[i]);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 90; i < 120; i++)
        {
            Serial.print(i);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 90; i < 120; i++)
        {
            Serial.print(activeKeyswitches[i]);
            Serial.print("\t");
        }
        Serial.println();
        for (int i = 90; i < 120; i++)
        {
            Serial.print(activeTactSwitches[i]);
            Serial.print("\t");
        }
        Serial.println();

        Serial.print("Rotary Enc Pos: ");
        Serial.print(rotaryEncoderPosition);
        Serial.print("\t");
        Serial.print("Rotary Enc Button: ");
        Serial.print(rotaryEncoderButton);
        Serial.print("\t");
        Serial.print("Top Pot Val: ");
        Serial.print(topPotValue);
        Serial.print("\t");
        Serial.print("Bottom Pot Val: ");
        Serial.print(bottomPotValue);
        Serial.print("\t");
        Serial.print("Foot Switch: ");
        Serial.print(footPedalButton);
        Serial.println();

        Serial.println();
    }
}


// MIDI PACKET FUNCTIONS

// Send MIDI Note On
// 1st byte = Event type (0x09 = note on, 0x08 = note off).
// 2nd byte = Event type bitwise ORed with MIDI channel.
// 3rd byte = MIDI note number.
// 4th byte = Velocity (7-bit range 0-127)
void noteOn(byte channel, byte pitch, byte velocity)
{
    if (loopRecordingEnabled == HIGH && loopTrackEnabled[channelMap[channelIndex]] == 0)            // If loop recording enabled
    {
        loopPacketTime[channel][loopRecordingIndex[channel]] = (millis() - loopStartTimestamp);     // Record event time in relation to loop start
        loopPacketType[channel][loopRecordingIndex[channel]] = 1;                                   // Record event type (1 = noteOn, 2 = noteOff, 3 = controlChange, 4 = pitchBendChange)
        loopPacketByte0[channel][loopRecordingIndex[channel]] = pitch;                              // Record pitch/control/highByte
        loopPacketByte1[channel][loopRecordingIndex[channel]] = velocity;                           // Record velocity/value/lowByte
        loopRecordingIndex[channel] = loopRecordingIndex[channel] + 1;                              // Increment the recording index
    }
    byte synthChannel = channel;
    channel = 0x90 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t noteOn = {0x09, channel, pitch, velocity};                // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(noteOn);                                                   // Send packet to the MIDI USB bus
    Serial1.write(0x90 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(pitch);                                                       // Send note number to the MIDI serial bus
    Serial1.write(velocity);                                                    // Send velocity value to the MIDI serial bus

    if (modeSelection == SYNTH)
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
                if (waveformPitch[synthChannel][myWaveform] == 255)                                        // Find the first available waveform in the 16 waveform array.  255 used as indicator.
                {
                    waveformPitch[synthChannel][myWaveform] = pitch;
                    waveformCh[synthChannel][myWaveform].amplitude(mapFloat(velocity, 0, 127, 0.0, 0.2) * mapFloat(bottomPotValue, 0, 127, 0.0, 1.0));
                    waveformCh[synthChannel][myWaveform].frequency(midiPitchFrequencyMap[pitch]);
                    envelopeCh[synthChannel][myWaveform].noteOn();
                    break;
                }
        }
    }

}

void loopNoteOn(byte channel, byte pitch, byte velocity)
{
    noteTracking[channel][pitch] = 1;
    byte synthChannel = channel;
    channel = 0x90 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t noteOn = {0x09, channel, pitch, velocity};                // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(noteOn);                                                   // Send packet to the MIDI USB bus
    Serial1.write(0x90 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(pitch);                                                       // Send note number to the MIDI serial bus
    Serial1.write(velocity);                                                    // Send velocity value to the MIDI serial bus

    if (modeSelection == SYNTH)
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
                if (waveformPitch[synthChannel][myWaveform] == 255)
                {
                    waveformPitch[synthChannel][myWaveform] = pitch;
                    waveformCh[synthChannel][myWaveform].amplitude(mapFloat(velocity, 0, 127, 0.0, 0.2) * mapFloat(bottomPotValue, 0, 127, 0.0, 1.0));
                    waveformCh[synthChannel][myWaveform].frequency(midiPitchFrequencyMap[pitch]);
                    envelopeCh[synthChannel][myWaveform].noteOn();
                    break;
                }
        }
    }

}

// Send MIDI Note Off
// 1st byte = Event type (0x09 = note on, 0x08 = note off).
// 2nd byte = Event type bitwise ORed with MIDI channel.
// 3rd byte = MIDI note number.
// 4th byte = Velocity (7-bit range 0-127)
void noteOff(byte channel, byte pitch, byte velocity)
{
    if (loopRecordingEnabled == HIGH && loopTrackEnabled[channelMap[channelIndex]] == 0)            // If loop recording enabled
    {
        loopPacketTime[channel][loopRecordingIndex[channel]] = (millis() - loopStartTimestamp);     // Record event time in relation to loop start
        loopPacketType[channel][loopRecordingIndex[channel]] = 2;                                   // Record event type (1 = noteOn, 2 = noteOff, 3 = controlChange, 4 = pitchBendChange)
        loopPacketByte0[channel][loopRecordingIndex[channel]] = pitch;                              // Record pitch/control/highByte
        loopPacketByte1[channel][loopRecordingIndex[channel]] = velocity;                           // Record velocity/value/lowByte
        loopRecordingIndex[channel] = loopRecordingIndex[channel] + 1;                              // Increment the recording index
    }

    byte synthChannel = channel;
    channel = 0x80 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t noteOff = {0x08, channel, pitch, velocity};               // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(noteOff);                                                  // Send packet to the MIDI USB bus
    Serial1.write(0x80 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(pitch);                                                       // Send note number to the MIDI serial bus
    Serial1.write(velocity);                                                    // Send velocity value to the MIDI serial bus

    if (modeSelection == SYNTH)
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
                if (waveformPitch[synthChannel][myWaveform] == pitch)                                        // Find matching waveform in the 16 waveform array, mark as available and send noteOff
                {
                    waveformPitch[synthChannel][myWaveform] = 255;
                    envelopeCh[synthChannel][myWaveform].noteOff();
                    break;
                }
        }
    }

}

void loopNoteOff(byte channel, byte pitch, byte velocity)
{
    noteTracking[channel][pitch] = 0;
    byte synthChannel = channel;
    channel = 0x80 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t noteOff = {0x08, channel, pitch, velocity};               // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(noteOff);                                                  // Send packet to the MIDI USB bus
    Serial1.write(0x80 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(pitch);                                                       // Send note number to the MIDI serial bus
    Serial1.write(velocity);                                                    // Send velocity value to the MIDI serial bus

    if (modeSelection == SYNTH)
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
                if (waveformPitch[synthChannel][myWaveform] == pitch)
                {
                    waveformPitch[synthChannel][myWaveform] = 255;
                    envelopeCh[synthChannel][myWaveform].noteOff();
                    break;
                }
        }
    }

}

// Send Control Change
// 1st byte = Event type (0x0B = Control Change).
// 2nd byte = Event type bitwise ORed with MIDI channel.
// 3rd byte = MIDI CC number (7-bit range 0-127).
// 4th byte = Control value (7-bit range 0-127).
void controlChange(byte channel, byte control, byte value)
{
    if (loopRecordingEnabled == HIGH && loopTrackEnabled[channelMap[channelIndex]] == 0)            // If loop recording enabled
    {
        loopPacketTime[channel][loopRecordingIndex[channel]] = (millis() - loopStartTimestamp);     // Record event time in relation to loop start
        loopPacketType[channel][loopRecordingIndex[channel]] = 3;                                   // Record event type (1 = noteOn, 2 = noteOff, 3 = controlChange, 4 = pitchBendChange)
        loopPacketByte0[channel][loopRecordingIndex[channel]] = control;                            // Record pitch/control/highByte
        loopPacketByte1[channel][loopRecordingIndex[channel]] = value;                              // Record velocity/value/lowByte
        loopRecordingIndex[channel] = loopRecordingIndex[channel] + 1;                              // Increment the recording index
    }
    // byte synthChannel = channel;
    channel = 0xB0 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t event = {0x0B, channel, control, value};                  // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(event);                                                    // Send packet to the MIDI USB bus
    Serial1.write(0xB0 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(control);                                                     // Send control change number to the MIDI serial bus
    Serial1.write(value);                                                       // Send control chnage value to the MIDI serial bus

/*
    // Apply a modulation effect to the onboard synth waveform
    if (modeSelection == SYNTH && control == 1) // CC #1 Modulation
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
            if (waveformPitch[synthChannel][myWaveform] != 255)
            {
                    // envelopeCh[synthChannel][myWaveform].noteOff();
                    // envelopeCh[synthChannel][myWaveform].noteOn();
            }
        }
    }
*/

}

void loopControlChange(byte channel, byte control, byte value)
{
    byte synthChannel = channel;
    channel = 0xB0 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t event = {0x0B, channel, control, value};                  // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(event);                                                    // Send packet to the MIDI USB bus
    Serial1.write(0xB0 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(control);                                                     // Send control change number to the MIDI serial bus
    Serial1.write(value);                                                       // Send control chnage value to the MIDI serial bus

    if (modeSelection == SYNTH && control == 1) // CC #1 Modulation
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
            if (waveformPitch[synthChannel][myWaveform] != 255)
            {
                waveformCh[synthChannel][myWaveform].pulseWidth(mapFloat(value, 0, 127, 0.25, 0.5));
            }
        }
    }

}

// Send Program Change
// 1st byte = Event type (0x0C = Program Change).
// 2nd byte = Event type bitwise ORed with MIDI channel.
// 3rd byte = Program value (7-bit range 0-127).
void programChange(byte channel, byte value)
{
    channel = 0xC0 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t event = {0x0C, channel, value};                           // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(event);                                                    // Send packet to the MIDI USB bus
    Serial1.write(0xC0 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(value);                                                       // Send program change value to the MIDI serial bus
}

// Send Pitch Bend
// (14 bit value 0-16363, neutral position = 8192)
// 1st byte = Event type (0x0E = Pitch bend change).
// 2nd byte = Event type bitwise ORed with MIDI channel.
// 3rd byte = The 7 least significant bits of the value.
// 4th byte = The 7 most significant bits of the value.
void pitchBendChange(byte channel, byte lowValue, byte highValue)
{
    if (loopRecordingEnabled == HIGH && loopTrackEnabled[channelMap[channelIndex]] == 0)            // If loop recording enabled
    {
        loopPacketTime[channel][loopRecordingIndex[channel]] = (millis() - loopStartTimestamp);     // Record event time in relation to loop start
        loopPacketType[channel][loopRecordingIndex[channel]] = 4;                                   // Record event type (1 = noteOn, 2 = noteOff, 3 = controlChange, 4 = pitchBendChange)
        loopPacketByte0[channel][loopRecordingIndex[channel]] = lowValue;                           // Record pitch/control/highByte
        loopPacketByte1[channel][loopRecordingIndex[channel]] = highValue;                          // Record velocity/value/lowByte
        loopRecordingIndex[channel] = loopRecordingIndex[channel] + 1;                              // Increment the recording index
    }
    byte synthChannel = channel;
    channel = 0xE0 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t bendEvent = {0x0E, channel, lowValue, highValue};         // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(bendEvent);                                                // Send packet to the MIDI USB bus
    Serial1.write(0xE0 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(lowValue);                                                    // Send pitch bend low byte to the MIDI serial bus
    Serial1.write(highValue);                                                   // Send pitch bend high byte to the MIDI serial bus


    if (modeSelection == SYNTH)
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
            if (waveformPitch[synthChannel][myWaveform] != 255)
            {
                float myPitchRangeFloor = midiPitchFrequencyMap[waveformPitch[synthChannel][myWaveform] - 2];       // Whole step
                float myPitchRangeCeiling = midiPitchFrequencyMap[waveformPitch[synthChannel][myWaveform] + 2];
                waveformCh[synthChannel][myWaveform].frequency(mapFloat(highValue, 0, 127, myPitchRangeFloor, myPitchRangeCeiling));
            }
        }
    }

}

void loopPitchBendChange(byte channel, byte lowValue, byte highValue)
{
    byte synthChannel = channel;
    channel = 0xE0 | channel;                                                   // Bitwise OR outside of the struct to prevent compiler warnings
    midiEventPacket_t bendEvent = {0x0E, channel, lowValue, highValue};         // Build a struct containing all of our information in a single packet
    MidiUSB.sendMIDI(bendEvent);                                                // Send packet to the MIDI USB bus
    Serial1.write(0xE0 | channel);                                              // Send event type/channel to the MIDI serial bus
    Serial1.write(lowValue);                                                    // Send pitch bend low byte to the MIDI serial bus
    Serial1.write(highValue);                                                   // Send pitch bend high byte to the MIDI serial bus

    if (modeSelection == SYNTH)
    {
        for (byte myWaveform = 0; myWaveform < 8; myWaveform++)
        {
            if (waveformPitch[synthChannel][myWaveform] != 255)
            {
                float myPitchRangeFloor = midiPitchFrequencyMap[waveformPitch[synthChannel][myWaveform] - 2];
                float myPitchRangeCeiling = midiPitchFrequencyMap[waveformPitch[synthChannel][myWaveform] + 2];
                waveformCh[synthChannel][myWaveform].frequency(mapFloat(highValue, 0, 127, myPitchRangeFloor, myPitchRangeCeiling));
            }
        }
    }

}

// END OF PROGRAM
//------------------------------------------------------------------------------------------------------------------------------------------------------------//
