#include "ConfigPage.h"
#include "DrivePage.h"
#include "Pref.h"

// ----------------------------------------------------------------------------------------------------
//

#define btnHeigt 40
#define btnWidth 200

ConfigPage::ConfigPage(char navigable) : Page(navigable) {

  btnStop = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_RED, TouchButton::Toggle, "Notaus", btnWidth, btnHeigt, TFT_WIDTH/2, TFT_HEIGHT*0.45);
  btnAutoAddress = new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "Letzte Adresse", btnWidth, btnHeigt, TouchButton::Below, btnStop);
  btnLink =        new TouchButton(TouchButton::Rectangle, TFT_WHITE, TFT_BLUE, TouchButton::Toggle, "Mitlauschen",    btnWidth, btnHeigt, TouchButton::Below, btnAutoAddress);

  btnStop->setToggleColors(TFT_BLACK, TFT_RED);
  btnAutoAddress->setToggleColors(TFT_BLACK, TFT_BLUE);
  btnLink->setToggleColors(TFT_BLACK, TFT_BLUE);

  // Etc

  Z21::addObserver(static_cast<Page*>(this));
}

void ConfigPage::setVisible(bool visible, bool clearScreen) {
  
  Page::setVisible(visible, clearScreen);

  if (visible) {
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_WHITE);
    tft->drawString("Z21-Adresse:", TFT_WIDTH / 2, TFT_HEIGHT * 0.10, 2);
    tft->drawString(Pref::get(prefNameZ21Addr), TFT_WIDTH / 2, TFT_HEIGHT * 0.20, 4);
    String xbusfwVersion = (Z21::xbusVersion == yetUnknown || Z21::fwVersion == yetUnknown) 
    ? "" : (Z21::xbusVersion + "/" + Z21::fwVersion);
    String hw = Z21::hwVersion == yetUnknown ? "" : Z21::hwVersion;
    tft->drawString(xbusfwVersion + " " + hw, TFT_WIDTH/2, TFT_HEIGHT*0.30, 2);
  }

  btnStop->set(Z21::getTrackPowerState() == BoolState::Off);
  btnStop->setVisible(visible); 

  btnAutoAddress->set(Pref::get(prefNameAutoAddress).toInt());
  btnAutoAddress->setVisible(visible);

  btnLink->set(Pref::get(prefNameLink).toInt());
  btnLink->setVisible(visible);

  if (visible) navigationHint(); 
}

// ----------------------------------------------------------------------------------------------------
//

void ConfigPage::touchButtonPressed(TouchButton* button, int duration) {

  if (button == btnStop) {
    // Button aktiv bedeutet Nothalt ist aktiv (track power off)
    Z21::LAN_X_SET_TRACK_POWER(!(btnStop->isOn()));
    return;
  }

  if (button == btnAutoAddress) {
    Pref::set(prefNameAutoAddress, String(btnAutoAddress->isOn()));
    return;
  }

  if (button == btnLink) {
    Pref::set(prefNameLink, String(btnLink->isOn()));
    DrivePage::setLinkLoco(btnLink->isOn());
    return;
  }

}

// ----------------------------------------------------------------------------------------------------
// Neuzeichnen des Buttons, wenn Notifikation über Zustandsänderung eintrifft

void ConfigPage::trackPowerStateChanged(BoolState trackPowerState) {
  btnStop->set(trackPowerState != BoolState::On);
  if (Page::currentPage() == this) btnStop->setVisible(true); 
};
