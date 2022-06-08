#if BT

#if HOOPTYDSP
// Accept a call
void acceptCall() {
  bm64.mmiAction(BM64_MMI_ACCEPT_CALL);
}
#endif

#if HOOPTYDSP
// Hang up a call
void endCall() {
  bm64.mmiAction(BM64_MMI_FORCE_END_CALL);
}
#endif

#if HOOPTYDSP
void voiceDial() {
  bm64.mmiAction(BM64_MMI_VOICE_DIAL);
}
#endif

//Check UART Event
void checkUartEvent() {
  bm64.run();
  #if DEBUG
    if (Serial.available() > 0)
    {
      // read the incoming byte:
      char c = Serial.read();
      switch (c)
      {
        case '2':
          // Other Action, Please see BM64_Debug.h
          bm64.bleEnable();
          break;
        case 'd':
          bm64.generateTone(0x07);
          break;
      }
    }
  #endif
}

void bm64StartPageScan() {
  #if DEBUG5
    Serial.println(F("Begin page scan"));
  #endif
  switch ( settings.spkMode ) {
    case 1:
      bm64MasterPage();
      break;
    case 2:
      bm64SlavePage();
      break;
    case 3:
      bm64MasterBroadcastPage();
      break;
    case 4:
      bm64SlaveBroadcastPage();
      break;
  }
}

void bm64MasterPage() {
  //bm64.mmiAction(BM64_MMI_NSPK_ENTER_NSPK_MODE);
  //if ( deviceConnected1 || deviceConnected2 ) { needToLinkBack = 1; }
  bm64.mmiAction(BM64_MMI_MASTERSPK_ENTER_CSB_PAGE);
}

void bm64SlavePage() {
  //bm64.mmiAction(BM64_MMI_NSPK_ENTER_NSPK_MODE);
  bm64.mmiAction(BM64_MMI_SLAVESPK_ENTER_CSB_PAGESCAN);
}

void bm64MasterBroadcastPage() {
  bm64.mmiAction(BM64_MMI_NSPK_ADD_THIRD_SPK);
}

void bm64SlaveBroadcastPage() {
  //bm64.mmiAction(BM64_MMI_NSPK_ENTER_BROADCAST_MODE);
  bm64.mmiAction(BM64_MMI_SLAVESPK_ENTER_CSB_PAGESCAN);
}

#if LINEINBT
void setSourceMode() {
  if ( BT ) {
    if ( settings.sourceMode == 1 ) {
      bm64.lineInEnable();
      bm64.mmiAction(BM64_MMI_EXIT_PAIRING_MODE);
    } else {
      bm64.lineInDisable();
      bm64.mmiAction(BM64_MMI_ANY_MODE_ENTERING_PAIRING);
    }
  }
}
#endif


void setSpkMode() {
  switch ( settings.spkMode ) {
    case 0: // Single speaker mode
      bm64.mmiAction(BM64_MMI_TERMINATE_CANCEL_MSPK_CONNECTION);
      break;
    case 1: // Master mode
      bm64.mmiAction(BM64_MMI_NSPK_ENTER_NSPK_MODE);
      bm64.mmiAction(BM64_MMI_MASTERSPK_ENTER_CSB_PAGE);
      bm64.mmiAction(BM64_MMI_ANY_MODE_ENTERING_PAIRING);
      break;
    case 2: // Slave mode
      bm64.mmiAction(BM64_MMI_DISCONNECT_A2DP);
      bm64.mmiAction(BM64_MMI_EXIT_PAIRING_MODE);
      bm64.mmiAction(BM64_MMI_NSPK_ENTER_NSPK_MODE);
      bm64.mmiAction(BM64_MMI_SLAVESPK_ENTER_CSB_PAGESCAN);
      break;
    case 3: // Concert master
      bm64.mmiAction(BM64_MMI_NSPK_ENTER_BROADCAST_MODE);
      bm64.mmiAction(BM64_MMI_MASTERSPK_ENTER_CSB_PAGE);
      bm64.mmiAction(BM64_MMI_ANY_MODE_ENTERING_PAIRING);
      break;
    case 4: // Concert slave
      bm64.mmiAction(BM64_MMI_DISCONNECT_A2DP);
      bm64.mmiAction(BM64_MMI_EXIT_PAIRING_MODE);
      bm64.mmiAction(BM64_MMI_NSPK_ENTER_BROADCAST_MODE);
      bm64.mmiAction(BM64_MMI_SLAVESPK_ENTER_CSB_PAGESCAN);
      break;
  }
}


