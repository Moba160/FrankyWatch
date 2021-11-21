#include "DrivePage.h"
#include <Z21.h>

// Breite Bedienelemente
#define SYMBOL_SIZE 26     // Richtungsdreieck, sollte geradzahlig sein
#define CIRCLE_DIAM 40     // Licht ein/aus
#define DIAM_PLUSMINUS 76  // Fahrstufe +/-
#define WIDTH_STEP  44     // Breite/Höhe Fahrstufendelta-Buttons
#define HEIGHT_STEP 30

// Erste Zeile: Richtung, Adresse, Licht
#define Y_LINE_ONE 40

// Zweite Zeile: Geschwindigkeit und +/- Buttons
#define Y_LINE_TWO TFT_HEIGHT/2

// Maxwerte
#define MAX_LOCOADDRESS 9999
#define MAX_LOCOSPEED 126

DrivePage* DrivePage::pages[NumDrivePages];

int DrivePage::lastPage = 0;

bool DrivePage::linkLoco = Pref::get(prefNameLink).toInt() != 0;

// ----------------------------------------------------------------------------------------------------
//

DrivePage::DrivePage(char channel, char navigable) : Page(navigable), channel{channel} {

  pages[lastPage++] = this;

  displayedLocoAddress = locoAddress = Pref::get(prefNameLocoChannelAddr + String(channel), "3").toInt();
  Serial.printf("Letzte Adresse Kanal %c: %s\n", channel, Pref::get(prefNameLocoChannelAddr + String(channel), "(noch keine)").c_str());

  btnHeadlights = new TouchButton(TouchButton::Circle, TFT_WHITE, TFT_ORANGE, TouchButton::Toggle, "", CIRCLE_DIAM, CIRCLE_DIAM, TFT_WIDTH / 8 * 7 - 5, Y_LINE_ONE);

  // Halbkreise, weil die Kreise ins "Off" reichen
  btnVMinus = new TouchButton(TouchButton::Circle, TFT_WHITE, TFT_RED, TouchButton::Push, "", DIAM_PLUSMINUS, DIAM_PLUSMINUS, 0, Y_LINE_TWO); // false
  btnVPlus = new TouchButton(TouchButton::Circle, TFT_WHITE, TFT_GREEN, TouchButton::Push, "", DIAM_PLUSMINUS, DIAM_PLUSMINUS, TFT_WIDTH, Y_LINE_TWO); // false

  // 5 Adressdelta-Buttons, zentriert um Button 0
  btnDeltaAddress[0] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "10", WIDTH_STEP, HEIGHT_STEP, TouchButton::BottomCentered);
  btnDeltaAddress[1] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "5", WIDTH_STEP, HEIGHT_STEP, TouchButton::LeftTo, btnDeltaAddress[0]);
  btnDeltaAddress[2] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "50", WIDTH_STEP, HEIGHT_STEP, TouchButton::RightTo, btnDeltaAddress[0]);
  btnDeltaAddress[3] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "1", WIDTH_STEP, HEIGHT_STEP, TouchButton::LeftTo, btnDeltaAddress[1]);
  btnDeltaAddress[4] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "100", WIDTH_STEP, HEIGHT_STEP, TouchButton::RightTo, btnDeltaAddress[2]);
  for (int i=0; i<NumDeltaButtonsAddress; i++) btnDeltaAddress[i]->setToggleColors(TFT_BLACK, TFT_BLUE);

  // 3 Speeddelta-Buttons, zentriert um Button 0
  btnDeltaSpeed[0] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "5", WIDTH_STEP, HEIGHT_STEP, TouchButton::BottomCentered);
  btnDeltaSpeed[1] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "1", WIDTH_STEP, HEIGHT_STEP, TouchButton::LeftTo, btnDeltaSpeed[0]);
  btnDeltaSpeed[2] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "10", WIDTH_STEP, HEIGHT_STEP, TouchButton::RightTo, btnDeltaSpeed[0]);
  for (int i=0; i<NumDeltaButtonsSpeed; i++) btnDeltaSpeed[i]->setToggleColors(TFT_BLACK, TFT_BLUE);

  // Unsichtbare Buttons für Moduswahl
  btnAddressMode = new TouchButton(TouchButton::Invisible, 0, 0, TouchButton::Push, "", 100, 40, TFT_WIDTH / 2, Y_LINE_ONE);
  btnSpeedMode = new TouchButton(TouchButton::Invisible, 0, 0, TouchButton::Push, "", DIAM_PLUSMINUS, DIAM_PLUSMINUS, TFT_WIDTH / 2, Y_LINE_TWO);

  // dto für Richtungswahl
  btnDirection = new TouchButton(TouchButton::Invisible, 0, 0, TouchButton::Push, "", 120, 120, 0, 0);

  for (int i=0; i<MaxFct; i++) fct[i] = false;

    // Etc

  Z21::addObserver(static_cast<Page*>(this));

}

// ----------------------------------------------------------------------------------------------------
//

