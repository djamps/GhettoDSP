#if GHETTODSP
void setDbassMode(uint8_t mode) {
  switch ( mode ) {
    case 0:
      dsp.dcSource(MOD_DBASS_MONOSWSLEW_ADDR, uint32_t(0)); // Disable dBass
      dsp.volume(MOD_SUBLOWMIX_ALG0_STAGE0_VOLONE_ADDR, 0); // Restore sub bass
      break;
    case 1:
      dsp.dcSource(MOD_DBASS_MONOSWSLEW_ADDR, uint32_t(1)); // Enable dBass
      dsp.volume(MOD_SUBLOWMIX_ALG0_STAGE0_VOLONE_ADDR, 0); // Restore sub bass
      break;
    case 2:
      #if LOWCUT
        dsp.dcSource(MOD_DBASS_MONOSWSLEW_ADDR, uint32_t(0)); // Disable dBass
        dsp.volume(MOD_SUBLOWMIX_ALG0_STAGE0_VOLONE_ADDR, -100); // Cut sub bass
      #endif
      break;
  }
}
#endif

#if HOOPTYDSP
void setCallVolume() {
  // Set volume override level
  dsp.volume(MOD_VOLUMELEVEL_DCINPALG4_ADDR, settings.callVolume);
  if ( callActive ) {
    // Override volume
    dsp.dcSource(MOD_VOLUMEOVERRIDE_MONOSWSLEW_ADDR, uint32_t(1));
  } else {
    // Do not override volume
    dsp.dcSource(MOD_VOLUMEOVERRIDE_MONOSWSLEW_ADDR, uint32_t(0));
  }
}

void setXover() {
  switch ( settings.xOverMode ) {
    case 0: // Off
      dsp.dcSource(MOD_LOWPASSONOFF_MONOSWSLEW_ADDR, uint32_t(0));
      dsp.dcSource(MOD_HIGHPASSONOFF1_MONOSWSLEW_ADDR, uint32_t(0));
      dsp.dcSource(MOD_HIGHPASSONOFF2_MONOSWSLEW_ADDR, uint32_t(0));
      break;
    default: // On
      dsp.dcSource(MOD_XOVERFREQ_DCINPALG3_ADDR, uint32_t(settings.xOverMode - 1));
      dsp.dcSource(MOD_LOWPASSONOFF_MONOSWSLEW_ADDR, uint32_t(1));
      dsp.dcSource(MOD_HIGHPASSONOFF1_MONOSWSLEW_ADDR, uint32_t(1));
      dsp.dcSource(MOD_HIGHPASSONOFF2_MONOSWSLEW_ADDR, uint32_t(1));
      break;
  }
}
#endif

#if HOOPTYDSP
void setFader() {
  int f, r;
  f = r = 0;
  if ( settings.faderLevel == 10 ) {
    r = -90;
  } else if ( settings.faderLevel == -10 ) {
    f = -90;
  } else if ( settings.faderLevel > 0 ) {
    r = settings.faderLevel * -3;
  } else if ( settings.faderLevel < 0 ) {
    f = settings.faderLevel * 3;
  }
  dsp.volume(MOD_FADERLEFT_ALG0_STAGE0_MULTICTRLSPLITTER11_ADDR, f);
  dsp.volume(MOD_FADERLEFT_ALG0_STAGE1_MULTICTRLSPLITTER12_ADDR, r);
  dsp.volume(MOD_FADERRIGHT_ALG0_STAGE0_MULTICTRLSPLITTER21_ADDR, f);
  dsp.volume(MOD_FADERRIGHT_ALG0_STAGE1_MULTICTRLSPLITTER22_ADDR, r);
}
#endif

void setLoudnessMode(bool enabled) {
  if ( enabled == true ) {
    dsp.safeload_writeRegister(MOD_LOUDNESSLHEXT1_ALG0_LEVEL0_ADDR, 0x00E8F5C3, true);
    dsp.safeload_writeRegister(MOD_LOUDNESSLHEXT1_ALG0_LEVEL1_ADDR, 0x00E8F5C3, true);
  } else {
    dsp.safeload_writeRegister(MOD_LOUDNESSLHEXT1_ALG0_LEVEL0_ADDR, 0x00400000, true);
    dsp.safeload_writeRegister(MOD_LOUDNESSLHEXT1_ALG0_LEVEL1_ADDR, 0x00400000, true);

  }
}

