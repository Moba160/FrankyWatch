#ifndef DRIVEPAGE_H
#define DRIVEPAGE_H

#include "Page.h"
#include "TouchButton.h"
#include "Pref.h"

#define NumDeltaButtonsSpeed 3
#define NumDeltaButtonsAddress 5
#define NumDrivePages 3

class DrivePage : public Page {

  public:
    DrivePage(char channel, char navigable);
    void setVisible(bool visible, bool force);
    void touchButtonPressed(TouchButton* button, int duration);
    static DrivePage* pages[NumDrivePages];

    virtual void locoInfoChanged(int addr, Direction dir, int fst, bool takenOver, int numSpeedSteps, bool f[]) override;

    void setAddress(int addr) { locoAddress = addr; }
    int getAddress() { return locoAddress; }
    bool getFct(int i) { return fct[i]; }
    void setFct(int i, bool val) { fct[i] = val; }

    static void setLinkLoco(bool linkLoco) { DrivePage::linkLoco = linkLoco; }

    void incrLocoAddress(int delta) { locoAddress += delta; }

    int locoSpeed = 0;
    Direction direction = Direction::Forward;
    int headlights = 0;

    int displayedLocoSpeed = 0;
    int displayedDirection = Direction::Forward;
    int displayedHeadlights = 0;
    int displayedLocoAddress;

  private:
    void drawLocoAddress(boolean force);
    void drawLocoSpeed(boolean force);
    void drawMode(boolean force);
    void drawDirection(boolean force);
    void drawHeadlights(boolean force);

    enum Mode { Speed, Address };
    Mode mode = Speed;
    char channel;

    static bool linkLoco;  // Mitlauschen

    int locoAddress;

    TouchButton* btnDirection;
    TouchButton* btnHeadlights;
    TouchButton* btnVPlus;
    TouchButton* btnVMinus;
    TouchButton* btnAddressMode;
    TouchButton* btnSpeedMode;
    
    TouchButton* btnDeltaSpeed[NumDeltaButtonsSpeed];
    TouchButton* btnDeltaAddress[NumDeltaButtonsAddress];
    int idxDeltaButtonSpeed = 0;
    int idxDeltaButtonAddress = 0;

    bool fct[MaxFct];

    static int lastPage;
  
};

#endif