void DrivePage::setVisible(bool visible, bool force) {

  Page::setVisible(visible, force);

  if (visible) {
    drawLocoAddress(force);
    drawLocoSpeed(force);
    drawMode(force);
    drawDirection(force);
    drawHeadlights(force);
    Z21::LAN_X_GET_LOCO_INFO(getAddress());
  }

  // alles andere funktioniert über Buttons

  btnDirection->setVisible(visible);
  btnHeadlights->setVisible(visible);
  btnAddressMode->setVisible(visible);
  btnSpeedMode->setVisible(visible);
  btnVMinus->setVisible(visible);
  btnVPlus->setVisible(visible);

  if (mode == Address) 
    for(int i=0; i<NumDeltaButtonsAddress; i++) btnDeltaAddress[i]->setVisible(visible);
  if (mode == Speed)
    for(int i=0; i<NumDeltaButtonsSpeed; i++) btnDeltaSpeed[i]->setVisible(visible);

  if (visible) navigationHint();
  
}

// ----------------------------------------------------------------------------------------------------
// Diverse Infos darstellen

void DrivePage::drawLocoAddress(boolean force) {

  // Kein Update nötig
  if (!force && locoAddress == displayedLocoAddress) return;

  tft->setTextDatum(MC_DATUM);
  tft->setTextColor(fgColor, bgColor);
  tft->fillRect(TFT_WIDTH / 2 - 60, Y_LINE_ONE - 12, 120, 22, bgColor);
  tft->drawString(String(channel) + " " + "Lok " + locoAddress, TFT_WIDTH / 2, Y_LINE_ONE, 4);

  displayedLocoAddress = locoAddress;
}

void DrivePage::drawLocoSpeed(boolean force) {

  // Kein Update nötig
  if (!force && locoSpeed == displayedLocoSpeed) return;

  tft->setTextDatum(MC_DATUM);
  tft->setTextColor(fgColor, bgColor);
  tft->fillRect(DIAM_PLUSMINUS / 2 + 1, Y_LINE_TWO - DIAM_PLUSMINUS / 2, TFT_WIDTH - 2 * DIAM_PLUSMINUS / 2, DIAM_PLUSMINUS, bgColor);
  tft->drawString(String(locoSpeed), TFT_WIDTH / 2, Y_LINE_TWO, 8);

  displayedLocoSpeed = locoSpeed;
}

void DrivePage::drawHeadlights(boolean force) {

  // Kein Update nötig
  if (!force && fct[0] == displayedHeadlights) return;

  btnHeadlights->set(fct[0]);
  btnHeadlights->setVisible(true);

  displayedHeadlights = fct[0];
}

void DrivePage::drawMode(boolean force) {
  tft->setTextDatum(MC_DATUM);
  tft->setTextColor(fgColor, bgColor);
  tft->fillRect(0, 180, TFT_WIDTH, 20, bgColor);
  tft->drawString(mode == Address ? "Delta Adresse" : "Delta Fahrstufe", TFT_WIDTH / 2, 190, 2);
  if (mode == Address) {
    for (int i = 0; i < NumDeltaButtonsSpeed; i++) btnDeltaSpeed[i]->setVisible(false);
    for (int i = 0; i < NumDeltaButtonsAddress; i++) {
      btnDeltaAddress[i]->set(i==idxDeltaButtonAddress);
      btnDeltaAddress[i]->setVisible(true);
    }
  } else {
    for (int i = 0; i < NumDeltaButtonsAddress; i++) btnDeltaAddress[i]->setVisible(false);
    for (int i = 0; i < NumDeltaButtonsSpeed; i++) {
      btnDeltaSpeed[i]->set(i==idxDeltaButtonSpeed);
      btnDeltaSpeed[i]->setVisible(true);
    }
  }
}

void DrivePage::drawDirection(boolean force) {

  // Kein Update nötig
  if (!force && direction == displayedDirection) return;

  tft->fillRect(TFT_WIDTH / 8 - SYMBOL_SIZE / 2, Y_LINE_ONE - SYMBOL_SIZE / 2, SYMBOL_SIZE + 1, SYMBOL_SIZE + 1, bgColor);
  byte x1 = TFT_WIDTH / 8 - SYMBOL_SIZE / 2; byte x2 = x1 + SYMBOL_SIZE; byte x12 = (x1 + x2) / 2;
  if (direction == Direction::Forward) {
    tft->fillTriangle(x1, Y_LINE_ONE + SYMBOL_SIZE / 2, x2, Y_LINE_ONE + SYMBOL_SIZE / 2, x12, Y_LINE_ONE - SYMBOL_SIZE / 2, fgColor);
  } else {
    tft->fillTriangle(x1, Y_LINE_ONE - SYMBOL_SIZE / 2, x2, Y_LINE_ONE - SYMBOL_SIZE / 2, x12, Y_LINE_ONE + SYMBOL_SIZE / 2, fgColor);
  }

  displayedDirection = direction; 
}

// ----------------------------------------------------------------------------------------------------
//