void setCorrectionsMode(bool enabled) {
  if ( enabled == true ) {
    dsp.dcSource(MOD_FILTERBYPASS_MONOSWSLEW_ADDR, uint32_t(1));
  } else {
    dsp.dcSource(MOD_FILTERBYPASS_MONOSWSLEW_ADDR, uint32_t(0));
  }
}

void setSysGain() {
  #if GHETTODSP
    switch ( settings.channelMode ) {
      case 0: // Mono
        dsp.volume(MOD_STEREOCOMBINE_ALG0_STAGE0_VOLONE_ADDR, settings.gainLevel);
        dsp.volume(MOD_STEREOCOMBINE_ALG0_STAGE0_VOLTWO_ADDR, settings.gainLevel);
        break;
      case 1: // Left
        dsp.volume(MOD_STEREOCOMBINE_ALG0_STAGE0_VOLONE_ADDR, settings.gainLevel + 3);
        dsp.volume(MOD_STEREOCOMBINE_ALG0_STAGE0_VOLTWO_ADDR, -90);
        break;
      case 2: // Right
        dsp.volume(MOD_STEREOCOMBINE_ALG0_STAGE0_VOLONE_ADDR, -90);
        dsp.volume(MOD_STEREOCOMBINE_ALG0_STAGE0_VOLTWO_ADDR, settings.gainLevel + 3);
        break;
    }
  #endif

  #if HOOPTYDSP
    dsp.volume(MOD_SYSGAIN_ALG0_TARGET_ADDR, settings.gainLevel);
    setCallVolume();
  #endif
}

#if HOOPTYDSP
void setSubsonic() {
  dsp.dcSource(MOD_SUBSONICONOFF_MONOSWSLEW_ADDR, uint32_t(settings.subsonicMode));
}
#endif

#if HOOPTYDSP
void setSubharmonic() {
  switch ( settings.subharmonicLevel ) {
    case 0: // Off
      dsp.volume(MOD_SUBHARMONICMIXER_ALG0_STAGE0_VOLONE_ADDR, -90); // Subharmonics Off
      dsp.volume(MOD_SUBHARMONICMIXER_ALG0_STAGE0_VOLTWO_ADDR, 0); // Dry On
      break;
    default: // -20 thru 0db
      int subLevel = -20 + (settings.subharmonicLevel * 2) + 6;
      int dryLevel = 0 - (settings.subharmonicLevel * 2);
      dsp.volume(MOD_SUBHARMONICMIXER_ALG0_STAGE0_VOLONE_ADDR, subLevel);
      dsp.volume(MOD_SUBHARMONICMIXER_ALG0_STAGE0_VOLTWO_ADDR, dryLevel);
      break;
  }
}
#endif

void setEqMode() {
  memcpy(&eq, &settings, sizeof(Settings));
  switch(settings.eqMode) {
    case 0: // Defeat all DSP
      eq.loudnessMode = 0;
      eq.correctionsMode = 0;
      eq.bassLevel = 0;
      eq.midrangeLevel = 0;
      eq.trebleLevel = 0;
      eq.dBassMode = 0;
      eq.lowLevel = 0;
      eq.midLevel = 0;
      eq.highLevel = 0;
      break;
    case 1: // Accuracy - only IIR corrections
      eq.loudnessMode = 0;
      eq.correctionsMode = 1;
      eq.bassLevel = 0;
      eq.midrangeLevel = 0;
      eq.trebleLevel = 0;
      eq.dBassMode = 0;
      eq.lowLevel = 0;
      eq.midLevel = 0;
      eq.highLevel = 0;
      break;
    case 2: // Bright
      eq.loudnessMode = 0;
      eq.correctionsMode = 1;
      eq.bassLevel = 0;
      eq.midrangeLevel = 0;
      eq.trebleLevel = 8;
      eq.dBassMode = 0;
      eq.lowLevel = 0;
      eq.midLevel = 0;
      eq.highLevel = 0;
      break;
    case 3: // Deep
      eq.loudnessMode = 0;
      eq.correctionsMode = 1;
      eq.bassLevel = 6;
      eq.midrangeLevel = 0;
      eq.trebleLevel = 3;
      eq.dBassMode = 0;
      eq.lowLevel = 0;
      eq.midLevel = 0;
      eq.highLevel = 0;
      break;
    case 4: // Punch
      eq.loudnessMode = 0;
      eq.correctionsMode = 1;
      eq.bassLevel = 6;
      eq.midrangeLevel = 0;
      eq.trebleLevel = 3;
      eq.dBassMode = 1;
      eq.lowLevel = 0;
      eq.midLevel = 0;
      eq.highLevel = 0;
      break;
    case 5: // Power
      eq.loudnessMode = 1;
      eq.correctionsMode = 0;
      eq.bassLevel = 6;
      eq.midrangeLevel = 0;
      eq.trebleLevel = 10;
      eq.dBassMode = 1;
      eq.lowLevel = 0;
      eq.midLevel = 0;
      eq.highLevel = 0;
      break;
    case 6: // Night
      eq.loudnessMode = 0;
      eq.correctionsMode = 1;
      eq.bassLevel = 0;
      eq.midrangeLevel = 0;
      eq.trebleLevel = 0;
      eq.dBassMode = 2;
      eq.lowLevel = 0;
      eq.midLevel = 0;
      eq.highLevel = 0;
      break;
    case 7: // Custom
      break;
  }
}

