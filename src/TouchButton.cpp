#include "TouchButton.h"
#include "Page.h"

#define LILYGO_WATCH_2020_V1
#include <LilyGoWatch.h>

TouchButton* TouchButton::button[NUM_BUTTONS];
int TouchButton::spacing;

#define MAXHIST 50
#define SAMPLING_RATE 5
#define MIN_SWIPE 50
static int16_t histX[MAXHIST], histY[MAXHIST];

// ----------------------------------------------------------------------------------------------------
//

void TouchButton::begin(int spacing) {
  TouchButton::spacing = spacing;
  for (int i = 0; i < NUM_BUTTONS; i++) button[i] = NULL;
}

// ----------------------------------------------------------------------------------------------------
//

TouchButton::TouchButton(Style style, int fgColor, int bgColor, Type type, String label, int width, int height, int x, int y) : TouchButton(style, fgColor, bgColor, type, label, width, height, CenteredCentered) {
  xc = x; yc = y;
  this->x = x - width / 2; this->y = y - height / 2;
}


TouchButton::TouchButton(Style style, int fgColor, int bgColor, Type type, String label, int width, int height, Position position, TouchButton* otherButton)
  : type{type},
    fgColor{fgColor},
    bgColor{bgColor},
    label{label},
    height{height},
    width{width},
    style{style} {

  int i;
  for (i = 0; i < NUM_BUTTONS; i++) {
    if (button[i] == NULL) {
      button[i] = this;
      break;
    }
  }
  
  //Serial.printf("%s: %d Buttons von %d verwendet\n", i >= NUM_BUTTONS ? "Zu wenig: " : "Ausreichend", i, NUM_BUTTONS);

  toggleable = type == Toggle;

  switch (position) {
    case TopLeft:
      x = spacing; y = spacing;
      break;
    case TopCentered:
      x = (TFT_WIDTH - width) / 2; y = spacing;
      break;
    case TopRight:
      x = TFT_WIDTH - width - spacing; y = spacing;
      break;
    case BottomCentered:
      x = (TFT_WIDTH - width) / 2; y = TFT_HEIGHT - height - spacing;
      break;
    case Below:
      x = otherButton -> x + otherButton -> width / 2 - width / 2; y = otherButton -> y + otherButton -> height + spacing;
      break;
    case LeftTo:
      x = otherButton -> x - width - spacing; y = otherButton -> y;
      break;
    case RightTo:
      x = otherButton -> x + otherButton -> width + spacing; y = otherButton -> y;
      break;
  }

  xc = x + width / 2; yc = y + height / 2;
  unselectedColor = fgColor; selectedColor = bgColor;

}

// ----------------------------------------------------------------------------------------------------
//

void TouchButton::setVisible(bool visible) {

  this->visible = visible;

  switch (style) {
    case RoundedRectangle:
    case Rectangle:
      Page::tft->fillRoundRect(x, y, width, height, Rectangle ? 0 : 3, Page::bgColor);
      if (visible) {
        if (!toggleable) Page::tft->drawRoundRect(x, y, width, height, Rectangle ? 0 : 3, bgColor);
        if (toggleable &&  on) Page::tft->fillRoundRect(x, y, width, height, Rectangle ? 0 : 3, selectedColor);
        if (toggleable && !on) {
          Page::tft->fillRoundRect(x, y, width, height, Rectangle ? 0 : 3, unselectedColor);
          if (unselectedColor == Page::currentPage()->getBgColor()) Page::tft->drawRoundRect(x, y, width, height, Rectangle ? 0 : 3, selectedColor);
        }
      }
      break;
    case Circle:
      Page::tft->fillCircle(xc, yc, width / 2, Page::bgColor);
      if (visible) {
        if (!toggleable || (toggleable && on)) { // Fläche: Normaler Button oder aktivierter toggleablebutton
          Page::tft->fillCircle(xc, yc, width / 2, bgColor);
        } else {                         // Rahmen: Deaktivierter toggleablebutton
          Page::tft->drawCircle(xc, yc, width / 2, bgColor);
        }
      }
      break;
  }

  if (visible) {
    Page::tft->setTextDatum(MC_DATUM);
    Page::tft->setTextColor(!toggleable || (toggleable && !on) ? bgColor : fgColor, !toggleable || (toggleable && !on) ? bgColor : Page::fgColor);
    Page::tft->drawString(label, xc, yc, 4);
  }
}

// ----------------------------------------------------------------------------------------------------
//

extern TTGOClass* watch;


void TouchButton::loop() {

  int16_t xP, yP;
  static int16_t x, y;
  static bool pressed = false;
  static long pressedAt = millis();
  static int h = 0;

  if (watch->getTouch(xP, yP)) {
    if (!pressed) {
      pressed = true;
      pressedAt = millis();
      x = xP; y = yP;
    } else {
      histX[h] = xP; histY[h] = yP;
      h = (millis() - pressedAt) / SAMPLING_RATE;
      if (h != 0 && h >= MAXHIST) h = MAXHIST - 1;
      return;
    }
  } else {
    if (pressed) { // losgelassen
      int deltaX = histX[h - 2] - histX[0];
      int deltaY = histY[h - 2] - histY[0];

      //Serial.printf("X[%d]: ", h); for (int i = 0; i < MAXHIST; i++) Serial.printf("%d ", histX[i]); Serial.println();
      //Serial.printf("Y[%d]: ", h); for (int i = 0; i < MAXHIST; i++) Serial.printf("%d ", histY[i]); Serial.println();

      //Serial.printf("%d %d\n", deltaX, deltaY);
      //Serial.printf("%s %s\n", deltaX>0 ? "rechts" : "links", deltaY>0 ? "unten": "oben");
      //for (int i = 0; i < MAXHIST; i++) histX[i] = histY[i] = MAX_LONG; h = 0;
      if (abs(deltaX) > MIN_SWIPE || abs(deltaY) > MIN_SWIPE) {
        // Swipe-Geste wurde erkannt -------------------------------------------------------
        if (abs(deltaX) >= abs(deltaY)) {
          if (deltaX > 0) Page::swipeGesture(Right, deltaX); else Page::swipeGesture(Left, abs(deltaX));
        } else {
          if (deltaY > 0) Page::swipeGesture(Down, deltaY); else Page::swipeGesture(Up, abs(deltaY));
        }
        pressed = false; return;
      }
      pressed = false;
      for (int i = 0; i < NUM_BUTTONS; i++) {
        if (button[i] == NULL) continue;
        if (!(button[i]->isVisible())) continue;
        // Serial.printf("x: %d, y: %d\n", x, y);
        if (x >= button[i]->x && x <= button[i]->x + button[i]->width && y >= button[i]->y && y < button[i]->y + button[i]->height) {
          // Touchbutton wurde gedrückt ----------------------------------------------------
          if (button[i] -> toggleable) button[i]->toggle();
          Page::currentPage() -> touchButtonPressed(button[i], millis() - pressedAt);
          button[i]->setVisible(true);
        }
      }
    }
  }
}
