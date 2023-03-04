#if LCD2004 || LCD2002

uint8_t scrollPosition = 0; // Flag to store scrolling position
uint8_t overScrolls = 0;
uint8_t lenMetaLast = 0;
uint8_t graphLastL = 0;
uint8_t graphLastR = 0;
#if VUPEAK
uint8_t peakL = 0;
uint8_t peakR = 0;
unsigned long lastPeakL;
unsigned long lastPeakR;
#endif

#if VUMETER
void lcdPrintStereoVu(uint8_t row)
{
  // True peak Db VU meter using peak-hold envelope module in ADAU1701 DSP.  
  // With envelope-hold of 50ms in DSP the meter does not miss brief peaks
  // with 50ms refresh rate.
  
  // Read VU meters
  uint32_t currentValueL = dsp.readBack(MOD_READBACKL_ALG0_VAL0_ADDR, MOD_READBACKL_ALG0_VAL0_VALUES, 3);
  uint32_t currentValueR = dsp.readBack(MOD_READBACKR_ALG0_VAL0_ADDR, MOD_READBACKR_ALG0_VAL0_VALUES, 3);
  
  static const uint8_t zeroRef = 58; // Full scale DB measure w/Iphone 0 Db test tone, scope output to confirm clip limit
  
  static const uint8_t dBPerLed = 1; // Db per LED/block
  static const uint8_t ledCount = 10; // 10 per channel on 20 wide
  
  static const uint8_t adj = zeroRef - (ledCount*dBPerLed) - 1;
  
  float volumeL = 20*log10(sqrt(currentValueL+1)); // Generate DB value
  float volumeR = 20*log10(sqrt(currentValueR+1)); // Generate DB value

  // Scale to bar graph specs
  volumeL = fmax(0,volumeL-adj)/dBPerLed;
  volumeR = fmax(0,volumeR-adj)/dBPerLed;

  // Map the bargraph to adjusted dB levels
  int currentGraphL = fmin(ledCount,map(round(volumeL),0,ledCount,0,ledCount));
  int currentGraphR = fmin(ledCount,map(round(volumeR),0,ledCount,0,ledCount));
  
  #if VUPEAK
  
  if ( currentGraphL == ledCount ) {
    peakL = 1;
    lastPeakL = millis();
  }
  
  if ( currentGraphR == ledCount ) {
    peakR = 1;
    lastPeakR = millis();
  }

  if ( millis() - 250 >= lastPeakL ) {
    peakL = 0;
  }

  if ( millis() - 250 >= lastPeakR ) {
    peakR = 0;
  }

  #endif

  #if DEBUG8
    Serial.print(adj);
    Serial.print(" ");
    Serial.print(volumeL);
    Serial.print(" ");
    Serial.println(currentGraphL);
    return;
  #endif
  
  #if VUSLOW
    // Make LEDs fall slower
    if ( currentGraphL > graphLastL ) { graphLastL = currentGraphL; } else { currentGraphL = graphLastL; }
    if ( currentGraphR > graphLastR ) { graphLastR = currentGraphR; } else { currentGraphR = graphLastR; }
  #endif
  
  int column;

#if VUREVERSE // reverse meters
  // Left meter (forward from left side)
  for (column = 0; column < 0 + currentGraphL; column++)
  {
    lcd.setCursor(column, row);
    lcd.print((char)0);
  }
  #if VUPEAK
    while (column++ < 10 - peakL)
    #else
    while (column++ < 10)
    #endif
    {
      lcd.setCursor(column-1, row);
      lcdPrintSpace();
    }
  
    // Right meter (Backwards from right side)
    for (column = 19; column > 19 - currentGraphR; column--)
    {
      lcd.setCursor(column, row);
      lcd.print((char)0);
    }
    column++;
    #if VUPEAK
    while (column-- > 10 + peakR)
    #else
    while (column-- > 10)
    #endif
    {
      lcd.setCursor(column, row);
      lcdPrintSpace();
    }

  #else // Forward meters
  
    // Left meter (backward from middle)
    for (column = 10; column > 10 - currentGraphL; column--)
    {
      lcd.setCursor(column-1, row);
      lcd.print((char)0);
    }
    column++;
    while (column-- > 0)
    {
      lcd.setCursor(column-1, row);
      lcdPrintSpace();
    }
  
    // Right meter (forward from middle)
    for (column = 10; column < 10 + currentGraphR; column++)
    {
      lcd.setCursor(column, row);
      lcd.print((char)0);
    }
    column--;
    while (column++ < 19)
    {
      lcd.setCursor(column, row);
      lcdPrintSpace();
    }
  #endif
  #if VUSLOW
    if ( graphLastL > 6 ) { graphLastL-=2; } else if ( graphLastL > 0 )  { graphLastL--; }
    if ( graphLastR > 6 ) { graphLastR-=2; } else if ( graphLastR > 0 )  { graphLastR--; }
  #endif
}
#endif

