#include <MFRC522.h>
#include <SPI.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define	BLACK   0x0000
#define	RED     0xF800
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREEN   0x0FF0

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// MFRC522 wiring details.
//
// Not using the RST -- but cannot use something like -1 -- 9 happens
// to be safe and not confuse the LCD too much.
//
#define RST_PIN         9
#define SS_PIN          10
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Build " __FILE__ " / " __DATE__ " " __TIME__ ));

  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(1);

  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522 module
  mfrc522.PCD_DumpVersionToSerial();  // Show version of PCD - MFRC522 Card Reader

#if 0
  // Do not od a selftest - as we cannot do a reset to get the
  // card back in working order (RST is not wired up). We should
  // perhaps wire the card up to the LCD reset - so we can
  // recover from below self test.
  //
  bool result = mfrc522.PCD_PerformSelfTest(); // perform the test
  tft.println(F("-----------------------------"));
  tft.print(F("Selftest: "));
  if (result)
    tft.println(F("OK"));
  else
    tft.println(F("DEFECT or UNKNOWN"));
  tft.println();
#endif
}

void firmware() {
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  tft.setTextSize(1);
  tft.print(F("Firmware Version: 0x"));
  tft.print(v, HEX);
  // Lookup which version
  switch (v) {
    case 0x00: tft.println(F(" = -- propably a comms error!"));  break;
    case 0x88: tft.println(F(" = (clone)"));  break;
    case 0x90: tft.println(F(" = v0.0"));     break;
    case 0x91: tft.println(F(" = v1.0"));     break;
    case 0x92: tft.println(F(" = v2.0"));     break;
    default:   tft.println(F(" = (unknown)"));
  }

}
void wipe() {
  tft.fillScreen(BLACK);

  tft.setCursor(0, 0);
  tft.setTextColor(GREEN);
  tft.setTextSize(5);
  tft.println(F("RFID Tester"));
  tft.setTextColor(YELLOW); tft.setTextSize(2);
  tft.print(F("Makerspace"));
  tft.setTextColor(RED);
  tft.println(F("  Leiden"));

}

void loop(void) {
  wipe();
  tft.setTextColor(WHITE);
  tft.println();
  firmware();
  tft.println();

  tft.setTextColor(YELLOW);
  tft.setTextSize(1);

  for (int j = 0; j < 17; j++) {
    //  tft.fillRect(0,tft.height()-100,tft.width(), 100, Black);

    long t = millis();
    int present = 0;
    while ((t + 2000 > millis()) && (present == 0)) {
      present = mfrc522.PICC_IsNewCardPresent();
    }

    if (!present) {
      tft.println(F(" - No card detected"));
      continue;
    }

    if ( ! mfrc522.PICC_ReadCardSerial()) {
      tft.println(F(" = Card dected  -but could not read it"));
      continue;
    }

    tft.print(F(" * Card: "));
    for (int i = 0; i < mfrc522.uid.size; i++ ) {
      if (i) tft.print('-');
      tft.print(mfrc522.uid.uidByte[i]);
    }
    tft.print(F("  (len: "));
    tft.print(mfrc522.uid.size);
    tft.println(F(")."));
  }

  return;
}
