bool audioDetected = 0; // DSP audio output detect // default to on during startup

#if GHETTODSP
void dBassModeSet() {
  switch ( settings.dBassMode ) {
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
        dsp.volume(MOD_SUBLOWMIX_ALG0_STAGE0_VOLONE_ADDR, -90); // Cut sub bass
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

void setDspParams() {
  // Bass and treble
  dsp.dcSource(MOD_BASSADJ_DCINPALG1_ADDR, uint32_t(settings.bassLevel + 10));
  dsp.dcSource(MOD_TREBLEADJ_DCINPALG2_ADDR, uint32_t(settings.trebleLevel + 10));
  
  #if HOOPTYDSP
    dsp.dcSource(MOD_MIDADJ_DCINPALG5_ADDR, uint32_t(settings.midrangeLevel + 10));
  #endif
  
  setSysGain();

  #if GHETTODSP
    dBassModeSet();
    dsp.volume(MOD_LOWLEVEL_GAIN1940ALGNS1_ADDR, settings.lowLevel);
    #if !TWOWAY
      dsp.volume(MOD_MIDLEVEL_GAIN1940ALGNS2_ADDR, settings.midLevel);
    #endif
    dsp.volume(MOD_HIGHLEVEL_GAIN1940ALGNS3_ADDR, settings.highLevel);
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
  #endif
}


void startAudio() {
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
  pinMode(DSP_WP, INPUT);
  //pinMode(DSP_SELFBOOT, INPUT); // self boot is already pulled high

  // Reset the DSP chip
  pinMode(DSP_RESET, OUTPUT);
  digitalWrite(DSP_RESET, LOW);
  delay(100);
  pinMode(DSP_RESET, INPUT);

  // Wait for DSP to finish self boot
  // this avoids I2C collisions
  delay(1000);

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

  // Audio DSP settings
  currentSpkMode = settings.spkMode;
  #if DEBUG7
    Serial.println(F("Configure DSP"));
  #endif
  setDspParams();

  // Set spk/nspk mode
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
  #if BT
    setSourceMode();
  #endif

  // Show settings
  #if LCD2004 || LCD2002
    #if DEBUG7
      Serial.println(F("Show settings"));
    #endif
    showSettings();
  #endif

  // Unmute external DAC
  digitalWrite(SOFT_MUTE, HIGH);

  // Turn watchdog timer in PWR module back on
  #if PWR
    enableWatchDog();
  #endif

}

#if LINEINDSP
void setSourceMode() {
  switch ( settings.sourceMode ) {
    case 0:
      dsp.dcSource(MOD_AUXL_MONOSWSLEW_ADDR, uint32_t(0));
      dsp.dcSource(MOD_AUXR_MONOSWSLEW_ADDR, uint32_t(0));
      break;
    case 1:
      dsp.dcSource(MOD_AUXL_MONOSWSLEW_ADDR, uint32_t(1));
      dsp.dcSource(MOD_AUXR_MONOSWSLEW_ADDR, uint32_t(1));
      break;
  }
}
#endif

void checkForAudio() {
  if ( dsp.readBack(MOD_READBACK1_ALG0_VAL0_ADDR, MOD_READBACK1_ALG0_VAL0_VALUES, 3) == 0 ) {
    if ( audioDetected != 1 ) {
      if ( DEBUG ) {
        Serial.println(F("DSP Detects audio"));
      }
      audioDetected = 1;
      #if PWR
        sendDataToPWR();
      #endif
    }
  } else {
    if ( audioDetected != 0 ) {
      if ( DEBUG ) {
        Serial.println(F("DSP Detects silence"));
      }
      audioDetected = 0;
      #if PWR
        sendDataToPWR();
      #endif
    }
  }
}