void lcdClearLine(const uint8_t &line)
{
  lcd.setCursor(0, line);
  for (int n = 0; n < 20; n++)
  {
    lcdPrintSpace();
  }
}

void lcdPrintSpace()
{
  lcd.print(" ");
}

void lcdPrintCentered(const __FlashStringHelper * text) {
  lcdPrintCentered(ftc(text));
}

void lcdPrintCentered(const char * text)
{
  int leftPad = (20 - strlen(text)) / 2;
  for ( uint8_t i = 0; i < 20;  i++ ) {
    if ( i < leftPad ) {
      lcdPrintSpace();
    } else if (i < leftPad + strlen(text)) {
      lcd.print(text[i - leftPad]);
    } else {
      lcdPrintSpace();
    }
  }
}

void lcdPrintLeft(const char * text)
{
  uint8_t rightPad = 20 - strlen(text);
  for ( uint8_t i = 0; i < 20;  i++ ) {
    if ( i < rightPad ) {
      lcd.print(text[i]);
      lcdPrintSpace();
    } else {
      lcdPrintSpace();
    }
  }
}

void lcdPrintSetting(const __FlashStringHelper *setting, const __FlashStringHelper *value) {
  const byte settingLength = FSHlength(setting);
  const byte valueLength = FSHlength(value);
  char buffer1[settingLength + 1];
  char buffer2[valueLength + 1];
  memcpy_P(buffer1, setting, settingLength + 1);  //+1 for the null terminator
  memcpy_P(buffer2, value, valueLength + 1);  //+1 for the null terminator
  lcdPrintSetting(buffer1, buffer2);
}

void lcdPrintSetting(const char * setting, int value) {
  char buffer2[3];
  sprintf(buffer2, "%d",value);
  lcdPrintSetting(setting, buffer2);
}

void lcdPrintSetting(const char * settingName, const char * settingValue)
{
  char buf[21];
  //lcd.setCursor(0, row);
  strcpy(buf, settingName);
  strcat(buf, " ");
  strcat(buf, settingValue);
  lcdPrintCentered(buf);
}

void lcdPrintScroll(char * text)
{
  // Metadata changed, reset scrolling
  if ( strlen(text) != lenMetaLast ) {
    lenMetaLast = strlen(text);
    scrollPosition = 0;
    overScrolls = 6;
  }
  if ( strlen(text) <= 20 ) {
    lcdPrintCentered(text);
    scrollPosition = 0;
    overScrolls = 6;
  } else {
    // Scrolling needed
    for ( uint8_t i = scrollPosition; i <= 20 + scrollPosition;  i++ ) {
      // Break if we exceed length of text
      // Also reset scroll position after some delay
      if ( i >= strlen(text)) {
        overScrolls++;
        break;
      } else if ( i - scrollPosition < 20 ) {
        lcd.print(text[i]);
      }
    }
  }
  if ( scrollPosition > 0 && overScrolls > 5 ) {
    scrollPosition--;
  } else if ( scrollPosition == 0 && overScrolls > 0 ) {
    overScrolls--;
  } else if ( overScrolls == 0 ) {
    scrollPosition++;
  }
}

