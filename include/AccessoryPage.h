#ifndef ACCESSORYPAGE_H
#define ACCESSORYPAGE_H

#include "Page.h"
#include "TouchButton.h"

// Anzahl der Adressen gleich auf Zeile*Spalte für Darstellung aufteilen
#define ADDR_ROWS 5
#define ADDR_COLS 5

#define NumAccessory 75

class AccessoryPage : public Page {

  public:
    AccessoryPage(int from, char navigable);
    void setVisible(bool visible, bool clearScreen);
    
    virtual void accessoryStateChanged(int addr, bool plus) override;

    void touchButtonPressed(TouchButton* button, int duration);

  private:
    TouchButton* button[ADDR_ROWS][ADDR_COLS];   
    int from;
  
};

#endif
