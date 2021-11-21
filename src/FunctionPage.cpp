#include "FunctionPage.h"
#include "DrivePage.h"
#include <Z21.h>

// ----------------------------------------------------------------------------------------------------
//

#define BTN_SIZE 42

FunctionPage::FunctionPage(DrivePage* dp, char navigable) : Page(navigable), dp{dp} {

  for (int r = 0; r < FCT_ROWS; r++) {
    for (int c = 0; c < FCT_COLS; c++) {
      int i = r * FCT_COLS + c;
      if (r == 0 && c == 0) {
        button[r][c] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_ORANGE, TouchButton::Toggle, String(i), BTN_SIZE, BTN_SIZE, TouchButton::TopLeft);
      } else if (r != 0 && c == 0) {
        button[r][c] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_ORANGE, TouchButton::Toggle, String(i), BTN_SIZE, BTN_SIZE, TouchButton::Below, button[r - 1][c]);
      } else {
        button[r][c] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_ORANGE, TouchButton::Toggle, String(i), BTN_SIZE, BTN_SIZE, TouchButton::RightTo, button[r][c - 1]);
      }
      button[r][c]->setToggleColors(TFT_BLACK, TFT_ORANGE);
    }
  }

  // Etc

  Z21::addObserver(static_cast<Page*>(this));
}

// ----------------------------------------------------------------------------------------------------
//

void FunctionPage::setVisible(bool visible, bool clearScreen) {

  Page::setVisible(visible, clearScreen);

  for (int r = 0; r < FCT_ROWS; r++) {
    for (int c = 0; c < FCT_COLS; c++) {
      button[r][c] -> set(dp->getFct(r*FCT_ROWS+c));
      button[r][c] -> setVisible(visible);
    }
  }

  if (visible) navigationHint();
}

// ----------------------------------------------------------------------------------------------------
//
void FunctionPage::touchButtonPressed(TouchButton* btn, int duration) {

  for (int r = 0; r < FCT_ROWS; r++) {
    for (int c = 0; c < FCT_COLS; c++) {
      int f = r * FCT_COLS + c;
      if (btn == button[r][c]) {
        dp->setFct(f, btn->isOn());
        Z21::LAN_X_SET_LOCO_FUNCTION(dp->getAddress(), f, dp->getFct(f));
        return;
      }
    }
  }
}

// ----------------------------------------------------------------------------------------------------
// 
void FunctionPage::locoInfoChanged(int addr, Direction dir, int fst, bool takenOver, int numSpeedSteps, bool f[])  {
  if (isVisible() && dp->getAddress()==addr) {
    int i=0;
    for (int r=0; r<FCT_ROWS; r++) 
      for (int c=0; c<FCT_COLS; c++) {
        button[r][c]->set(f[i++]);
        button[r][c]->setVisible(true); // forciert Neudarstellung
      }
  }
}