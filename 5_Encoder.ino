#if ENCODER

uint8_t longButtonPush = 0;
bool aLast;
int8_t bValCount = 0;
unsigned long lastDebounce = 0;
unsigned long lastButtonPush = 0;
uint8_t encoderDebounceDelay = 3;
bool encoderSwLast = HIGH;

void setupEncoder() {
  pinMode(ENCODER_SW, INPUT_PULLUP);
  aLast = digitalRead(ENCODER_DT);
}

bool buttonState() {
  if (digitalRead(ENCODER_SW) == LOW) {
    return true;
  } else {
    return false;
  }
}

void handleEncoder() {
  // If enough time has passed check the rotary encoder
  if ((millis() - lastDebounce) > encoderDebounceDelay) {
    if ( settingMode > 0 ) { checkRotary(); }  // Check for rotation only in a settings mode

    lastDebounce = millis(); // Set variable to current millis() timer

    // Check for button down event by comparing value to last
    // Change encoder/display mode for adjustmenting
    if (digitalRead(ENCODER_SW) == LOW && encoderSwLast == HIGH) {
      lastButtonPush = millis();
      #if DEBUG4
        Serial.println(F("Button down"));
      #endif
    } else if (digitalRead(ENCODER_SW) == LOW && encoderSwLast == LOW) { // Button still down
      // Check if button held for 2 seconds
      #if BT
      if ( millis() - lastButtonPush >= 2000 && !longButtonPush ) {
        // Activate page scan when in master/slave mode
        if ( settings.spkMode > 0 ) {
          bm64StartPageScan();
        } else {
          #if HOOPTYDSP
          voiceDial();
          #else
          // skip track in single mode
          bm64.musicControl(MUSIC_CONTROL_NEXT);
          #endif
        }
        longButtonPush = 1;
        lastButtonPush = millis();
      }
      #endif
      // Button up event
    } else if (digitalRead(ENCODER_SW) == HIGH && encoderSwLast == LOW && (millis() - lastButtonPush < 2000)) {
      #if DEBUG4
        Serial.println(F("Button up"));
      #endif
      if ( !longButtonPush ) {
        if ( !callActive) {
          advSettingMode();
        } else {
          #if BT && HOOPTYDSP
            if ( callActive ) {
              if ( callInProgress ) {
                endCall();
              }  else {
                acceptCall();
              }
            }
          #endif
        }
        #if LCD2002 || LCD2004
          showSettings();
        #endif
        lastSettingChange = millis();
      } else {
        longButtonPush = 0;
      }
      lastButtonPush = millis();
    }
    encoderSwLast = digitalRead(ENCODER_SW);
  }
}

void checkRotary() {
  bool aVal = digitalRead(ENCODER_DT);
  bool bVal = digitalRead(ENCODER_CLK);
  if ((aVal != aLast) && (aVal == LOW)) { // Means the knob is rotating
    #if DEBUG4
      Serial.println(F("Rotate"));
    #endif
    if (bVal == LOW) {
      if ( bValCount < 0 ) { bValCount = 0; };
      bValCount++;
      if ( bValCount > 1 ) { // Two rotation "clicks" required per increment
        adjLevel(1); // If bVal is Low, too, CW  ++
        bValCount = 0;
      }
    }
    else {
      if ( bValCount > 0 ) { bValCount = 0; };
      bValCount--;
      if ( bValCount < -1 ) {
        adjLevel(-1); // else, CCW --
        bValCount = 0;
      }
    }
  }
  aLast = aVal;    //Don't forget to reset aLast
}

#endif