void showMetaData() {
  #if LCD2004
    const uint8_t line = 2; 
  #else
    const uint8_t line = 1; 
  #endif
  lcd.setCursor(0, line);
  if ( settings.sourceMode == 1 )
  {
    #if LINEINDSP
    if ( audioDetected == 1 ) {
    #else
    if ( auxDetect == 1 ) {
    #endif
      lcdPrintCentered(F("Aux-in Playing"));
    } else {
      lcdPrintCentered(F("Aux-in Silence"));
    }
  } else {
    if ( !callActive ) {
      // Show metadata
      // Combine artist - song if needed
      char buf[121];
      if ( strlen(metaDataSong) > 0 && strlen(metaDataArtist) > 0 ) {
        strcpy(buf, metaDataSong);
        strcat(buf, " by ");
        strcat(buf, metaDataArtist);
        lcdPrintScroll(buf);
      } else if ( strlen(metaDataSong) > 0 ) {
        lcdPrintScroll(metaDataSong);
      } else if ( strlen(metaDataArtist) > 0 ) {
        lcdPrintScroll(metaDataArtist);
      } else {
        // No metadata or not playing
        #if LCD2004
          lcdClearLine(line);
        #elif LCD2002
          showLinks();
        #endif
      }
    } else {
      // Call in progress
      // Show caller ID
      if ( strlen ( callerID ) > 0 ) {
        lcdPrintScroll(callerID);
      } else {
        lcdPrintCentered(F("Call In Progress"));
      }
    }
  }
}

void showStatePower(const uint8_t line) {

  char buffer[21];
  char vTmp[5] = "----";
  char pTmp[4] = "---";

  dtostrf(voltageBattAvg, 4, 1, vTmp);

#if GHETTODSP
  if ( tempC > 0 ) {
    dtostrf(tempC, 3, 0, pTmp);
  }
#endif

#if GHETTODSP && PWR
  if ( stateCharging == 1 ) {
    sprintf(buffer, "Charging: %sV %sC", vTmp, pTmp);
  } else {
    sprintf(buffer, "Battery : %sV %sC", vTmp, pTmp);
  }
#endif

#if HOOPTYDSP
  sprintf(buffer, "Battery : %sV", vTmp);
#endif

  lcd.setCursor(0, line);
  lcdPrintCentered(buffer);
}


void showLinks() {
  #if LCD2004
    const uint8_t line = 3;
  #elif LCD2002
    const uint8_t line = 1;
  #endif
  if ( !whichLinkToShow && deviceConnected1  ) {
    lcd.setCursor(0, line);
    lcd.print(F("Link1:"));
    if ( devicePlaying1 == 1 ) {
      lcd.print((char)2);
    } else {
      lcd.print((char)3);
    }
    lcd.print(phoneName1);
    for ( uint8_t i = strlen(phoneName1); i < 13; i++ ) {
      lcdPrintSpace();
    } 
  #if !HOOPTYDSP
    } else if ( deviceConnected2 ) {
  
      lcd.setCursor(0, line);
      lcd.print(F("Link2:"));
      if ( devicePlaying2 == 1 ) {
        lcd.print((char)2);
      } else {
        lcd.print((char)3);
      }
      lcd.print(phoneName2);
      for ( uint8_t i = strlen(phoneName2); i < 13; i++ ) {
        lcdPrintSpace();
      }
  #endif
  #if GHETTODSP // Two connections allowed
    } else if ( !devicePlaying1 && !devicePlaying2 && ( !deviceConnected1 || !deviceConnected2 )) {
  #else // Only one connection allowed
    } else if ( !deviceConnected1 ) {
  #endif
    if (!deviceConnected1 && settings.sourceMode != 2 && settings.spkMode != 4 ) {
      //lcd.setCursor(0, 2);
      #if LCD2002
        lcd.setCursor(0, 0);
        lcdPrintCentered(F("Ready to Pair"));
      #endif
      lcd.setCursor(0, line);
      lcdPrintCentered(speakerName);
    } else {
      //lcdClearLine(2);
    }
  }
  whichLinkToShow = !whichLinkToShow;
}



char * lcdGetDb(int8_t val, char * buf) {
  if ( val > 0 ) {
    sprintf(buf, "+%i dB", val);
  } else if ( val <= 0 ) {
    sprintf(buf, "%i dB", val);
  }
  return buf;
}

void showSettings() {
  char buf[21];
  #if LCD2004
    lcd.setCursor(0, 1);
  #else
    lcd.setCursor(0, 0);
  #endif
    
  switch ( settingMode ) {
    case 0:
      lcdPrintSpkStatus();
      break;
    case 1:
      switch ( settings.dBassMode ) {
        case 2:
          lcdPrintSetting("D.bass","Low Cut");
          break;
        default:
          lcdPrintSetting(ftc(F("D.bass")),lcdGetOnOff(settings.dBassMode,buf));
      }
      break;
    case 2:
      lcdPrintSetting(ftc(F("Bass")),lcdGetDb(settings.bassLevel,buf));
      break;
    case 3:
      lcdPrintSetting(ftc(F("Treble")),lcdGetDb(settings.trebleLevel,buf));
      break;
    case 4:
      lcdPrintSetting(ftc(F("Low Gain")),lcdGetDb(settings.lowLevel,buf));
      break;
    case 5:
      lcdPrintSetting(ftc(F("Mid Gain")),lcdGetDb(settings.midLevel,buf));
      break;
    case 6:
      lcdPrintSetting(ftc(F("Hi Gain")),lcdGetDb(settings.highLevel,buf));
      break;
      break;
    case 7:
      lcdPrintSetting(ftc(F("Sys Gain")),lcdGetDb(settings.gainLevel,buf));
      break;
    case 8:
      lcdPrintSetting(ftc(F("Spk Mode")),lcdGetSpkMode(buf));
      break;
    case 9:
      lcdPrintSetting(ftc(F("Source")),lcdGetSourceMode(buf));
      break;
    case 10:
      lcdPrintSetting(ftc(F("Channel")),lcdGetChannelMode(buf));
      break;
    #if HOOPTYDSP
      case 11:
        lcdPrintSetting(ftc(F("Fader")),lcdGetFaderLevel(buf));
        break;
      case 12:
        lcdPrintSetting(ftc(F("Crossover")),lcdGetXoverMode(buf));
        break;
      case 13:
        lcdPrintSetting(ftc(F("Subharmonics")),settings.subharmonicLevel);
        break;
      case 14:
        lcdPrintSetting(ftc(F("Subsonic Filter")),lcdGetOnOff(settings.subsonicMode,buf));
        break;
      case 15:
        lcdPrintSetting(ftc(F("Call Volume")),lcdGetDb(settings.callVolume,buf));
        break;
      case 16:
        lcdPrintSetting(ftc(F("Mid")),lcdGetDb(settings.midrangeLevel,buf));
        break;
    #endif
  }
}

void lcdPrintSpkStatus() {
  if ( settings.spkMode > 0 ) {
    char buffer[21];
    char buff[11];
    switch ( NSPK_STATE ) {
      case 0:
        sprintf(buffer, "%s %s", lcdGetSpkMode(buff), "(No Spk)");
        break;
      case 2:
        sprintf(buffer, "%s %s", lcdGetSpkMode(buff), "(Scan)");
        break;
      case 3:
        sprintf(buffer, "%s %s (%d)", lcdGetSpkMode(buff), "Linked", NSPK_GROUP);
        break;
      default:
        sprintf(buffer, "%s %s", lcdGetSpkMode(buff), "(Busy)");
    }
    lcdPrintCentered(buffer);
  } else {
  #if GHETTODSP
      lcdPrintCentered(F("Single Speaker Mode"));
  #elif HOOPTYDSP
      lcdPrintCentered(F(" "));
  #endif
  }
}

const char * lcdGetSpkMode(char * buf) {
  switch ( settings.spkMode ) {
    case 0:
      strcpy(buf,"Single");
      break;
    case 1:
      strcpy(buf,"Master");
      break;
    case 2:
      strcpy(buf,"Slave");
      break;
    case 3:
      strcpy(buf,"ConcertM");
      break;
    case 4:
      strcpy(buf,"ConcertS");
      break;
  }
  return buf;
}

char * lcdGetOnOff(int8_t &onOff,char * buf) {
  if ( onOff ) {
    strcpy(buf,"On");
  } else {
    strcpy(buf,"Off");
  }
  return buf;
}

char * lcdGetSourceMode(char * buf) {
  switch ( settings.sourceMode ) {
    case 0:
      strcpy(buf,"BT In");
      break;
    case 1:
      strcpy(buf,"Aux In");
      break;
  }
  return buf;
}

char * lcdGetChannelMode(char * buf) {
  switch ( settings.channelMode ) {
    case 0:
      strcpy(buf,"Mono");
      break;
    case 1:
      strcpy(buf,"Left");
      break;
    case 2:
      strcpy(buf,"Right");
      break;
  }
  return buf;
}

#if HOOPTYDSP
char * lcdGetXoverMode(char * buf) {
  switch ( settings.xOverMode ) {
    case 0:
      strcpy(buf,"Off");
      return buf;
    case 1:
      strcpy(buf,"60");
      break;
    case 2:
      strcpy(buf,"70");
      break;
    case 3:
      strcpy(buf,"80");
      break;
    case 4:
      strcpy(buf,"90");
      break;
    case 5:
      strcpy(buf,"100");
      break;
    case 6:
      strcpy(buf,"110");
      break;
    case 7:
      strcpy(buf,"120");
      break;
  }
  strcat(buf," Hz");
  return buf;
}
#endif

#if HOOPTYDSP
char * lcdGetFaderLevel(char * buf) {
  if ( settings.faderLevel == 0 ) {
    strcpy(buf,"Center");
  } else {
    if ( settings.faderLevel > 0 ) {
      sprintf(buf, "Front +%d", abs(settings.faderLevel));
    } else {
      sprintf(buf, "Rear +%d", abs(settings.faderLevel));
    }
  }
  return buf;
}
#endif

void showPowerOff() {
  lcdClearLine(1);
  #if LCD2004
    lcd.setCursor(0, 2);
  #elif LCD2002
    lcd.setCursor(0, 1);
  #endif
  if ( stateBattery == 0 ) {
    lcdPrintCentered(F("Critical Low Battery"));
  } else if ( stateBattery == 1 ) {
    lcdPrintCentered(F("Low Battery"));
  } else {
    lcdPrintCentered(F("Audio Off"));
  }
  #if LCD2004
    lcdClearLine(3);
  #endif
}
#endif
