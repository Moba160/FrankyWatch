#include "Page.h"
#include "TouchButton.h"
#include "DrivePage.h"
#include "FunctionPage.h"
#include "AccessoryPage.h"
#include "ConfigPage.h"
#include "AboutPage.h"
#include "TracePage.h"
#include "Pref.h"
#include "configuration.h"
#include <WiFi.h>

TFT_eSPI* Page::tft;
int Page::fgColor = TFT_WHITE;
int Page::bgColor = TFT_BLACK, Page::bgColorOn;

int Page::row;
int Page::col;

Page* Page::navigationGrid[PAGE_ROWS][PAGE_COLS];
bool Page::blocked = false;

// ----------------------------------------------------------------------------------------------------
//

void Page::begin(TTGOClass* watch) {

  TouchButton::begin();

  AboutPage* ap = new AboutPage(NAV_RIGHT | NAV_DOWN);
  ConfigPage* cp = new ConfigPage(NAV_RIGHT | NAV_UP);
  DrivePage* dpA = new DrivePage('A', NAV_ALL);
  DrivePage* dpB = new DrivePage('B', NAV_ALL);
  DrivePage* dpC = new DrivePage('C', NAV_ALL);

  Page* tmp[PAGE_ROWS][PAGE_COLS] = {
    {0,  new TracePage(NAV_ALL), 0},
    {ap,  dpA, new FunctionPage(dpA, NAV_LEFT)},
    {cp,  dpB, new FunctionPage(dpB, NAV_LEFT)},
    {0,  dpC, new FunctionPage(dpC, NAV_LEFT)},
    {new AccessoryPage(1, NAV_RIGHT),  new AccessoryPage(26, NAV_ALL), new AccessoryPage(51, NAV_LEFT)}
  };

  // Initialisierung oben hat nicht funktioniert, Konstruktoren wurden nicht gerufen
  memcpy(navigationGrid, tmp, sizeof(tmp));

  // initiale Seite
  row = 1; col = 1; 
  
  tft = watch->tft;
}

// ----------------------------------------------------------------------------------------------------
//

void Page::loop() {

  return;

  ////////// Rotation

  static int prevRotation;
  int rotation = bmaSensor->direction();
  if (prevRotation != rotation) {
    prevRotation = rotation;

    switch (rotation) {
      case DIRECTION_DISP_DOWN:
        break;
      case DIRECTION_DISP_UP:
        break;
      case DIRECTION_BOTTOM_EDGE:
        tft->setRotation(2);
        break;
      case DIRECTION_TOP_EDGE:
        tft->setRotation(0);
        break;
      case DIRECTION_RIGHT_EDGE:
        // tft->setRotation(3);
        break;
      case DIRECTION_LEFT_EDGE:
        // tft->setRotation(1);
        break;
      default:
        break;
    }
    //Display::locoPage();
  }

  ///// Touchbuttons

}

// ----------------------------------------------------------------------------------------------------
//

void Page::swipeGesture(TouchButton::Gesture gesture, int distance) {

  if (blocked) return;

  int oldCol = col, oldRow = row;

  switch (gesture) {
    // Umschalten auf Nachbarseite, sofern diese existiert (zweites if) und
    // wenn dorthin navigieren erlaubt (letztes if)
    case TouchButton::Down:
      if (row > 0) if (navigationGrid[row - 1][col] != 0) {
        if ((currentPage()->navigable & NAV_UP) > 0) row--;
      }
      break;
    case TouchButton::Up:
      if (row < PAGE_ROWS - 1) if (navigationGrid[row + 1][col] != 0) {
        if ((currentPage()->navigable & NAV_DOWN) > 0) row++;
      }
      break;
    case TouchButton::Right:
      if (col > 0) if (navigationGrid[row][col - 1] != 0) {
        if ((currentPage()->navigable & NAV_LEFT) > 0) col--;
      }
      break;
    case TouchButton::Left:
      if (col < PAGE_COLS - 1) if (navigationGrid[row][col + 1] != 0) {
        if ((currentPage()->navigable & NAV_RIGHT) > 0) col++;
      }
      break;
  }

  // Falls es neue Seite gibt, alte verbergen und neue anzeigen
  if (oldCol != col || oldRow != row) {
    navigationGrid[oldRow][oldCol] -> setVisible(false, false);
    navigationGrid[row][col] -> setVisible(true, true);
  }

}

