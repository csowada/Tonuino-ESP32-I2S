#ifndef H_CONSTS
#define H_CONSTS

#include <stdint.h>

// Define modules to compile:
//#define MQTT_ENABLE
#define FTP_ENABLE
#define NEOPIXEL_ENABLE             // Don't forget configuration of NUM_LEDS
#define NEOPIXEL_REVERSE_ROTATION   // Some Neopixels are adressed/soldered counter-clockwise. This can be configured here.
// #define ROTARY_SWITCH_ENABLE

#define TOUCH_BUTTONS_ENABLE

// Use PN532 either MFRC522 as RFID reader module
#define RFID_PN532
// #define RFID_MFRC522

// Info-docs:
// https://docs.aws.amazon.com/de_de/freertos-kernel/latest/dg/queue-management.html
// https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html#how-do-i-declare-a-global-flash-string-and-use-it

// Loglevels
#define LOGLEVEL_ERROR                  1           // only errors
#define LOGLEVEL_NOTICE                 2           // errors + important messages
#define LOGLEVEL_INFO                   3           // infos + errors + important messages
#define LOGLEVEL_DEBUG                  4           // almost everything

// GPIOs (uSD card-reader)
#define SPISD_CS                        5
#define SPISD_MOSI                      23
#define SPISD_MISO                      19         // 12 doesn't work with Lolin32-devBoard: uC doesn't start if put HIGH at start
#define SPISD_SCK                       18

// GPIOs (RFID-readercurrentRfidTagId)
// #define RST_PIN                         31
// #define RFID_CS                         15
// #define RFID_MOSI                       23
// #define RFID_MISO                       19
// #define RFID_SCK                        18
// #define RFID_IRQ                        22

// GPIOs (DAC)
#define I2S_DOUT                        25
#define I2S_BCLK                        27
#define I2S_LRC                         26

// GPIO used to trigger transistor-circuit / RFID-reader
#define POWER                           4

// GPIOs (Rotary encoder)
#ifdef ROTARY_SWITCH_ENABLE
    #define DREHENCODER_CLK                 12
    #define DREHENCODER_DT                  14
    #define DREHENCODER_BUTTON              35
#endif

// GPIOs (Control-buttons)
#define PAUSEPLAY_BUTTON                15//0
#define NEXT_BUTTON                     33//0//2
#define PREVIOUS_BUTTON                 2//15

#define PAUSEPLAY_BUTTON_THRESHOLD      50
#define NEXT_BUTTON_THRESHOLD           50
#define PREVIOUS_BUTTON_THRESHOLD       50

// GPIOs (LEDs)
#define LED_PIN                         14

// Neopixel-configuration
#ifdef NEOPIXEL_ENABLE
    #define NUM_LEDS                    16          // number of LEDs
    #define CHIPSET                     WS2812B     // type of Neopixel
    #define COLOR_ORDER                 GRB
#endif

// Track-Control
#define STOP                            1           // Stop play
#define PLAY                            2           // Start play (currently not used)
#define PAUSEPLAY                       3           // Pause/play
#define NEXTTRACK                       4           // Next track of playlist
#define PREVIOUSTRACK                   5           // Previous track of playlist
#define FIRSTTRACK                      6           // First track of playlist
#define LASTTRACK                       7           // Last track of playlist

// Playmodes
#define NO_PLAYLIST                     0           // If no playlist is active
#define SINGLE_TRACK                    1           // Play a single track
#define SINGLE_TRACK_LOOP               2           // Play a single track in infinite-loop
#define AUDIOBOOK                       3           // Single track, can save last play-position
#define AUDIOBOOK_LOOP                  4           // Single track as infinite-loop, can save last play-position
#define ALL_TRACKS_OF_DIR_SORTED        5           // Play all files of a directory (alph. sorted)
#define ALL_TRACKS_OF_DIR_RANDOM        6           // Play all files of a directory (randomized)
#define ALL_TRACKS_OF_DIR_SORTED_LOOP   7           // Play all files of a directory (alph. sorted) in infinite-loop
#define ALL_TRACKS_OF_DIR_RANDOM_LOOP   9           // Play all files of a directory (randomized) in infinite-loop
#define WEBSTREAM                       8           // Play webradio-stream
#define PLAYER_BUSY                     10          // Used if playlist is created

