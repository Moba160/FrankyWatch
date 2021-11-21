#ifndef PREF_H
#define PREF_H

/*
  Verwaltung von Präferenzen. Statt direkt das ESP32-Preferences-Interface zu verwenden,
  wird hier bei set noch zusätzlich geprüft, ob nicht der schon gespeicherte Wert geschrieben
  wird (und dann nicht ausgeführt, um SRAM-Lebenszeit zu sparen)
*/

#include <Preferences.h>

#define prefNameSSID "ssid"
#define prefNamePasswd "passwd"
#define prefNameZ21Addr "z21addr"
#define prefNameAutoAddress "autoAddress"
#define prefNameLink "link"
#define prefNameLocoChannelAddr "chAddr"

class Pref {

  public:

    static void begin();
    static void init(String key, String defValue);
	  static void purge() { prefs.clear(); };
    
    static void set(String key, String value);
    static String get(String key);
	  static String get(String key, String defValue);

  private:
    
    static Preferences prefs;

};

#endif
