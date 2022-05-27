#if HOOPTYDSP
unsigned long lastKeyOff = 0;
unsigned long lastBrownout = 0;

void readVoltages() {

  voltageBatt = analogRead(VACC) / VCOMP;
  #if DEBUG2
    Serial.print("Voltage:");
    Serial.println(voltageBatt);
  #endif
  if ( voltageBattAvg == 0 )
  {
    voltageBattAvg = voltageBatt;
  }
  // Calculate average (only if key has power)
  if ( !lastKeyOff ) {
    if ( millis() - 5000 > voltageBattAvgLast && voltageBattAvgCount > 2 )
    {
      voltageBattAvg = voltageBattAvgSum / voltageBattAvgCount;
      voltageBattAvgSum = 0;
      voltageBattAvgCount = 0;
      voltageBattAvgLast = millis();
    } else {
      voltageBattAvgSum = voltageBattAvgSum + voltageBatt;
      voltageBattAvgCount++;
    }
  }

  if ( statePower == 1 ) {
    if ( voltageBatt < 1 ) { // Key off or engine start, ovveride low battery
      stateBattery = 2;
      lastBrownout = 0;
      // Wait 5 seconds before actual poweroff
      if ( lastKeyOff == 0 ) {
        lastKeyOff = millis();
        #if DEBUG2
          Serial.println(F("Keyoff detected"));
        #endif
      }
      if ( millis() - 5000 >= lastKeyOff ) {
      #if DEBUG2
        Serial.println(F("Shutdown: Key off"));
      #endif
        doPowerOff();
      }
    } else if ( voltageBatt >= BROWNOUTVOLTAGE ) { // Voltage restored
      stateBattery = 2;
      lastKeyOff = 0;
      lastBrownout = 0;
    } else if ( voltageBatt < BROWNOUTVOLTAGE ) { // Voltage between 1 and BROWNOUTVOLTAGE
      stateBattery = 0;
      // Wait 10 seconds before critical poweroff
      if ( lastBrownout == 0 ) {
        lastBrownout = millis();
        #if DEBUG2
          Serial.println(F("Brownout detected"));
        #endif
      }
      if ( millis() - 10000 >= lastBrownout ) {
        doPowerOff();
      }
    }
  }
}

void checkPowerButton() {

  // Is button pushed?
  if ( digitalRead(BUTTON) == LOW ) {
    // Wait until release
    while (digitalRead(BUTTON) == LOW);
    //debounce
    delay(10);
    if ( statePower == 1 ) {
      #if DEBUG2
        Serial.println(F("Shutdown: Button push"));
      #endif
      doPowerOff();
      settings.powerState = 0; // Set power state
      writeEEPROM();
    } else {
      #if DEBUG2
        Serial.println(F("Startup: Button push"));
      #endif
      doPowerOn();
    }
  }
}

void initPowerState() {
  if ( settings.powerState == 1 ) { // Previous power state was on
    doPowerOn(); // Power it on
  } else {
    #if LCD2002 || LCD2004
      showPowerOff(); // Show standby screen
    #endif
  }
}

void doPowerOn() {
  if ( settings.powerState == 0 ) {
    settings.powerState = 1;
    lastSettingChange = millis();
  }
  digitalWrite(PWRON, HIGH);
  pinMode(PWRON, OUTPUT); // Hold power on
  startAudio(); // Start DSP
  digitalWrite(REMOTE, HIGH); // Turn on amplifier(s)
  statePower = 1;
  needToLinkBack = 1; // Link back bluetooth
}

void doPowerOff() {
  #if LCD2002 || LCD2004
    showPowerOff();
  #endif
  digitalWrite(REMOTE, LOW); // Shut off amplifier(s)
  stopAudio(); // Stop DSP
  pinMode(PWRON, INPUT); // Let vehicle control power state
  statePower = 0;
}
#endif
