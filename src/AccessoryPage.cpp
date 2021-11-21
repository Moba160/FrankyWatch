#include "AccessoryPage.h"
#include "Pref.h"
#include <Z21.h>

// ----------------------------------------------------------------------------------------------------
//

#define BTN_SIZE 42

AccessoryPage::AccessoryPage(int from, char navigable) : Page(navigable), from{from} {

  for (int r = 0; r < ADDR_ROWS; r++) {
    for (int c = 0; c < ADDR_COLS; c++) {
      int i = from + r * ADDR_COLS + c;
      if (r == 0 && c == 0) {
        button[r][c] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_PURPLE, TouchButton::Toggle, String(i), BTN_SIZE, BTN_SIZE, TouchButton::TopLeft);
      } else if (r != 0 && c == 0) {
        button[r][c] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_PURPLE, TouchButton::Toggle, String(i), BTN_SIZE, BTN_SIZE, TouchButton::Below, button[r - 1][c]);
      } else {
        button[r][c] = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_PURPLE, TouchButton::Toggle, String(i), BTN_SIZE, BTN_SIZE, TouchButton::RightTo, button[r][c - 1]);
      }
      if (Pref::get("sig" + String(i)) == "on") button[r][c]->setToggleColors(TFT_RED, TFT_DARKGREEN);
      else button[r][c]->setToggleColors(TFT_BLACK, TFT_PURPLE);
    }
  }

    // Etc

  Z21::addObserver(static_cast<Page*>(this));
}

// ----------------------------------------------------------------------------------------------------
//

void AccessoryPage::setVisible(bool visible, bool clearScreen) {

  Page::setVisible(visible, clearScreen);

  for (int r = 0; r < ADDR_ROWS; r++) {
    for (int c = 0; c < ADDR_COLS; c++) {
      button[r][c] -> setVisible(visible);
    }
  }

  if (visible) navigationHint();
}

// ----------------------------------------------------------------------------------------------------
//
void AccessoryPage::touchButtonPressed(TouchButton* btn, int duration) {
  for (int r = 0; r < ADDR_ROWS; r++) {
    for (int c = 0; c < ADDR_COLS; c++) {
      if (btn == button[r][c]) {
        Serial.printf("Schalte %d auf %s\n", btn->getLabel().toInt(), btn->isOn() ? "Minus" : "Plus");
        Z21::LAN_X_SET_TURNOUT(btn->getLabel().toInt(), !(btn->isOn()));
        return;
      }
    }
  }
}

void AccessoryPage::accessoryStateChanged(int addr, bool plus) {

  TouchButton* btn;
  for (int r = 0; r < ADDR_ROWS; r++) {
    for (int c = 0; c < ADDR_COLS; c++) {
      btn = button[r][c];
      if (btn->getLabel().toInt() == addr) {
        btn->set(!plus); // true = plus soll nicht markiert/aktiv dargestellt werden
        if (Page::currentPage() == this) btn->setVisible(true);
        return;
      }
    } 
  }
}

