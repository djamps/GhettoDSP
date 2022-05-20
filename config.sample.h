#pragma once

// Power settings
#define BROWNOUTVOLTAGE       11.5 // PWR or hoopty Shutoff if battery voltage is below this
#define VOLTAGEBATTMAXCHARGE  14.3 // PWR module Stop charging when battery reaches this level
#define VCOMP                 27.6 // PWR or hoopty ADC calibration

#define SPEAKER_NAME "GhettoKlipsch" // Bluetooth name (20 char max)
//                   "--------------------"

#define DSP_VERSION 29 // Must be increased each time DSP program changed

// Hardware settings
#define LCD2004   true  // 20x4 LCD
#define LCD2002   false // 20x2 LCD
#define BT        true  // BT module
#define ENCODER   true  // Encoder
#define PWR       true // Separate PWR board
#define GHETTODSP true // HooptyDSP board (external PWR)
#define HOOPTYDSP false  // HooptyDSP board (integrated PWR)

// Software settings
#define LOWCUT    false // Include low-cut option in dBass menu
#define CUSTOM    false // Use custom settings array
#define VUMETER   false  // Show stereo VU meter on home screen
#define VUREVERSE true  // Reverse VU meter movement
#define VUSLOW    true  // Slow down VU dropoff
#define VUPEAK    false // Peak indication
#define LINEINDSP false  // Line input via DSP
#define LINEINBT  true // Line in via BM64

// This section defines which settings included in LCD menu and which order

// Settings options:
// 1: dBass Mode
// 2: Bass
// 3: Treble
// 4: Low Level (3-way) // Ghetto only
// 5: Mid Level (3-way) // Ghetto only
// 6: High Level (3-way) // Ghetto only
// 7: System Gain
// 8: nSpk Mode
// 9: Aux in mode
// 10: Channel mode // Ghetto only
// 11: Fader // Hoopty only
// 12: Xover // Hoopty only
// 13: Subharmonics // Hoopty only
// 14: Subsonic Filter // Hoopty only
// 15: Call volume // Hoopty only
// 16: Midrange // Hoopty only

#if CUSTOM
  #define SETTINGS_ARRAY {1,2,3} // Custom settings
#elif HOOPTYDSP
  #define SETTINGS_ARRAY {2,16,3,11,13,9,12,14,8,15,7} // Default Hoopty settings
#elif GHETTODSP
  #define SETTINGS_ARRAY {1,2,3,9,10,8,4,5,6,7} // Default Ghetto settings
#endif

// Debugging
#define DEBUG   false // Startup
#define DEBUG2  false // Hoopty
#define DEBUG3  false // PWR/I2C
#define DEBUG4  false // Encoder
#define DEBUG5  false // BM64 All Events
#define DEBUG6  false // BM64 NSPK Status
#define DEBUG7  false // startAudio()
#define DEBUG8  false // Temp/misc

// Pin assignments for ghetto
#if GHETTODSP
  #define ENCODER_SW    2 // Input: Encoder knob
  #define ENCODER_CLK   4 // Input: Encoder knob
  #define ENCODER_DT    3 // Input: Encoder knob
  #define UART_RX       8 // Input: BM64 UART (software serial)
  #define UART_TX       9 // Output: BM64 UART (software serial)
  #define BT_RESET      7 // Output: BM64 reset line
  #define SOFT_MUTE     5 // Output: For AUX DAC, Low = mute, High = not mute
  #define BT_TX_IND     10 // Output: BM64 UART TX indication
  #define DSP_RESET     A2 // Output: DSP reset line 
  #define DSP_WP        6 // Output: DSP/eeprom write protect
  #define ADC0          A6 // Input: Aux ADC in (temp, tone, ect)
  #define ADC1          A7 // Input: Aux ADC in (temp, tone, ect)
#endif

// Pin assignments for hoopty
#if HOOPTYDSP
  #define ENCODER_SW    2 // Input: Encoder knob button
  #define ENCODER_CLK   4 // Input: Encoder knob clock
  #define ENCODER_DT    3 // Input: Encoder knob data
  #define UART_RX       8 // Input: BM64 UART (software serial)
  #define UART_TX       9 // Output: BM64 UART (software serial)
  #define BT_RESET      7 // Output: BM64 reset line
  #define SOFT_MUTE     5 // Output: For AUX DAC, Low = mute, High = not mute
  #define BT_TX_IND     10 // Output: BM64 UART TX indication
  #define DSP_RESET     A2 // Output: DSP reset line / digital
  #define PWRON         A3 // Output: Hold system power on / digital
  #define DSP_WP        6 // Output: DSP/eeprom write protect
  #define VACC          A6 // Input: Key switched input / analog
  #define ADC1          A7 // Input: Aux ADC / analog
  #define REMOTE        A1 // Output: Amplifier remote output / digital
  #define BUTTON        A0 // Input: Power button / MFB / digital
#endif