// RFID-modifcation-types
#define LOCK_BUTTONS_MOD                100         // Locks all buttons and rotary encoder
#define SLEEP_TIMER_MOD_15              101         // Puts uC into deepsleep after 15 minutes + LED-DIMM
#define SLEEP_TIMER_MOD_30              102         // Puts uC into deepsleep after 30 minutes + LED-DIMM
#define SLEEP_TIMER_MOD_60              103         // Puts uC into deepsleep after 60 minutes + LED-DIMM
#define SLEEP_TIMER_MOD_120             104         // Puts uC into deepsleep after 120 minutes + LED-DIMM
#define SLEEP_AFTER_END_OF_TRACK        105         // Puts uC into deepsleep after track is finished + LED-DIMM
#define SLEEP_AFTER_END_OF_PLAYLIST     106         // Puts uC into deepsleep after playlist is finished + LED-DIMM
#define SLEEP_AFTER_5_TRACKS            107         // Puts uC into deepsleep after five tracks
#define REPEAT_PLAYLIST                 110         // Changes active playmode to endless-loop (for a playlist)
#define REPEAT_TRACK                    111         // Changes active playmode to endless-loop (for a single track)
#define DIMM_LEDS_NIGHTMODE             112         // Changes LED-brightness
#define VOLUME_DECREASE                 113         // Lautstärke leiser
#define VOLUME_INCREASE                 114         // Lautstärke lauter

// Repeat-Modes
#define NO_REPEAT                       0
#define TRACK                           1
#define PLAYLIST                        2
#define TRACK_N_PLAYLIST                3

typedef struct { // Bit field
    uint8_t playMode:                   4;      // playMode
    char **playlist;                            // playlist
    bool repeatCurrentTrack:            1;      // If current track should be looped
    bool repeatPlaylist:                1;      // If whole playlist should be looped
    uint16_t currentTrackNumber:        9;      // Current tracknumber
    uint16_t numberOfTracks:            9;      // Number of tracks in playlist
    unsigned long startAtFilePos;               // Offset to start play (in bytes)
    uint8_t currentRelPos:              7;      // Current relative playPosition (in %)
    bool sleepAfterCurrentTrack:        1;      // If uC should go to sleep after current track
    bool sleepAfterPlaylist:            1;      // If uC should go to sleep after whole playlist
    bool saveLastPlayPosition:          1;      // If playposition/current track should be saved (for AUDIOBOOK)
    char playRfidTag[13];                       // ID of RFID-tag that started playlist
    bool pausePlay:                     1;      // If pause is active
    bool trackFinished:                 1;      // If current track is finished
    bool playlistFinished:              1;      // If whole playlist is finished
    uint8_t playUntilTrackNumber:       6;      // Number of tracks to play after which uC goes to sleep
} playProps;

// Button-helper
typedef struct {
    bool lastState;
    bool currentState;
    bool isPressed;
    bool isReleased;
    unsigned long lastPressedTimestamp;
    unsigned long lastReleasedTimestamp;
} t_button;

typedef struct {
    char nvsKey[13];
    char nvsEntry[275];
} nvs_t;

extern bool gotoSleep;                                 // Flag for turning uC immediately into deepsleep
extern bool bootComplete;
extern bool lockControls;

extern uint8_t currentVolume;                           
extern uint8_t maxVolume;                               // Maximum volume that can be adjusted
extern uint8_t minVolume;                               // Lowest volume that can be adjusted
extern uint8_t initVolume;                              // 0...21 (If not found in NVS, this one will be taken)

extern playProps playProperties;

extern t_button buttons[];

extern const uint8_t serialDebug;

#endif