void setBTName() {
  char buffer[29];
  if ( settings.spkMode == 1 || settings.spkMode == 3 ) {
    sprintf(buffer, "%s%s", SPEAKER_NAME, "(Master)");
  } else if ( settings.spkMode == 2 || settings.spkMode == 4 ) {
    sprintf(buffer, "%s%s", SPEAKER_NAME, "(Slave)");
  } else {
    sprintf(buffer, "%s", SPEAKER_NAME);
  }
  bm64.setName(buffer);
  bm64.setName(buffer);
}



void getMetaData() {
  if ( devicePlaying1 ) {
    bm64.getSongName(0);
  } else if ( devicePlaying2 ) {
    bm64.getSongName(1);
  }
}

void resetBT() {
  // Set BT reset line low, then high impedance
  pinMode(BT_RESET, OUTPUT);
  digitalWrite(BT_RESET, LOW);
  delay(100);
  pinMode(BT_RESET, INPUT);
}



// UART received event from BT module callback
// Here we process interesting events

void onEventCallback(BM64_event_t *event) {
  const char null = '\0';
  // Log all events and their payload
  #if DEBUG5
    Serial.print(F("[ EVENT "));
    Serial.print(event->event_code, HEX);
    Serial.print(F(" ]: "));
    for (uint8_t i = 0; i < event->param_len; i++) {
      Serial.print(event->parameter[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println();
  #endif

  // Call related events
  #if HOOPTYDSP
  
    // SCO active (phone/siri/nav audio)
    if ( event->event_code == BM64_EVENT_DEVICE_STATE && event->parameter[0] == 0x09 ) {
      // Override volume to preset level
      callActive = true;
      setCallVolume(); // Override volume knob
      settingMode = 15; // Show call volume setting
      showSettings(); // Show call volume setting
      showMetaData(); // Show call info
    // SCO ends
    } else if ( event->event_code == BM64_EVENT_DEVICE_STATE && event->parameter[0] == 0x0A ) {
      // Put volume back to normal
      callActive = false;
      callerID[0] = null;
      setCallVolume();
      settingMode = 0;
      showSettings();
    }
  
    // Caller ID
    if ( event->event_code == BM64_EVENT_CALL_ID && event->parameter[0] == 0x00 ) {
      for (uint8_t i = 1; i < event->param_len; i++) {
        callerID[i-1] = event->parameter[i];
      }
      callerID[event->param_len - 1] = null;
    }
  
    // Call in progress
    if ( event->event_code == BM64_EVENT_CALL_STATUS && event->parameter[1] == 0x04 ) {
      callInProgress = true;
    // Call no longer in progress
    } else if  (event->event_code == BM64_EVENT_CALL_STATUS && event->parameter[1] == 0x00 ) {
      callInProgress = false;
    }
  
  #endif

  // nSPK event
  if ( event->event_code == BM64_EVENT_REPORT_nSPK_STATUS ) {
    //bool updateLcd = 0;
    if ( event->parameter[1] <= 7 ) {
      if ( NSPK_CONNECT != event->parameter[0]) {
        NSPK_CONNECT = event->parameter[0];
        //updateLcd = 1;
      }
    }
    if ( event->parameter[2] <= 9 ) {
      if ( NSPK_STATE != event->parameter[1]) {
        NSPK_STATE = event->parameter[1];
        NSPK_GROUP = event->parameter[2];
        //updateLcd = 1;
      }
    }

// showSettings() already called by main loop
//#if LCD2004 || LCD2002
//    if ( updateLcd == 1 && settingMode == 0 ) {
//      showSettings();
//    }
//#endif

  #if DEBUG6
      Serial.print(F("[ NSPK STATUS ] "));
      for (uint8_t i = 0; i < event->param_len; i++) {
        Serial.print(event->parameter[i], HEX);
        Serial.print(F(" "));
      }
      Serial.println();
  #endif
  }

  // Metadata event
  if ( event->event_code == BM64_EVENT_AVRCP_SPEC_RSP && event->parameter[1] == 0x0C && event->parameter[2] == 0x48 ) {
    uint8_t len = ( event->parameter[10] - 10 );
    if ( len > 59 ) {
      len = 59;
    };
    if ( event->parameter[15] == 0x01 ) {
      metaDataSong[len + 1] = null;
    }
    if ( event->parameter[15] == 0x02 ) {
      metaDataArtist[len + 1] = null;
    }
    if ( DEBUG5 ) {
      Serial.print(F("Got metadata (len "));
      Serial.print(len);
      Serial.print(F("): "));
    }
    for (uint8_t i = 20; i <= len + 20 ; i++) {
      if ( event->parameter[i] == 0 && event->parameter[15] == 0x01 ) // Song name event
      {
        metaDataSong[i - 20] = null;
        break;
      }
      if ( event->parameter[i] == 0 && event->parameter[15] == 0x02 ) // Artist name event
      {
        metaDataArtist[i - 20] = null;
        break;
      }
      if ( event->parameter[15] == 0x01 ) {  // Song name event
        metaDataSong[i - 20] = char(event->parameter[i]);
      }
      if ( event->parameter[15] == 0x02 ) { // Artist name event
        metaDataArtist[i - 20] = char(event->parameter[i]);
      }
    }
    // We got the song, now get the artist
    if ( event->parameter[15] == 0x01 ) {
      if ( devicePlaying1 ) {
        bm64.getArtistName(0);
      } else if ( devicePlaying2 ) {
        bm64.getArtistName(1);
      }
    }
  }

  // Aux Silent
  if ( event->event_code == 0x1b && event->parameter[0] == 0x01 && event->parameter[1] == 0x03 ) {
    auxDetect = 0;
    if ( DEBUG5 ) {
      Serial.println(F("Aux is silent"));
    }
  }

  if ( event->event_code == 0x1b && event->parameter[0] == 0x01 && event->parameter[1] == 0x02 ) {
    auxDetect = 1;
    if ( DEBUG5 ) {
      Serial.println(F("Aux is playing"));
    }
  }

  // Link disconnect link 1
  if ( event->event_code == BM64_EVENT_DEVICE_STATE && event->parameter[0] == 0x0C && event->parameter[1] == 0x00 ) {
    for (uint8_t i = 0; i <= 12 ; i++) {
      phoneName1[i] = ' ';
    }
    deviceConnected1 = 0;
    showLinks();
  }

  // Link disconnect link 2
  if ( event->event_code == BM64_EVENT_DEVICE_STATE && event->parameter[0] == 0x0C && event->parameter[1] == 0x01 ) {
    for (uint8_t i = 0; i <= 12 ; i++) {
      phoneName2[i] = ' ';
    }
    deviceConnected2 = 0;
    showLinks();
  }

  // Link connect
  if ( event->event_code == BM64_EVENT_AVRCP_SPEC_RSP && ( event->parameter[0] == 0x00 || event->parameter[0] == 0x01 ) ) {
    bm64.getPhoneName(event->parameter[0]);
  }

  // Playing event link 1
  if ( event->event_code == BM64_EVENT_AVRCP_SPEC_RSP &&  event->parameter[0] == 0x00 && ( event->parameter[1] == 0x0d || event->parameter[1] == 0x0f) ) {
    if (  event->parameter[12] == 1 ) {
      devicePlaying1 = 1;
      getMetaData();
      showLinks();
    } else {
      devicePlaying1 = 0;
      showLinks();
      if ( devicePlaying2 == 0) {
        metaDataSong[0] = null;
        metaDataArtist[0] = null;
      }
    }
  }

  // Playing event link 2
  if ( event->event_code == BM64_EVENT_AVRCP_SPEC_RSP &&  event->parameter[0] == 0x01 && ( event->parameter[1] == 0x0d || event->parameter[1] == 0x0f) ) {
    if (  event->parameter[12] == 1 ) {
      devicePlaying2 = 1;
      getMetaData();
      #if LCD2004 || LCD2002
        showLinks();
      #endif
    } else {
      devicePlaying2 = 0;
      #if LCD2004 || LCD2002
        showLinks();
      #endif
      if ( devicePlaying1 == 0) {
        metaDataSong[0] = null;
        metaDataArtist[0] = null;
      }
    }
  }

  // Handle phone name 1 response
  if ( event->event_code == BM64_EVENT_REMOTE_DEVICE_NAME && event->parameter[0] == 0x00 && event->parameter[1] == 0x00) {
    for (uint8_t i = 2; i < event->param_len; i++) {
      // Limit to 13 chars
      if ( i <= 14 ) {
        phoneName1[i - 2] = char(event->parameter[i]);
      }
    }
    deviceConnected1 = 1;
  }

  // Handle phone name 2 response
  if ( event->event_code == BM64_EVENT_REMOTE_DEVICE_NAME && event->parameter[0] == 0x01 && event->parameter[1] == 0x00) {
    for (uint8_t i = 2; i < event->param_len; i++) {
      // Limit to 13 chars
      if ( i <= 14 ) {
        phoneName2[i - 2] = char(event->parameter[i]);
      }
    }
    deviceConnected2 = 1;
  }
}
#endif
