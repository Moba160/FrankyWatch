#include "AboutPage.h"
#define LILYGO_WATCH_2020_V1
#include <LilyGoWatch.h>
AXP20X_Class *power;
#include <WiFi.h>
#include "Pref.h"

// ----------------------------------------------------------------------------------------------------
//

#define btnHeigt 40
#define btnWidth 200

AboutPage::AboutPage(char navigable) : Page(navigable) {
  btnRefresh = new TouchButton(TouchButton::Rectangle, TFT_BLUE, TFT_BLUE, TouchButton::Push, "Aktualisieren",    btnWidth, btnHeigt, TouchButton::BottomCentered);

    // Etc

  Z21::addObserver(static_cast<Page*>(this));
}

void AboutPage::setVisible(bool visible, bool clearScreen) {
  
  Page::setVisible(visible, clearScreen);

  if (visible) {
    power = TTGOClass::getWatch()->power;

    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->drawString("Eigene Adressen:", TFT_WIDTH / 2, TFT_HEIGHT * 0.08, 2);
    tft->drawString(WiFi.softAPIP().toString(), TFT_WIDTH / 2, TFT_HEIGHT * 0.18, 4);
    tft->drawString(WiFi.localIP().toString(), TFT_WIDTH / 2, TFT_HEIGHT * 0.28, 4);

    char buf[50];
 
    if (power->isChargeing()) {
      tft->drawString("Akku wird aufgeladen", TFT_WIDTH / 2, TFT_HEIGHT * 0.43, 2);
      sprintf(buf, "%1.1fV/%dmA %d%%", 
        power->getBattVoltage()/1000, 
        (int)(power->getBattChargeCurrent()), 
        power->getBattPercentage());
      tft->drawString(buf, TFT_WIDTH / 2, TFT_HEIGHT * 0.53, 4);
    } else {
     
     tft->drawString("Akku wird entladen", TFT_WIDTH / 2, TFT_HEIGHT * 0.43, 2);
      sprintf(buf, "%1.1fV", 
        power->getBattVoltage()/1000);
        // getBattPercentage() liefert 127% :-(
        // float getBattDischargeCurrent() liefert 0
        tft->drawString(buf, TFT_WIDTH / 2, TFT_HEIGHT * 0.53, 4);
    }

    sprintf(buf, "Laufzeit seit Einschalten: %dmin", millis()/1000/60);
    tft->drawString(buf, TFT_WIDTH / 2, TFT_HEIGHT * 0.68, 2);    

  }

  btnRefresh->setVisible(visible);

  if (visible) navigationHint();
}

// ----------------------------------------------------------------------------------------------------
//

void AboutPage::touchButtonPressed(TouchButton* button, int duration) {
    if (button == btnRefresh) {
      setVisible(true, false);
    }
}

