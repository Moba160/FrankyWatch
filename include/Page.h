#ifndef PAGE_H
#define PAGE_H

/*
  Abstrakte Basisklasse der GUI
  Die GUI besteht aus einem Array aus Seiten, die von Page abgeleitet werden.
  Mit Wischgesten bewegt man sich von einer Seite zur anderen (zwischen den
  Elementen des Arrays)
*/

#define LILYGO_WATCH_2020_V1
#include <LilyGoWatch.h>

#include "TouchButton.h"
#include <Z21Observer.h>
#include <Z21.h>

// Dimension des Seiten-Arrays, es gibt also (maximal) PAGE_ROWS x PAGE_COLS Seiten,
// wobei nicht zu besetzende Plätze im Array mit 0 gekennzeichnet werden
#define PAGE_ROWS 5
#define PAGE_COLS 3

// In welche Richtung darf von einer Seite aus navigiert (gewischt) werden?
// Es ist nicht immer erwünscht, von jeder Seite zu jeder zu kommen
#define NAV_LEFT  0b1000
#define NAV_RIGHT 0b1001
#define NAV_UP    0b0100
#define NAV_DOWN  0b0010
#define NAV_ALL NAV_LEFT | NAV_RIGHT | NAV_UP | NAV_DOWN

class Page : public Z21Observer {

    friend class TouchButton;

  public:

    Page(char navigable) : navigable{navigable} {}

    static void begin(TTGOClass*);
    static void setBlocked(boolean blocked) { Page::blocked = blocked; }
    static bool isBlocked() { return blocked; }
    static void loop();

    virtual void trackPowerStateChanged(BoolState trackPowerState) override;
    virtual void shortCircuitStateChanged(BoolState shortCircuitState) override;
    virtual void emergencyStopStateChanged(BoolState emergencyStopState) override;
    virtual void progStateChanged(BoolState progState) override;
    virtual void progResult(ProgResult result, int value) override {};
    virtual void traceEvent(FromToZ21 direction, long diffLastSentReceived, String message, String parameters) override {};

    virtual void setVisible(bool visible, bool clearScreen); // müsste protected sein
    virtual void navigationHint();
    boolean isVisible() { return visible; };
    
    static Page* currentPage();

    static int getBgColor() { return bgColor; }

  protected:
    static TFT_eSPI* tft;
    static BMA* bmaSensor;
    static int bgColor, bgColorOn;
    static int fgColor;
    boolean visible = false;
    static bool blocked;

    // benachbarte Seiten
    static Page* navigationGrid[PAGE_ROWS][PAGE_COLS];

    virtual void touchButtonPressed(TouchButton* button, int duration) {};

    static void swipeGesture(TouchButton::Gesture gesture, int distance);


    static int row, col;

  private:
    char navigable;

};

#endif
