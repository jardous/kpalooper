#include <bluefruit.h>

BLEClientDis  clientDis;
BLEClientMidi clientMidi;


// https://github.com/BurenkovS/bjdevlib/blob/master/tbseries/src/midi.c !!!
// AVOID USING FUCKING MIDI.sendControlChange !!! it is not reliable
#define CC_MSG_LEN 5
#define CC_STATUS   0xB0
#define MIDI_CHANNEL 1
// Create a new instance of the Arduino MIDI Library,
// and attach BluefruitLE MIDI as the transport.
//MIDI_CREATE_INSTANCE(BLEClientMidi, clientMidi, MIDI)



static byte cc_msg[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
void kpaSendControlChange(uint8_t ctrlNum, uint8_t val, uint8_t chanNum = MIDI_CHANNEL)
{
  short t = millis() & 0xFFFF;
  cc_msg[0] = (t >> 7) & 0x3F | 0x80;
  cc_msg[1] = t & 0x7F | 0x80;
  cc_msg[2] = CC_STATUS | (0x0F & chanNum);
  cc_msg[3] = 0x7F & ctrlNum;
  cc_msg[4] = 0x7F & val;

  clientMidi.write(cc_msg, sizeof(cc_msg));
}

void sendNrpn(byte addr_msb, byte addr_lsb, byte val_msb, byte val_lsb) {
  kpaSendControlChange(99, addr_msb);
  kpaSendControlChange(98, addr_lsb);
  kpaSendControlChange(06, val_msb);
  kpaSendControlChange(38, val_lsb);
  clientMidi.flush();
}
void sendNrpn(short nrpn, short value) {
  sendNrpn(nrpn >> 7, nrpn & 0x7F, value >> 7, value & 0x7F);
}

void blemidi_rx_callback(BLEClientMidi& midi_svc);  // defined in the main module


/**
   Callback invoked when scanner pick up an advertising data
   @param report Structural advertising data
*/
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  // Check if advertising contain BleMidi service
  if ( Bluefruit.Scanner.checkReportForService(report, clientMidi) )
  {
    Serial.print("BLE Midi service detected. Connecting ... ");

    // Connect to device with bleMidi service in advertising
    Bluefruit.Central.connect(report);
  }
}

/**
   Callback invoked when an connection is established
   @param conn_handle
*/
void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");

  Serial.print("Dicovering DIS ... ");
  if ( clientDis.discover(conn_handle) )
  {
    Serial.println("Found it");
    char buffer[32 + 1];

    // read and print out Manufacturer
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getManufacturer(buffer, sizeof(buffer)) )
    {
      Serial.print("Manufacturer: ");
      Serial.println(buffer);
    }

    // read and print out Model Number
    memset(buffer, 0, sizeof(buffer));
    if ( clientDis.getModel(buffer, sizeof(buffer)) )
    {
      Serial.print("Model: ");
      Serial.println(buffer);
    }

    Serial.println();
  }

  Serial.print("Discovering BLE Midi Service ... ");

  if ( clientMidi.discover(conn_handle) )
  {
    Serial.println("Found it");

    Serial.print("Enable TXD's notify: ");
    Serial.println(clientMidi.enableTXD());

    Serial.println("Ready to receive from peripheral");
    ledRainbow.m_active = false;

    uint32_t color = strip.Color(0x55, 0x55, 0x55);
    strip.setPixelColor(0, color);
    strip.show();
    
  } else {
    Serial.println("Found NONE");

    // disconect since we couldn't find bleMidi service
    Bluefruit.Central.disconnect(conn_handle);
    ledRainbow.reset();
  }
}

/**
   Callback invoked when a connection is dropped
   @param conn_handle
   @param reason
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println("Disconnected");
  ledRainbow.reset();
}


void conn_setup() {
  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);

  Bluefruit.setName("Bluefruit52 Central");

  // Configure DIS client
  clientDis.begin();

  clientMidi.setRxCallback(blemidi_rx_callback);
  // Init BLE Central Midi Serivce
  clientMidi.begin();


  // Increase Blink rate to different from PrPh advertising mode
  Bluefruit.setConnLedInterval(250);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Interval = 100 ms, window = 80 ms
     - Don't use active scan
     - Start(timeout) with timeout = 0 will scan forever (until connected)
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.useActiveScan(true);
  Bluefruit.Scanner.start(0);                   // // 0 = Don't stop scanning after n seconds
}
