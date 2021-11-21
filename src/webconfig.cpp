/*
    Zunächst Angebot eines Access Point, damit man sich über hier definierte
    Webseite auf das so aufgespannte eigene WLAN-Netz aufschalten kann und
    darüber die Zugangsdaten (und andere Konfigurationsdaten) für das später
    eigentlich zu verwendende WLAN-Netz eingeben kann

    https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/issues/82

*/

#include <LilyGoWatch.h>
#include <WiFi.h>
#include <AccessoryPage.h>
#include <ESPAsyncWebServer.h>
#include "configuration.h"
#include "Pref.h"
#include "Page.h"

extern TTGOClass* watch;
AsyncWebServer webServer(80);

// ----------------------------------------------------------------------------------------------------
//

void webconfig() {

    WiFi.mode(WIFI_MODE_APSTA); // AP = Access Point, STA = Station Mode
    WiFi.setHostname(HOST_NAME); // Leichter auffindbar im Router, bedeutet allerdings
                                 // auch, dass nur eine FrankyWatch im Netz sein kann

    // watch->touch->setMonitorPeriod(250);
    boolean inAPMode = true;

    String ssid = Pref::get(prefNameSSID, "Erdgeschoss24");
    String passwd = Pref::get(prefNamePasswd, "FrankUndUte");
    Serial.printf("Gelesen SSID: |%s|, passwd: |%s|\n", ssid.c_str(), passwd.c_str());

    // WLAN-Daten wurden bereits eingegeben (und gespeichert)
    if (ssid != "" && passwd != "") {

        inAPMode = false;

        // WLAN-Bildschirm mit Countdown
        WiFi.begin(ssid.c_str(), passwd.c_str());

        watch->tft->fillScreen(TFT_BLUE);
        watch->tft->setTextColor(TFT_WHITE);
        watch->tft->setTextDatum(CC_DATUM);
        watch->tft->drawString("Verbinde mit WLAN SSID", TFT_WIDTH / 2, TFT_HEIGHT * 0.30, 2);
        watch->tft->drawString(ssid, TFT_WIDTH / 2, TFT_HEIGHT * 0.50, 4);

        int connCount = 50; int pixels=30;
        while (WiFi.status() != WL_CONNECTED && connCount > 0) {
            watch->tft->fillRect(0, TFT_HEIGHT * 0.80 - pixels/2, TFT_WIDTH, pixels, TFT_BLUE);
            watch->tft->drawString(String(connCount--), TFT_WIDTH / 2, TFT_HEIGHT * 0.80, 4);
            delay(500);
        }

        webServer.begin();

        if (connCount != 0) {
            watch->tft->fillRect(0, TFT_HEIGHT * 0.80 - pixels/2, TFT_WIDTH, pixels, TFT_BLUE);
            watch->tft->drawString(WiFi.localIP().toString(), TFT_WIDTH / 2, TFT_HEIGHT * 0.80, 4);
            Page::setBlocked(false);
        } else {
            inAPMode = true;
        }
    }
    
    // Keine WLAN-Daten (beim allerersten Start), Access Point einrichten
    if (inAPMode) {

        // AP-Bildschirm, um auf zu verwendente Adresse hinzuweisen
        WiFi.softAP("FrankyWatch", "", 13); // 13=Channel, ohne (default 1) wird AP manchmal nicht aufgespannt
        Serial.printf("AP auf Adresse %s\n",  WiFi.softAPIP().toString().c_str());
        watch->tft->fillScreen(TFT_BLUE);
        watch->tft->setTextColor(TFT_WHITE);
        watch->tft->setTextDatum(CC_DATUM);
        watch->tft->drawString("1", TFT_WIDTH / 2, TFT_HEIGHT * 0.20, 8);
        watch->tft->drawString("Verbinden Sie sich mit dem WLAN", TFT_WIDTH / 2, TFT_HEIGHT * 0.50, 2);
        watch->tft->setTextColor(TFT_YELLOW);
        watch->tft->drawString(HOST_NAME, TFT_WIDTH / 2, TFT_HEIGHT * 0.62, 4);
        watch->tft->setTextColor(TFT_WHITE);
        watch->tft->drawString("(kein Passwort erforderlich)", TFT_WIDTH / 2, TFT_HEIGHT * 0.70, 2);
        char buffer[50];
        sprintf(buffer, "Eigene IP: %s", WiFi.softAPIP().toString().c_str());
        watch->tft->setTextColor(TFT_YELLOW);
        watch->tft->drawString(buffer, TFT_WIDTH / 2, TFT_HEIGHT * 0.90, 2);
        WiFi.begin();
        Page::setBlocked(true);
    }

    // Formular für Eingabe von SSID/Passwort und weiterer Konfigurationsdaten
    webServer.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
        watch->tft->fillScreen(TFT_BLUE);
        watch->tft->setTextColor(TFT_WHITE);
        watch->tft->setTextDatum(CC_DATUM);
        watch->tft->drawString("2", TFT_WIDTH / 2, TFT_HEIGHT * 0.20, 8);
        watch->tft->drawString("Client hat sich verbunden - ", TFT_WIDTH / 2, TFT_HEIGHT * 0.70, 2);
        watch->tft->drawString("dort die Konfiguration vornehmen", TFT_WIDTH / 2, TFT_HEIGHT * 0.80, 2);
        String addr=""; String id;
        for (int i=0; i<NumAccessory; i++) {
            id = "sig" + String(i+1);
            addr += "<input type=\"checkbox\" name=\"" + id + "\"" +
            String(Pref::get(id, "") == "on" ? " checked" : "") +
            ">" + String(i+1) + "&nbsp;";
        }
        request->send(200, "text/html",
        "<style>"
        "html { font-family: Helvetica; }"
        "body { background-color: #0000FF; }"
        "td { padding:10px; }"
        "</style>"
        "<h1 style=\"color:#FFFFFF\"><font size=+4>2 </font>" PRODUCT_NAME " konfigurieren</h1>"
        "<font size+=2>"
        "<form action=\"/get\">"
        "<table border=0 style=\"color:#FFFFFF\">"
        "<tr><td>SSID</td><td><input type=\"text\" name=\"ssid\" value=\"" + Pref::get(prefNameSSID) + "\"></td></tr>"
        "<tr><td>Password</td><td><input type=\"text\" name=\"password\" value=\"" + Pref::get(prefNamePasswd) + "\"></td></tr>"
        "<tr><td>Z21-Adresse</td><td><input type=\"text\" name=\"z21addr\" value=\"" + Pref::get(prefNameZ21Addr, Z21_DEFAULT_ADDR) + "\"></td></tr>"
        "<tr><td colspan=2>Welche Zubeh&ouml;radressen sind Signale?<br/>" + addr +
        "</td></tr>"
        "<tr><td><input type=\"submit\" value=\"Speichern\"></td><td></td></tr>"
        "</table>"
        "<p/>"
        "</form>"
        "</font>"
        );
    });
  
    // Daraufhin zurückgelieferte ausgefüllte Daten des Formulars
    webServer.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {

    String ssid, passwd, z21addr;
    bool credentialsChanged = false;
    if (request->hasParam("ssid")) {
        ssid = request->getParam("ssid")->value();
        Serial.printf("SSID: %s\n", ssid.c_str());
        if (Pref::get(prefNameSSID) != ssid) {
            Pref::set(prefNameSSID, ssid);
            credentialsChanged = true;
        }
    } 
    if (request->hasParam("password")) {
        passwd = request->getParam("password")->value();
        Serial.printf("password: %s\n", passwd.c_str());
        if (Pref::get(prefNamePasswd) != passwd) {
            Pref::set(prefNamePasswd, passwd);
            credentialsChanged = true;
        }
    } 
    if (request->hasParam("z21addr")) {
        z21addr = request->getParam("z21addr")->value();
        Serial.printf("z21addr: %s\n", z21addr.c_str());
        Pref::set(prefNameZ21Addr, z21addr);
    } 
    for (int i=0; i<NumAccessory; i++) {
        String id="sig" + String(i+1);
        Pref::set(id, request->hasParam(id) ? "on" : "off");
    }

    watch->tft->fillScreen(TFT_BLUE);
    watch->tft->setTextColor(TFT_WHITE);
    watch->tft->setTextDatum(CC_DATUM);
    watch->tft->drawString("3", TFT_WIDTH / 2, TFT_HEIGHT * 0.20, 8);
    watch->tft->drawString("Konfigurationsdaten erhalten" + String(credentialsChanged ? " - Neustart" : ""), 
        TFT_WIDTH / 2, TFT_HEIGHT * 0.80, 2);
 
    String message = "Daten empfangen. "
        + String(credentialsChanged ? "Neustart, da sich WLAN-Daten ge&auml;ndert haben." : "");
    request->send(200, "text/html", 
        "<style>"
        "html { font-family: Helvetica; }"
        "body { background-color: #0000FF; color: #FFFFFF; }"
        "</style>"
        "<body>"
        "<font size=+2>" + message + "</font>"
        "</body>"
        );
        if (credentialsChanged) ESP.restart();
    });

}