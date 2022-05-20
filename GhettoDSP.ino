#include "config.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SigmaDSP.h>
#include "SigmaDSP_parameters.h"
#include <EEPROM.h>
#include <math.h>

#if BT
#include <SoftwareSerial.h>
//#include <AltSoftSerial.h>
#include <BM64.h>
#endif

#if PWR
#include <I2C_Anything.h>
#endif

#define SW_VERSION "1.12"
#define CONFIG_VERSION "1.2" // Must be increased whenever persistant config params are modified
#define CONFIG_START 32 // Where in EEPROM to store persistant config
#define PWR_I2C_ADDRESS 0x30 // I2C address of power supply

// Set our bluetooth advertisement name
// Max 20 chars
char speakerName[21] = SPEAKER_NAME;

uint8_t settingsArray[] = SETTINGS_ARRAY;

// Settings object structure
struct SettingsStruct {
  char version[4];
  // The variables of your settings
  int8_t
  lowLevel,
  midLevel,
  highLevel,
  bassLevel,
  trebleLevel,
  gainLevel,
  spkMode,
  sourceMode,
  dBassMode,
  channelMode,
  powerState,
  faderLevel,
  xOverMode,
  subharmonicLevel,
  subsonicMode,
  callVolume,  // in DB
  midrangeLevel;
} settings = {
  CONFIG_VERSION,
  // The default values
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -20, 0
};

// Init libraries
SigmaDSP dsp(Wire, DSP_I2C_ADDRESS, 48000.00f /*,12*/); // Set up DSP at 48khz
DSPEEPROM ee(Wire, EEPROM_I2C_ADDRESS, 128); // Set up eeprom

#if LCD2002 || LCD2004
LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);
// Custom chars
uint8_t solidChar[8]   = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}; // Solid block
uint8_t speakerChar[8] = {0x01,0x03,0x05,0x19,0x19,0x05,0x03,0x01}; // Speaker
uint8_t playChar[8]    = {0x08,0x0C,0x0E,0x0F,0x0E,0x0C,0x08,0x00}; // Play
uint8_t stopChar[8]    = {0x00,0x00,0x0F,0x0F,0x0F,0x0F,0x00,0x00}; // Stop
uint8_t batt0[8]       = {0x0C,0x1E,0x12,0x12,0x12,0x12,0x12,0x1E}; // Batt 0%
uint8_t batt20[8]      = {0x0C,0x1E,0x12,0x12,0x12,0x12,0x1E,0x1E}; // Batt 20%
uint8_t batt40[8]      = {0x0C,0x1E,0x12,0x12,0x12,0x1E,0x1E,0x1E}; // Batt 40%
uint8_t batt60[8]      = {0x0C,0x1E,0x12,0x12,0x1E,0x1E,0x1E,0x1E}; // Batt 60%
uint8_t batt80[8]      = {0x0C,0x1E,0x12,0x1E,0x1E,0x1E,0x1E,0x1E}; // Batt 80%
uint8_t batt1000[8]      = {0x0C,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E}; // Batt 100%
#endif

uint8_t NSPK_CONNECT = 0;
uint8_t NSPK_STATE = 0;
uint8_t NSPK_GROUP = 0;

bool callActive = 0;
bool callInProgress = 0;

float voltageBatt = 0; // Measured power supply voltage (instantanious)
int unsigned voltageBattAvgCount = 0; // Related to battery voltage averaging
float voltageBattAvgSum = 0; // Related to battery voltage averaging
unsigned long voltageBattAvgLast = millis(); // Related to battery voltage averaging
float voltageBattAvg = 0; // Slow average battery voltage reported from power module
float voltageBuck = 0; // Instantanious buck voltage (should be 5v) reported from power module
float voltageBoost = 0; // Instantanious boost voltage (variable) reported from power module
float voltageCharge = 0; // Instantanious boost/buck charge input voltage (variable) reported from power module
float tempC = 0; // Temperature of PWR board
uint8_t statePower = 2; // Power state reported from power module (1 = on, 0 = off, 2 = unknown)
bool stateCharging; // Charge state reported from power module (1 = charging, 0 = discharging)
uint8_t percentBattery = 0; // Battery state reported from power module
uint8_t stateSys = 0; // System/LED status reported from power module

uint8_t stateBattery = 2; // Battery state from power module (assume good)

char phoneName1[14]; // Name of BT device on connection 1
char phoneName2[14]; // Name of BT device on connection 2
char metaDataSong[61]; // Song metadata
char metaDataArtist[61]; // Artist metadata
char callerID[32]; // Incoming phone number
bool auxDetect = 0; // Aux input signal detect

