#ifndef TOUCH_BUTTON_H
#define TOUCH_BUTTON_H

#define NUM_BUTTONS 200

#include <Arduino.h>

class TouchButton {

  friend class Page;
  friend class DrivePage; // muss weg!

  public:

    enum Position { LeftTo, RightTo, Above, Below,                  // relativ zu anderem Button
                    TopLeft, TopCentered, TopRight,                 // Obere Displayreihe
                    CenteredLeft, CenteredCentered, CenteredRight,  // Mittlere Displayreihe
                    BottomLeft, BottomCentered, ButtomRight         // Untere Displayreihe
                  };

    enum Gesture { Left, Right, Up, Down };

    enum Style { Rectangle, RoundedRectangle, Circle, Invisible };

    enum Type { Push, Toggle, Slider };

    static void begin(int spacing=5);

    static void loop();

    TouchButton(Style style, int fgColor, int bgColor, Type type, String label, int width, int height, Position position, TouchButton* otherButton = NULL);
    TouchButton(Style style, int fgColor, int bgColor, Type type, String label, int width, int height, int x, int y);

    void setVisible(bool visible);
    bool isVisible() { return visible; }

    void setToggleColors(int unC, int sC) { unselectedColor = unC; selectedColor = sC; }

    String getLabel() { return label; }

    void toggle() { on = !on; }
    void set(boolean on) { this->on = on; }
    boolean isOn() { return on; }

  private:
    Type type;
    int fgColor, bgColor;
    int unselectedColor, selectedColor;
    bool visible = false;
    bool toggleable;
    bool on = false;
    static int spacing;
    String label;
    int x, y, xc, yc;
    int height, width;
    Style style;

    static TouchButton* button[NUM_BUTTONS];

};

#endif
