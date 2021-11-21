#define LILYGO_WATCH_2020_V1
#include <LilyGoWatch.h>

#include "esp_wifi.h"
#include <WiFi.h>
#include <Z21.h>
#include "configuration.h"
#include "Pref.h"
#include "Page.h"
#include "DrivePage.h"

extern void webconfig();

TTGOClass* watch = TTGOClass::getWatch();
BMA* bmaSensor;
WiFiUDP Udp;

// ----------------------------------------------------------------------------------------------------
//

void setup() {
  
  Serial.begin(115200);
  Pref::begin();

  Serial.println(PRODUCT_NAME);

  watch->begin();
  watch->tft->setRotation(0);
  watch->openBL(); // Hintergrundbeleuchtung ein
  // watch->motor_begin(); // Vibrationsmotor. Verdunkelt schon beim ersten Mal das
  // Display, verbraucht also signifikant Energie
  // watch->enableLDO3(); // MP3 Vorher testen, ob Webzugriff noch möglich

  // WLAN-Verbindung konfigurieren bzw. verbinden. Initial: AccessPoint, danach
  // Station Mode
  webconfig(); 
  Serial.println("Gestartet");

  // Z21
  Z21::setIPAddress(Pref::get(prefNameZ21Addr, Z21_DEFAULT_ADDR));
  Udp.begin(Z21_PORT);
  Z21::init();

  // Lagesensor
  bmaSensor = watch->bma;
  Acfg cfg;
  cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  cfg.range = BMA4_ACCEL_RANGE_2G;
  cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  cfg.perf_mode = BMA4_CONTINUOUS_MODE;
  bmaSensor->accelConfig(cfg);
  bmaSensor->enableAccel();

  // GUI initialisieren
  Page::begin(watch);

  if (!Page::isBlocked()) Page::currentPage()->setVisible(true, true);
}


long lastHeartbeat = 0;
int oldWiFiConnectionState = !WL_CONNECTED;
boolean oldZ21Received = -123; // damit auf jeden Fall anfangs Änderung
boolean z21Connected = false;
long lastAccMeasured = 0;
int accMeasureInterval = 200;
int accX=0, accY=0, accZ=0;
Accel acc;

// ----------------------------------------------------------------------------------------------------
//

void loop() {

  // Lebenszeichen
  if (millis() - lastHeartbeat > Z21_HEARTBEAT) {
    Z21::heartbeat();
    lastHeartbeat = millis();
    for (int i=0; i<NumDrivePages; i++) 
      if (DrivePage::pages[i]->isVisible()) 
        Z21::LAN_X_GET_LOCO_INFO(DrivePage::pages[i]->getAddress());
  }

  if (WiFi.status() == WL_CONNECTED && !z21Connected) {
    Serial.printf("Verbinde mit Z21/DR5000 %s@%d\n", Pref::get(prefNameZ21Addr).c_str(), Z21_PORT);
    Z21::init();
    z21Connected = true;

    // Lokdaten erstmals für aktuelle Lokseite abrufen (später regelmäßig während Lebenszeichen)
    for (int i=0; i<NumDrivePages; i++) 
      if (DrivePage::pages[i]->isVisible()) 
        Z21::LAN_X_GET_LOCO_INFO(DrivePage::pages[i]->getAddress());
  }

  // Geänderten WiFi-Verbindungsstatus anzeigen
  if (WiFi.status() != oldWiFiConnectionState) {
    Page::currentPage()->navigationHint();
    oldWiFiConnectionState = WiFi.status();
  }

  // Geänderten Z21-Verbindungsstatus anzeigen
  bool z21Received = millis() - Z21::lastReceived <= Z21_HEARTBEAT;
  if (z21Received != oldZ21Received) {
      Page::currentPage()->navigationHint();
      oldZ21Received = z21Received;
  }

//   // Beschleunigungsmesser
//   if (millis() - lastAccMeasured > accMeasureInterval) {
//     if (bmaSensor->getAccel(acc)) {
//       watch->tft->setCursor(0, 0);
//       static char xyz[20];
//       sprintf(xyz, "%d %d %d   ", acc.x, acc.y, acc.z);
//       watch->tft->setTextColor(TFT_WHITE, TFT_BLACK);
//       watch->tft->drawString(xyz, 0, 0, 2);
//       watch->tft->fillCircle(220, 10, 10, acc.y > 500 && acc.z > -800? TFT_RED : TFT_GREEN);
//       // https://diyprojects.io/t-watch-sleep-and-wake-up-with-bma423-accelerometer-or-esp32-button/#.YDA4cOj0mUk
//     }
//     lastAccMeasured = millis();
//   }

  // Prüfen, ob Rückmeldungen von Z21 vorliegen
  Z21::receive();

  // GUI-Events
  Page::loop();
  TouchButton::loop();
}

// // ----------------------------------------------------------------------------------------------------
// // Z21 Callbacks
// // Werden als offene Referenzen in der Z21-Lib definiert und hier erfüllt
// // = weitergeleitet and entsprechende Interessenten(seiten)

// ///// Trace-Eintrag
// void Z21::trace(byte toZ21, long timeDiff, String message, String parameter) {
//   // Serial.printf("%s %ld %s: %s\n", toZ21 == TO_Z21 ? "\t>" : "\t\t<", timeDiff, message.c_str(), parameter.c_str());
//   // Page::notifyTrace(toZ21 == TO_Z21, timeDiff, message, parameter);
// }

// ///// Gleisspannung ein/aus
// void Z21::trackPowerStateChanged(Z21::TrackPowerState state) {
//   Serial.println(state == Z21::Off ? "AUS" : "EIN");
//   Page::currentPage()->navigationHint(); // sorgt für Statusrahmen
//   Page::notifyTrackPowerChanged(state);
//   // if (state == Z21::Off) { // Auskommentiert: bewirkt sofortigen Spannungsabfall, Display wird dunkler oder schaltet sogar ab!
//   //   watch->motor->onec(); 
//   // } 
// }

// ///// Lok-Info
// void Z21::locoInfo(int addr, boolean forward, int fst, boolean takenOver, int numSpeedSteps, boolean fct[]) {
//   Serial.printf("Lok %d Fst %d %c ", addr, fst, forward ? '>' : '<');
//   for (int i=0; i<MaxFct; i++) if (fct[i]) Serial.printf("F%d ", i);
//   Serial.println();
//   Page::notifyLocoInfoChanged(addr, forward, fst, takenOver, numSpeedSteps, fct);
// }

// ///// Zubehör-Info
// void Z21::accessoryInfo(int addr, Z21::AccState state) {
//   Serial.printf("Zubehör %d%c\n", addr, state == Z21::Plus ? '+' : '-');
//   Page::notifyAccessoryChanged(addr, state);
// }

// ///// Programmiermodus
// void Z21::progStateChanged(Z21::ProgState state) {
//   Serial.printf("Programmiermodus %s\n", state == Z21::POn ? "EIN" : "AUS");
//   Page::notifyProgStateChanged(state);
//   Page::currentPage()->navigationHint(); // sorgt für Statusrahmen
// }