void DrivePage::touchButtonPressed(TouchButton* button, int duration) {

  bool redrawDelta = false;

  // Licht
  if (button == btnHeadlights) {
    fct[0] = ! fct[0];
    button->set(fct[0]);
    Z21::LAN_X_SET_LOCO_FUNCTION(locoAddress, 0, fct[0]);
    return;
  }

  // Loknothalt
  if (button == btnSpeedMode && duration > 500) {
    Z21::locoStop(locoAddress, direction);
    return;
  }

  // Moduswechsel Adress-/Speedänderung
  if (button == btnAddressMode || button == btnSpeedMode) {
    // a) Wechsel von Adresseingabe zu Geschwindigkeitseingabe -> Lokinfo aktualisieren
    // b) Kein Moduswechsel -> auch Lokinfo aktualisieren (kann damit erzwungen werden)
    if (button == btnSpeedMode) {
      Z21::LAN_X_GET_LOCO_INFO(locoAddress); 
      // Als letztgewählte Adresse speichern. Sollte sie sich nicht geändert haben,
      // fängt das Pref::set ab
      Pref::set(prefNameLocoChannelAddr + String(channel), String(locoAddress));
    }
    mode = button == btnSpeedMode ? Speed : Address;
    redrawDelta = true;
  } 
  
  // Deltabutton Adresse
  for (int i=0; i<NumDeltaButtonsAddress; i++) {
    if (button == btnDeltaAddress[i]) {
      redrawDelta = true;
      btnDeltaAddress[i]->set(true);
      idxDeltaButtonAddress = i;
    } else {
      btnDeltaAddress[i]->set(false);    
    }
  }

  // Deltabutton Speed
  for (int i=0; i<NumDeltaButtonsSpeed; i++) {
    if (button == btnDeltaSpeed[i]) {
      redrawDelta = true;
      btnDeltaSpeed[i]->set(true);
      idxDeltaButtonSpeed = i;
    } else {
      btnDeltaSpeed[i]->set(false);    
    }
  }

  if (redrawDelta) drawMode(false);

  // Richtungsänderung
  if (button == btnDirection) {
    direction = direction == Direction::Forward ? Direction::Backward : Direction::Forward;
    drawDirection(false);
    locoSpeed = 0; drawLocoSpeed(false);
    Z21::LAN_X_SET_LOCO_DRIVE(locoAddress, direction, locoSpeed);
    return;
  }

  // Geschwindigkeitsänderung

  if (button == btnVPlus || button == btnVMinus) {
    if (button == btnVPlus && duration > 500) {
      switch (mode) {
        case Speed:
          locoSpeed = 126; drawLocoSpeed(false);
          break;
        case Address:
          setAddress(MAX_LOCOADDRESS); drawLocoAddress(false);
          break;
      }
      Z21::LAN_X_SET_LOCO_DRIVE(locoAddress, direction, locoSpeed);
      return;
    }

    if (button == btnVMinus && duration > 500) {
      switch (mode) {
        case Speed:
          locoSpeed = 0; drawLocoSpeed(false);
          break;
        case Address:
          setAddress(1); drawLocoAddress(false);
          break;
      }
      Z21::LAN_X_SET_LOCO_DRIVE(locoAddress, direction, locoSpeed);
      return;
    }

    // Erhöhen von Adresse oder Fahrstufe, je nach Modus
    int delta;
    if (mode == Speed) {
      delta = btnDeltaSpeed[idxDeltaButtonSpeed]->getLabel().toInt();
      locoSpeed = locoSpeed + (button == btnVPlus ? delta : -delta);
      locoSpeed = locoSpeed - locoSpeed % delta;
      if (locoSpeed < 0) locoSpeed = 0;
      if (locoSpeed > 126) locoSpeed = MAX_LOCOSPEED;
      drawLocoSpeed(false);
      Z21::LAN_X_SET_LOCO_DRIVE(locoAddress, direction, locoSpeed);
      return;
    } else { // Address
      delta = btnDeltaAddress[idxDeltaButtonAddress]->getLabel().toInt();
      incrLocoAddress((button == btnVPlus ? delta : -delta));
      setAddress(locoAddress - locoAddress % delta);
      if (locoAddress < 1) setAddress(1);
      if (locoAddress > MAX_LOCOADDRESS) setAddress(MAX_LOCOADDRESS);
      drawLocoAddress(false);
      return;
    }
  }
}

// ----------------------------------------------------------------------------------------------------
// 
void DrivePage::locoInfoChanged(int addr, Direction dir, int fst, bool takenOver, int numSpeedSteps, bool f[]) {

  // Ignorieren, wenn nicht aktuell gefahrene Lok (aber nicht ignorieren, wenn Lauschmodus)
  if (!linkLoco && addr != getAddress()) return;

  if (isVisible()) {
    setAddress(addr);
    direction = dir;
    locoSpeed = fst;

    for (int i=0; i<MaxFct; i++) this->fct[i] = fct[i]; 
    
    // Update der empfangenen Infos (false = nur, wenn geändert)
    drawDirection(false);
    drawLocoSpeed(false);
    drawHeadlights(false);
    drawLocoAddress(false);
  }
}