void setDspParams() {
  Settings tmp;

  #if EQ
    setEqMode();
  #endif
  
  if ( settings.eqMode == 7 ) {
    memcpy(&tmp, &settings, sizeof(Settings));
  } else {
    memcpy(&tmp, &eq, sizeof(Settings));
  }
  
  // Corrections and loudness
  setCorrectionsMode(tmp.correctionsMode);
  setLoudnessMode(tmp.loudnessMode);
  
  // Bass and treble
  dsp.dcSource(MOD_BASSADJ_DCINPALG1_ADDR, uint32_t(tmp.bassLevel + 10));
  dsp.dcSource(MOD_TREBLEADJ_DCINPALG2_ADDR, uint32_t(tmp.trebleLevel + 10));
  
  #if HOOPTYDSP
    dsp.dcSource(MOD_MIDADJ_DCINPALG5_ADDR, uint32_t(tmp.midrangeLevel + 10));
  #endif
  
  setSysGain();

  #if GHETTODSP
    setDbassMode(tmp.dBassMode);
    dsp.volume(MOD_LOWLEVEL_GAIN1940ALGNS1_ADDR, tmp.lowLevel);
    #if !TWOWAY
      dsp.volume(MOD_MIDLEVEL_GAIN1940ALGNS2_ADDR, tmp.midLevel);
    #endif
    dsp.volume(MOD_HIGHLEVEL_GAIN1940ALGNS3_ADDR, tmp.highLevel);
  #endif

  #if HOOPTYDSP
    setFader();
    setXover();
    setSubsonic();
    setSubharmonic();
  #endif
}

void stopAudio() {
  // Mute external DAC
  digitalWrite(SOFT_MUTE, LOW);
  delay(100);
  
  // Stop the DSP
  digitalWrite(DSP_RESET, LOW);
  
  // Shutdown the BM64
  #if BT
    if ( settings.spkMode > 0 ) {
      bm64.mmiAction(BM64_MMI_TERMINATE_CANCEL_MSPK_CONNECTION);
    }
    delay(100);
    bm64.powerOff();
    bm64PwrState = 0;
  #endif
}


