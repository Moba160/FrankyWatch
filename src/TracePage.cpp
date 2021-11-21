#include "TracePage.h"
#define LILYGO_WATCH_2020_V1
#include <LilyGoWatch.h>

extern TTGOClass* watch;

// ----------------------------------------------------------------------------------------------------
//

TracePage::TracePage(char navigable) : Page(navigable) {}

// ----------------------------------------------------------------------------------------------------
//

void TracePage::setVisible(bool visible, bool clearScreen) {

  Page::setVisible(visible, clearScreen);

  if (visible) {
    watch->tft->setTextSize(1);
    watch->tft->setTextColor(fgColor, bgColor);
    watch->tft->setCursor(0, 5);
    watch->tft->setTextWrap(true);
    navigationHint();
  }

  // Etc

  Z21::addObserver(static_cast<Page*>(this));
}

// ----------------------------------------------------------------------------------------------------
//

void TracePage::trace(bool toZ21, long timeDiff, String message, String parameter) {
  if (isVisible()) {
    watch->tft->setTextColor(toZ21 ? fgColor : TFT_YELLOW, bgColor);
    watch->tft->println((toZ21 ? "" : "   ") + message + (parameter != "" ? (": " + parameter) : ""));
  } 
}