// ----------------------------------------------------------------------------------------------------
//

#define NAV_IND_LEN  10 // Länge des Indikatorbalkens
#define NAV_IND_WIDTH 4 // Dicke 

void Page::setVisible(bool visible, bool clearScreen) {

  this->visible = visible;

  tft->setTextColor(fgColor, bgColor);
  if (clearScreen) tft->fillScreen(bgColor);

}

// ----------------------------------------------------------------------------------------------------
//

void Page::navigationHint() {

  if (blocked) return;

  int color = colorAllConnected;
  if (WiFi.status() != WL_CONNECTED) color = colorWiFiDisconnected;
  else if (millis() - Z21::lastReceived > Z21_HEARTBEAT) color = colorDigiStationDisconnected;
  else if (Z21::getProgState() == BoolState::On) color = colorProgMode;
  else if (Z21::getTrackPowerState() == BoolState::Off) color = colorTrackPowerOff;

  // Status: Balken oben
  tft->fillRect(0, 0, TFT_WIDTH, NAV_IND_WIDTH, color);

  // Am jeweiligen Rand Navigationshinweis (dass es dort eine Nachbarseite gibt)

  // oben
  if (row > 0) {
    if (navigationGrid[row-1][col] != 0) 
      if ((currentPage()->navigable & NAV_UP))
        tft->fillRect(tft->width()/2-NAV_IND_LEN/2, 0, NAV_IND_LEN, NAV_IND_WIDTH, fgColor);
  }
  // unten
  if (row < PAGE_ROWS - 1) {
    if (navigationGrid[row+1][col] != 0) 
      if ((currentPage()->navigable & NAV_DOWN))
        tft->fillRect(tft->width()/2-NAV_IND_LEN/2, tft->height()-NAV_IND_WIDTH+1, NAV_IND_LEN, NAV_IND_WIDTH, fgColor);
  }
  // links
  if (col > 0) {
    if (navigationGrid[row][col-1] != 0) 
      if ((currentPage()->navigable & NAV_LEFT))
        tft->fillRect(0, tft->height()/2-NAV_IND_LEN/2, NAV_IND_WIDTH, NAV_IND_LEN, fgColor);
  } 
   // rechts
  if (col < PAGE_COLS - 1) {
    if (navigationGrid[row][col+1] != 0)
      if ((currentPage()->navigable & NAV_RIGHT))
        tft->fillRect(tft->width()-NAV_IND_WIDTH+1, tft->height()/2-NAV_IND_LEN/2, NAV_IND_WIDTH, NAV_IND_LEN, fgColor);
  } 

}

// // ----------------------------------------------------------------------------------------------------
// //

// void Page::notifyTrackPowerChanged(Z21::TrackPowerState state) {
//   for (int r=0; r<PAGE_ROWS; r++)
//     for (int c=0; c<PAGE_COLS; c++)
//       if (navigationGrid[r][c] != 0) {
//         navigationGrid[r][c]->trackPowerChanged(state);
//       }
// }

Page* Page::currentPage() {
  return navigationGrid[row][col];
}

// ----------------------------------------------------------------------------------------------------
//

void Page::trackPowerStateChanged(BoolState trackPowerState) {
  navigationHint();
}

void Page::shortCircuitStateChanged(BoolState shortCircuitState) {
  navigationHint();
}

void Page::emergencyStopStateChanged(BoolState emergencyStopState) {
  navigationHint();
}

void Page::progStateChanged(BoolState progState) {
  navigationHint();
}

// void Page::notifyAccessoryChanged(int addr, Z21::AccState state) {
//   for (int r=0; r<PAGE_ROWS; r++) 
//     for (int c=0; c<PAGE_COLS; c++) 
//       if (navigationGrid[r][c] != 0) {
//         navigationGrid[r][c]->accessoryChanged(addr, state);
//       }
// }


