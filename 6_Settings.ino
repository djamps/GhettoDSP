void readEEPROM() {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2]) {
    for (unsigned int t = 0; t < sizeof(settings); t++) {
      *((char*)&settings + t) = EEPROM.read(CONFIG_START + t);
    }
  } else {
    if ( DEBUG ) {
      Serial.println(F("EEPROM Load failed"));
    }
  }
    #if DEBUG4
      Serial.print(F("Read eep: "));
      Serial.println(settings.eqMode);
    #endif
}

void writeEEPROM() {
  if ( DEBUG ) {
    Serial.println(F("Writing EEPROM"));
  }
  for (unsigned int t = 0; t < sizeof(settings); t++)
    EEPROM.write(CONFIG_START + t, *((char*)&settings + t));

    #if DEBUG4
      Serial.print(F("Wrote eep: "));
      Serial.println(settings.eqMode);
    #endif
}

bool valueInArray(uint8_t val, uint8_t *arr, size_t n) {
    for(size_t i = 0; i < n; i++) {
        if(arr[i] == val) {
            #if DEBUG4
             Serial.print(F("Skipped: "));
             Serial.println(arr[i]);
            #endif
            return true;
        }
    }
    #if DEBUG4
      Serial.print(F("Allowed: "));
      Serial.println(val);
    #endif
    return false;
}

// Advance settings mode +1
void advSettingMode() {

  #if DEBUG4
   Serial.println(F("Adv setting"));
  #endif
      
  uint8_t disabledSettingsArray[] = DISABLED_SETTINGS_IN_EQ_MODE;
  bool gotSetting = false;
  
  if ( settingMode == 0 ) {
    settingMode = settingsArray[0];
    return;
  }
  
  // Find current setting mode in array and advance one
  uint8_t nSize = sizeof(settingsArray);
  uint8_t i = 0;
  while( i < nSize ) {
    
    // Did we reach the current setting?
    if ( settingMode == settingsArray[i] ) {
      gotSetting = true;
    }
    if (gotSetting) {
      // Skip certain DSP settings in a fixed EQ mode
      if ( settings.eqMode == 7 || (settings.eqMode < 7 && !valueInArray(settingsArray[i+1], disabledSettingsArray, sizeof(disabledSettingsArray)))) {
        #if DEBUG4
          Serial.print(F("Next: "));
          Serial.println(settingsArray[i + 1]);
        #endif
        settingMode = settingsArray[i + 1];
        return;
      }
      #if DEBUG4
        Serial.print(F("Failed1 "));
        Serial.println(settingsArray[i + 1]);
      #endif
    }
    i++;
  }
  Serial.println(F("Failed2"));
}

void adjLevel(int8_t change) {
  switch (settingMode) {
    case 1:
      uint8_t dBassModeMax;
      #if LOWCUT
        dBassModeMax = 2;
      #else
        dBassModeMax = 1;
      #endif
      adjSetting(settings.dBassMode,change, 0, dBassModeMax);
      #if GHETTODSP
        setDbassMode(settings.dBassMode);
      #endif
      break;
    case 2:
      adjSetting(settings.bassLevel,change, -10, 10);
      setDspParams();
      break;
    case 3:
      adjSetting(settings.trebleLevel,change, -10, 10);
      setDspParams();
      break;
    case 4:
      adjSetting(settings.lowLevel,change, -12, 0);
      setDspParams();
      break;
    case 5:
      adjSetting(settings.midLevel,change, -12, 0);
      setDspParams();
      break;
    case 6:
      adjSetting(settings.highLevel,change, -12, 0);
      setDspParams();
      break;
    case 7:
      adjSetting(settings.gainLevel,change, -6, 6);
      setSysGain();
      break;
    case 8:
      adjSetting(settings.spkMode,change, 0, 4);
      break;
    #if BT
      case 9:
        settings.sourceMode = !settings.sourceMode;
        setSourceMode();
        break;
    #endif
    case 10:
      adjSetting(settings.channelMode,change, 0, 2);
      setSysGain();
      break;
    #if HOOPTYDSP
      case 11:
        adjSetting(settings.faderLevel,change, -10, 10);
        setFader();
        break;
      case 12:
        adjSetting(settings.xOverMode,change, 0, 7);
        setDspParams();
        break;
      case 13:
        adjSetting(settings.subharmonicLevel,change, 0, 10);
        setDspParams();
        break;
      case 14:
        adjSetting(settings.subsonicMode,change, 0, 1);
        setDspParams();
        break;
      case 15:
        adjSetting(settings.callVolume,change, -20, -6);
        setCallVolume();
        break;
      case 16:
        adjSetting(settings.midrangeLevel,change,-10,10);
        setDspParams();
        break;
    #endif
    case 17:
      adjSetting(settings.loudnessMode,change,0,1);
      setDspParams();
      break;
    case 18:
      adjSetting(settings.correctionsMode,change,0,1);
      setDspParams();
      break;
    case 19:
      adjSetting(settings.eqMode,change,0,7);
      setDspParams();
      break;
  }
  #if LCD2002 || LCD2004
    showSettings();
  #endif
  lastSettingChange = millis();
}


void adjSetting ( int8_t &setting, int8_t &change, int8_t lower, int8_t upper ) {
  setting += change;
  setting = min(setting,upper);
  setting = max(setting,lower);
}