bool deviceConnected1 = 0; // Device is connected to slot 1
bool deviceConnected2 = 0; // Device is connected to slot 2
bool devicePlaying1 = 0; // Device is playing on slot 1
bool devicePlaying2 = 0; // Device is playing on slot 2
bool needToCheckDeviceNames = 0; // Flag to force a query of connected devices
bool needToLinkBack = 0; // Flag to force a reconnect of previous device(s)
bool whichLinkToShow = 0; // Flag for LCD display alternating between connected devices

uint8_t settingMode = 0; // Current settings mode (LCD)
int8_t currentSpkMode; // tracking the Spk mode (for linking with other speakers via BM64 concert mode)

unsigned long lastSettingChange = 0; // Used for doing occasional eeprom writes
unsigned long last50mSecTask = 0; // 50-mSec task timer
unsigned long last500mSecTask = 0; // 500-mSec task timer
unsigned long last1SecTask = 0; // 1-Sec task timer
unsigned long last5SecTask = 0; // 5-Sec task timer


unsigned long lastTimeLinkShown = millis(); // Timer for which device to show


// Set up communication with BT module (UART)
#if BT
SoftwareSerial swSerial(UART_RX, UART_TX);
//AltSoftSerial swSerial(UART_RX, UART_TX);
BM64 bm64(swSerial, BT_TX_IND);
#endif

void setup() {
  // Load settings/state
  readEEPROM();

  // Start USB serial and UART
  Serial.begin(115200);

#if BT
  swSerial.begin(9600);
#endif

#if DEBUG || DEBUG2 || DEBUG3 || DEBUG4 || DEBUG5 || DEBUG6 || DEBUG7
  Serial.println(F("*****************************"));
#endif

#if HOOPTYDSP
  // Set up remote output and acc/key sense
  digitalWrite(REMOTE, LOW);
  pinMode(REMOTE, OUTPUT);
  pinMode(PWRON, INPUT); // Input = off, output high = on.
  pinMode(VACC, INPUT);
  pinMode(ADC1, INPUT);
  pinMode(BUTTON, INPUT);
#endif

#if GHETTODSP
  // GhettoDSP ADC inputs
  pinMode(ADC0, INPUT);
  pinMode(ADC1, INPUT);
#endif

  // Mute external DAC
  pinMode(SOFT_MUTE, OUTPUT);
  digitalWrite(SOFT_MUTE, LOW);

  // Halt the DSP
  pinMode(DSP_RESET, OUTPUT);
  pinMode(DSP_WP, OUTPUT);
  digitalWrite(DSP_WP, LOW);
  digitalWrite(DSP_RESET, LOW);

  // Start the LCD 20x4
#if DEBUG
  Serial.println(F("Starting LCD"));
#endif

#if LCD2004 || LCD2002
  #if LCD2004
    lcd.begin(20, 4);
  #elif LCD2002
    lcd.begin(20, 2);
  #endif
  lcd.createChar(0, solidChar); // Sends the custom char to lcd
  lcd.createChar(1, speakerChar); // Sends the custom char to lcd
  lcd.createChar(2, playChar); // Sends the custom char to lcd
  lcd.createChar(3, stopChar); // Sends the custom char to lcd
#endif

#if DEBUG
  Serial.println(F("Clearing LCD"));
#endif

#if LCD2002 || LCD2004
  //lcd.setBacklight(63);
  lcd.clear();
  char buffer[21];
  sprintf(buffer, "S/W Version %s", SW_VERSION);
  lcdPrintCentered(buffer);
#endif

  // Set BT event callback
#if BT
  bm64.setCallback(onEventCallback);
#endif

  // and Init sigmadsp control
#if DEBUG
  Serial.println(F("Init DSP libs"));
#endif

  dsp.begin();
  ee.begin();

#if DEBUG
  Serial.print(F("EEPROM reply: "));
  Serial.println(ee.ping() ? F("Not present") : F("Present"));
#endif

  if ( ee.ping() ) {
#if LCD2002 || LCD2004
    lcd.setCursor(0, 1);
    lcdPrintCentered(F("No EEPROM found!"));
#endif
    while (1);
  }

  // Set up encoder
#if ENCODER
  setupEncoder();
#endif

#if PWR
  // Send data to PWR immediately if present
  sendDataToPWR();
#endif

#if !PWR && GHETTODSP
  startAudio();
#endif

#if HOOPTYDSP
  readVoltages();
  initPowerState();
#endif

#if DEBUG
  Serial.println(F("Init complete"));
#endif
}


