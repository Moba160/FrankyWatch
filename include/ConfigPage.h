#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include "Page.h"
#include <Z21.h>

class ConfigPage: public Page {

  public:
    ConfigPage(char navigable);
    void setVisible(bool visible, bool clearScreen);
    void touchButtonPressed(TouchButton* button, int duration);

    virtual void trackPowerStateChanged(BoolState trackPowerState) override;

  public:
    TouchButton* btnStop;
    TouchButton* btnAutoAddress;
    TouchButton* btnLink;

};

#endif