void startAudio() {
  delay(100); // freeze test
  
  #if LCD2004
    lcdClearLine(1);
    lcdClearLine(2);
    lcdClearLine(3);
  #endif

  #if LCD2002
    lcdClearLine(1);
  #endif

  #if PWR
    // Temporarily disable watchdog timer in PWR module in case we need to program the DSP
    disableWatchDog();
    delay(100);
  #endif

  #if BT // Start the BM64
    #if DEBUG7
      Serial.println(F("Start BM64"));
    #endif

    #if LCD2004 || LCD2002
      lcd.setCursor(0, 1);
      lcdPrintCentered(F("Starting Bluetooth"));
    #endif
    delay(500);
    setBTName();
    delay(500);
    bm64.powerOn();
    bm64PwrState = 1;
    delay(500);
  #endif

  #if LCD2004 || LCD2002
    lcd.setCursor(0, 1);
    lcdPrintCentered(F("Updating DSP"));
  #endif

  #if DEBUG7
    Serial.println(F("Starting EEPROM update"));
  #endif

  ee.writeFirmware(DSP_eeprom_firmware, sizeof(DSP_eeprom_firmware), DSP_VERSION);

  #if DEBUG7
    Serial.println(F("Finished EEPROM update"));
  #endif

  // Start the DSP
  #if DEBUG7
    Serial.println(F("Loading DSP"));
  #endif

  #if LCD2004 || LCD2002
    lcd.setCursor(0, 1);
    lcdPrintCentered(F("Loading DSP"));
  #endif

  // Set write protect and self boot lines high
  #if HASEEP
  pinMode(DSP_WP, INPUT);
  digitalWrite(DSP_SELFBOOT, HIGH);
  #else
  //pinMode(DSP_WP, INPUT);
  digitalWrite(DSP_SELFBOOT, LOW);
  #endif

  // Reset the DSP chip
  pinMode(DSP_RESET, OUTPUT);
  digitalWrite(DSP_RESET, LOW);
  delay(100);
  pinMode(DSP_RESET, INPUT);

  #if HASEEP
  // Wait for DSP to finish self boot
  // this avoids I2C collisions
  delay(DSP_WAIT);
  #else
  loadProgram(dsp);
  #endif

  // Audio DSP settings
  #if DEBUG7
    Serial.println(F("Configure DSP"));
  #endif
  setDspParams();
  
  // Unmute external DAC
  digitalWrite(SOFT_MUTE, HIGH);

  // Tell PWR to turn on power to the amp
  #if PWR
    audioDetected = 1;
    sendDataToPWR();
  #endif

  // Tell the BM64 we have a shitty 64 byte buffer
  #if BT
    #if DEBUG
      Serial.println(F("BM64 UART"));
    #endif
    bm64.setUart64();
  #endif

  // Set spk/nspk mode
  currentSpkMode = settings.spkMode;
  #if DEBUG7
    Serial.println(F("Set SPK mode"));
  #endif
  #if BT
    if ( settings.spkMode > 0 ) {
      setSpkMode();
    }
  #endif

  // Set line in mode
  #if DEBUG7
    Serial.println(F("Set AUX mode"));
  #endif
  setSourceMode();

  // Show settings
  #if LCD2004 || LCD2002
    #if DEBUG7
      Serial.println(F("Show settings"));
    #endif
    lcdClearLine(0);
    showSettings();
  #endif

  // Turn watchdog timer in PWR module back on
  #if PWR
    enableWatchDog();
  #endif

}


void setSourceMode() {

  #if LINEINDSP
    #if !LINEINBT // Pass line-in ADC's directly through DSP
      switch ( settings.sourceMode ) {
        case 0: // BT In
          if ( bm64PwrState == 0 ) {
            bm64.powerOn(); // Enable bluetooth
            bm64PwrState = 1;
          }
          dsp.dcSource(MOD_INPUTSELECTL_MONOSWSLEW_ADDR, uint32_t(0));
          dsp.dcSource(MOD_INPUTSELECTR_MONOSWSLEW_ADDR, uint32_t(0));
          break;
        case 1: // Aux In
          bm64.powerOff(); // Disable bluetooth
          bm64PwrState = 0;
          dsp.dcSource(MOD_INPUTSELECTL_MONOSWSLEW_ADDR, uint32_t(1));
          dsp.dcSource(MOD_INPUTSELECTR_MONOSWSLEW_ADDR, uint32_t(1));
          break;
      }
    #else // Pass line-in ADC to BM64
      // Do nothing, because we're always doing that
    #endif
  #endif

  #if LINEINBT
    setSourceModeBT();
  #endif
}


void checkForAudio() {
  if ( DEBUGX ) {Serial.println(F("Checking audio"));};
  if ( dsp.readBack(MOD_READBACK1_ALG0_VAL0_ADDR, MOD_READBACK1_ALG0_VAL0_VALUES, 3) == 0 ) {
    if ( audioDetected != 1 ) {
      if ( DEBUGX ) {
        Serial.println(F("DSP Detects audio"));
      }
      audioDetected = 1;
      #if PWR
        sendDataToPWR();
      #endif
    }
  } else {
    if ( audioDetected != 0 ) {
      if ( DEBUGX ) {
        Serial.println(F("DSP Detects silence"));
      }
      audioDetected = 0;
      #if PWR
        sendDataToPWR();
      #endif
    }
  }
}
