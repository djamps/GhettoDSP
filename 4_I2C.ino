#if PWR

void disableWatchDog() {
  sendDataToPWR(true);
}

void enableWatchDog() {
  sendDataToPWR(false);
}

void sendDataToPWR() {
  sendDataToPWR(false);
}

void sendDataToPWR(bool disableWatchDog) {
  if ( DEBUG3 ) {
    Serial.println(F("Sending data to PWR"));
  }
  struct
  {
    uint8_t audioDetected;
    float voltageBattMaxCharge;
    float vComp;
    bool disableWatchDog;
  } response;
  response.audioDetected = audioDetected;
  response.voltageBattMaxCharge = VOLTAGEBATTMAXCHARGE;
  response.vComp = VCOMP;
  response.disableWatchDog = disableWatchDog;
  Wire.beginTransmission(PWR_I2C_ADDRESS);
  Wire.write ((byte *) &response, sizeof response);
  Wire.endTransmission();
  delay(100);
}

void getDataFromPWR() {
  uint8_t currentPowerState = statePower;

#if DEBUG3
  Serial.print(F("Asking PWR for data..."));
#endif

  Wire.requestFrom(PWR_I2C_ADDRESS, 21);

  if ( Wire.available() ) {
    I2C_readAnything(voltageBattAvg);
    //I2C_readAnything(voltageBuck);
    //I2C_readAnything(voltageBoost);
    //I2C_readAnything(voltageCharge);
    I2C_readAnything(tempC);
    I2C_readAnything(statePower);
    I2C_readAnything(stateCharging);
    I2C_readAnything(stateSys);
    I2C_readAnything(percentBattery);
    I2C_readAnything(stateBattery);

    if ( DEBUG3 ) {
      Serial.print(F("Got data"));
    }

#if LCD2002 || LCD2004
    // Don't override start banner until at least 5 seconds
    if ( millis() > 5000 ) {
      showStatePower(0);
    }
#endif

    // Power state changed, handle accordingly
    if ( currentPowerState != statePower ) {
      if ( statePower == 1 ) {
#if DEBUG3
        Serial.println(F("*****Power state change to ON*****"));
#endif
        startAudio();
      } else {
#if DEBUG3
        Serial.println(F("*****Power state change to OFF*****"));
#endif
        stopAudio();
#if LCD2002 || LCD2004
        showPowerOff();
#endif
      }
    }
#if LCD2002 || LCD2004
    if ( stateCharging == 1 && statePower == 0 ) {
      showPowerOff();
    }
#endif
  } else {
#if DEBUG3
    Serial.println(F("No response from PWR!"));
#endif

#if LCD2002 || LCD2004
    lcd.setCursor(0, 0);
    lcdPrintCentered(F("Comm Failure"));
#endif
  }

}
#endif
