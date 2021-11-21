#ifndef FUNCTIONPAGE_H
#define FUNCTIONPAGE_H

#include "Page.h"
#include "DrivePage.h"
#include "TouchButton.h"

// Anzahl der Funktionen gleich auf Zeile*Spalte für Darstellung aufteilen
#define FCT_ROWS 5
#define FCT_COLS 5

class FunctionPage : public Page {

  public:
    FunctionPage(DrivePage* dp, char navigable);
    void setVisible(bool visible, bool clearScreen);
    void touchButtonPressed(TouchButton* button, int duration);

    virtual void locoInfoChanged(int addr, Direction dir, int fst, bool takenOver, int numSpeedSteps, bool f[]) override;
  
  private:
    TouchButton* button[FCT_ROWS][FCT_COLS];   
    DrivePage* dp;
  
};

#endif
