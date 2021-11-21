#ifndef TRACEPAGE_H
#define TRACEPAGE_H

#include "Page.h"
#include "Z21.h"

class TracePage : public Page {

  public:
    TracePage(char navigable);
    void setVisible(bool visible, bool clearScreen);
    void trace(bool toZ21, long timeDiff, String message, String parameter);
  
};

#endif
