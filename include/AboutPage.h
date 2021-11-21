#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include "Page.h"

class AboutPage: public Page {

  public:
    AboutPage(char navigable);
    void setVisible(bool visible, bool clearScreen);
    void touchButtonPressed(TouchButton* button, int duration);

  public:
    TouchButton* btnRefresh;
};

#endif