void loop() {

  if ( statePower == 1 ) {

    // Look for encoder action
    #if ENCODER
      handleEncoder();
    #endif

    // Handle any BT UART events
    #if BT
      checkUartEvent();
    #endif

    // Link back if needed
    #if BT
      if ( needToLinkBack != 0 ) {
        bm64.linkBack();
        needToLinkBack = 0;
      }
    #endif

    // Query phone names if new connection event found
    #if BT
      if ( needToCheckDeviceNames == 1 ) {
  //      bm64.getPhoneName1();
  //      bm64.getPhoneName2();
        needToCheckDeviceNames = 0;
      }
    #endif

    // Handle phone link display
    #if BT
      if ( millis() > 5000 && millis() - 5000 >= lastTimeLinkShown ) {
        #if LCD2004 || LCD2002
          if ( LCD2002 && !devicePlaying1 && !devicePlaying2 ) {
            showLinks();
          } else if ( LCD2004 ) {
            showLinks();
          }
        #endif
        lastTimeLinkShown = millis();
      }
    #endif

    // Handle EEPROM storage of settings and speaker mode changes
    if ( lastSettingChange > 0 && millis() > 5000 && millis() - 5000 > lastSettingChange ) {
      writeEEPROM();
      lastSettingChange = 0;
      if ( !callActive) {
        settingMode = 0;
      }
      showSettings();
      #if BT
        if ( currentSpkMode != settings.spkMode ) {
          setBTName();
          setSpkMode();
          currentSpkMode = settings.spkMode;
        }
      #endif
    }

    // Handle these 500ms tasks
    if ( millis() > 500 && millis() - 500 >= last500mSecTask  ) {
      #if LCD2002 || LCD2004
        showMetaData();
      #endif
      checkForAudio();
      #if PWR
        getDataFromPWR();
      #endif
      #if HOOPTYDSP
        readVoltages();
      #endif

      //if ( settingsMode == 0 { showSettings(); }
      last500mSecTask = millis();
    }
    // Handle these 50ms tasks
    if ( millis() > 50 && millis() - 50 >= last50mSecTask  ) {
    #if LCD2002 || LCD2004
      #if VUMETER
        if ( settingMode == 0 ) {
          #if LCD2002
            if ( devicePlaying1 || devicePlaying2 ) {
              lcdPrintStereoVu(0); // show on line 0
            } else if ( deviceConnected1 || deviceConnected2 ) {
              showStatePower(0);
            }
          #elif LCD2004
            lcdPrintStereoVu(1); // show on line 1
          #endif
        }
      #endif
    #endif

      last50mSecTask = millis();
    }

    // Handle these 5-sec tasks
    if ( millis() > 5000 && millis() - 5000 >= last5SecTask  ) {
#if BT
      getMetaData();
#endif
      #if LCD2004 && ( (GHETTODSP && PWR) || HOOPTYDSP )
        showStatePower(0); // show on line 0
      #endif
      last5SecTask = millis();
    }

    // Handle these 1-sec tasks
    if ( millis() > 1000 && millis() - 1000 >= last1SecTask ) {
      last1SecTask = millis();
    }

#if HOOPTYDSP
    checkPowerButton();
#endif

  } else {
    
    ////////////////////// Stuff to do in powered down state //////////////////////

#if HOOPTYDSP
    checkPowerButton();
#endif

    // Handle these 5-sec tasks
    if ( millis() > 5000 && millis() - 5000 >= last5SecTask  ) {
      #if LCD2002 || LCD2004
        showStatePower(0); // show on line 0
      #endif
      last5SecTask = millis();
    }
    
    // Handle these 500ms tasks
    if ( millis() > 500 && millis() - 500 >= last500mSecTask  ) {
#if PWR
      getDataFromPWR();
#endif

#if HOOPTYDSP
      readVoltages();
#endif
      last500mSecTask = millis();
    }
  }
}

uint8_t FSHlength(const __FlashStringHelper * FSHinput) {
  PGM_P FSHinputPointer = reinterpret_cast<PGM_P>(FSHinput);
  uint8_t stringLength = 0;
  while (pgm_read_byte(FSHinputPointer++)) {
    stringLength++;
  }
  return stringLength;
}

char buffer[21]; // For flash string helper to char* converting
const char * ftc(const __FlashStringHelper *text) {
  const byte textLength = FSHlength(text);
  memcpy_P(buffer, text, textLength + 1);  //+1 for the null terminator
  return buffer;
}
