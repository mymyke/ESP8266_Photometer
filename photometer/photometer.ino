/*--------------------------------------------------
  Photometer Programm
  ein LED Photometer mit dem Lichtsensor TSL2561,
  einer dreifarbigen LED (RGB-LED) und
  mit eigenem WIFI AccessPoint und kleinem Webserver
  HAW Hamburg - Labor BPA - Ulrich Scheffler
  Version 0.30
  02.11.2017
  --------------------------------------------------*/
const int led_farben_max = 3; // Anzahl der LED Farben :=3 für eine RGB-LED
//const int led_farben_max = 1; // Anzahl der LED Farben :=1 für eine einfarbige LED
#include <ESP8266WiFi.h> // Bibliothek für die ESP8266 Baugruppe und seine WIFI Funktionalität
#include <Wire.h> // Bibliothek für die I2C Datenkummunikation - hier zum Datenaustausch mit dem Lichtsensor benötigt
#include <Adafruit_Sensor.h> // Basis-Bibliothek für die Adafruit Sensor Biblitheken (Bibliothek von Adafruit)
#include <Adafruit_TSL2561_U.h> // Bibliothek für den Lichtsensor TSL2661 (Bibliothek von Adafruit)
String sVersion = "Version 0.30 -";
String sVersion2 = " vom 02.11.2017 - HAW Hamburg, Lab. BPA, U. Scheffler";
const char* ssid = "HAW-PHOTOMETER-"; // der erste Teil des Namens des WIFI Access Points
const char* password = "Sciencepool"; // Auf "" (leerer String) eingestellt für einen offenen Access Point ohne Passwort
unsigned long ulReqcount; // Zähler für die Aufrufe der Webseite
int ledPin[] = {13, 12, 14}; // grüne LED an GPIO Pin 13/D7, rote LED an GPIO Pin 12/D6, blaue LED an GPIO Pin 14/D5 (hier für Lucky Light: LL-509RGBC2E-006)
const String farbkennung[] = {"Gr&uumlne LED &nbsp", "Rote LED &nbsp", "Blaue LED &nbsp", "Gr&uumlne, rote und blaue LED"}; // Texte für die RGB-Auswahl Buttons (Grün, Rot, Blau)
const int sdaPin = 4; // SDA an GPIO/Pin 4 / D2 Anschluss-Pin für das SDA-Signal zur Datenkommunikation mit dem Lichtsensor
const int sclPin = 5; // SCL an GPIO/Pin 5 / D1 Anschluss-Pin für das SCL-Signal zur Datenkommunikation mit dem Lichtsensor
const int data_runs = 7; // Anzahl der Wiederholungsmessungen aus denen dann ein Mittelwert gebildet wird
float VIS_IRZEROdata[] = {0.0, 0.0, 0.0}; // Für die aktuellen Nullproben-Messwerte vom Lichtsensor. Hier der Messwert vom Sensorteil, der im VIS und IR Bereich misst
float IRZEROdata[] = {0.0, 0.0, 0.0}; // Für die aktuellen Nullproben-Messwerte vom Lichtsensor. Hier der Messwert vom Sensorteil, der nur im IR Bereich misst
float LUXZEROdata[] = {0.0, 0.0, 0.0}; // Für die aktuellen Nullproben-Messwerte vom Lichtsensor. Hier die laut Datenblatt berechnete Beleuchtungsstärke in Lux
float VIS_IRdata[] = {0.0, 0.0, 0.0}; // Für die aktuellen Proben-Messwerte vom Lichtsensor. Hier der Messwert vom Sensorteil, der im VIS und IR Bereich misst
float IRdata[] = {0.0, 0.0, 0.0}; // Für die aktuellen Proben-Messwerte vom Lichtsensor. Hier der Messwert vom Sensorteil, der nur im IR Bereich misst
float LUXdata[] = {0.0, 0.0, 0.0}; // Für die aktuellen Proben-Messwerte vom Lichtsensor. Hier die laut Datenblatt berechnete Beleuchtungsstärke in Lux
float E_LUX[] = {0.0, 0.0, 0.0}; // Für die berechnete Extiktionen aus den Beleuchtungsstärke-Daten
float E_VIS_IR[] = {0.0, 0.0, 0.0}; // Für die berechnete Extiktionen aus den Sensor-Daten vom Sensorteil der im VIS und IR Bereich misst
float E_IR[] = {0.0, 0.0, 0.0}; // Für die berechnete Extiktionen aus den Sensor-Daten vom Sensorteil der nur im IR Bereich misst
int probenzeilenIndex = 0; // Zeiger auf die aktuell zu füllende Probenzeile
const int probenzeilenMax = 5; // Anzahl der Zeilen für Proben in der HTML Tabelle
float LUX_werte[3][probenzeilenMax]; // Array für Proben-Messwerte(Beleuchtungsstärke-Daten) vom Lichtsensor in Lux
float E_werte[3][probenzeilenMax]; // Array für berechnete Extiktionen aus den Beleuchtungsstärke-Daten
String anzeige = "a"; // Kennung für die Anzeige der Tabellenanteile a=alle, g=grün, r=rot, b=blau
bool download = false; // Flag: Wenn True Download der DatenTabelle gewünscht, wenn False dan nicht.
String datentabelle = ""; // Nimmt Datentabelle für den Download auf
const String trennzeichen = "\t"; // Spalten Trennzeichen für die Datentabelle (\t := Tabulatorzeichen)
uint16_t broadband = 0; // Sensor-Daten vom Sensorteil der im VIS und IR Bereich misst
uint16_t infrared = 0; // Sensor-Daten vom Sensorteil der nur im IR Bereich misst
WiFiServer server(80); // Eine Instanz auf dem Server für Port 80 erzeugen
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); //Objekt anlegen für den TSL2561 Sensor
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void displaySensorDetails(void) { // Zeigt einige Basisinformationen über den Sensor an
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print ("Sensor: "); Serial.println(sensor.name);
  //Serial.print ("Driver Ver: "); Serial.println(sensor.version);
  //Serial.print ("Unique ID: "); Serial.println(sensor.sensor_id);
  Serial.print ("Max Value: "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print ("Min Value: "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print ("Resolution: "); Serial.print(sensor.resolution); Serial.println(" lux");
  delay(500);
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void configureSensor(void) { // Verstärkung und Integrationszeit konfigurieren
  // tsl.setGain(TSL2561_GAIN_1X); // 1-fache Verstärkung ... bei hoher Beleuchtungsstärke, um eine Sensor-Übersättigung zu verhindern
  // tsl.setGain(TSL2561_GAIN_16X); // 16-fache Verstärkung ... bei niediger Beleuchtungsstärke, um die Sensor-Empfindlichkeit zu erhöhen
  tsl.enableAutoRange(true); // Automatische Verstärkung ... Verstärkung wechselt automatisch zwischen 1-fach und 16-fach
  // Das Ändern der Integrationszeit bewirkt eine Änderung der Sensor-Genauigkeit bzw. seiner Auflösung (402ms = 16-bit data)
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); /* 13ms: Schnell aber niedrige Auflösung */
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS); /* 101ms: Mittlere Geschwindigkeit und Auflösung */
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS); /* 402ms: Langsamste Geschwindigkeit aber hohe Auflösung (16-bit Daten)*/
  // Einstellungen zur Info ausgeben
  Serial.print ("Gain: "); Serial.println("Auto");
  Serial.print ("Timing: "); Serial.println("13 ms");
  //Serial.print ("Timing: "); Serial.println("101 ms");
  Serial.println("------------------------------------");
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
bool readSensor(int color) { // Sensor-Messdaten auslesen
  sensors_event_t event; // Ein neues TSL2561 Lichtsensor Ereigniss einrichten
  int ok = 0; // Zähler für geglückte Messungen
  float LUX[data_runs + 1]; // Daten Array zur Aufname der Einzelmessungen (ein Element mehr als Summenspeicher)
  float VIS_IR[data_runs + 1]; // Daten Array zur Aufname der Einzelmessungen
  float IR[data_runs + 1]; // Daten Array zur Aufname der Einzelmessungen
  float LUX_max = 0.0, LUX_min = 0.0;
  for (int j = 0; j <= data_runs; j++) { // Daten Arrays mit "0.0"-Werten vorbelegen
    LUX[j] = 0.0;
    VIS_IR[j] = 0.0;
    IR[j] = 0.0;
  }
  if (led_farben_max > 1) {
    digitalWrite(ledPin[color], HIGH); // LED einschalten
  }
  else {
    digitalWrite(ledPin[1], HIGH); // LED einschalten
  }
  for (int j = 0; j <= data_runs - 1; j++) { // Messungen "data_runs"-mal wiederholen und Einzelmessungen in Daten Array eintragen
    tsl.getEvent(&event); // Eine Messung durchführen
    if (event.light) { // Wird nur dann "Wahr" wenn erfolgreich gemesen wurde
      LUX[j] = (event.light * 1.0); // LUX-Wert abholen
      if (j == 0) {
        LUX_max = LUX[j]; // Min- und Max-Werte am Anfang auf den ersten Messwert setzten
        LUX_min = LUX[j];
      }
      if (LUX[j] > LUX_max) LUX_max = LUX[j]; // Neuen Max-Wert suchen und ggf neu setzten
      if (LUX[j] < LUX_min) LUX_min = LUX[j]; // Neuen Min-Wert suchen und ggf neu setzten
      tsl.getLuminosity (&broadband, &infrared); // VIS-IR- und IR-Werte auslesen
      VIS_IR[j] = (broadband * 1.0); // VIS-IR-Wert zuweisen
      IR[j] = (infrared * 1.0); // IR-Wert zuweisen
      delay(25); // Zwischen den Messungen warten (Zeit in Milli-Sekunden)
      ok += 1; // Zähler für geglückte Messungen um 1 erhöhen
    }
    else { // Wenn "event.light = 0 lux" ist, dann ist der Sensor möglicher Weise auch gesättigt und es können keinen Daten generiert werden!
      Serial.println("Der Sensor-Messwert ist unbrauchbar. -> Sensor fehler!");
    }
  }
  if (led_farben_max > 1) {
    digitalWrite(ledPin[color], LOW); // LED wieder ausschalten
  }
  else {
    digitalWrite(ledPin[1], LOW); // LED wieder ausschalten
  }
  if (ok >= data_runs) { // Nur wenn alle Einzelmessungen erfolgreich durchgeführt wurden ...
    for (int j = 0; j <= data_runs - 1; j++) { // Einzelmessungen aufaddieren
      LUX[data_runs] += LUX[j];
      VIS_IR[data_runs] += VIS_IR[j];
      IR[data_runs] += IR[j];
    }
    // Die aufaddierte Summe der Einzelwerte ohne den Max-Wert und den Min-Wert geteilt durch Anzahl der Einzelmessungen minus 2 ergibt den bereinigten Mittelwert
    LUX[data_runs] = (LUX[data_runs] - LUX_max - LUX_min) / (data_runs * 1.0 - 2.0);
    // Die aufaddierte Summe der Einzelwerte geteilt durch Anzahl der Einzelmessungen ergibt den jeweiligen Mittelwert
    VIS_IR[data_runs] = VIS_IR[data_runs] / (data_runs * 1.0);
    IR[data_runs] = IR[data_runs] / (data_runs * 1.0);
    LUXdata[color] = LUX[data_runs]; // Berechneten LUX-Wert in die Datentabelle übertragen
    VIS_IRdata[color] = VIS_IR[data_runs]; // Berechneten VIS-IR-Wert in die Datentabelle übertragen
    IRdata[color] = IR[data_runs]; // Berechneten IR-Wert in die Datentabelle übertragen
    return true;
  }
  else {
    return false;
  }
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void cleardata() {
  Serial.println("Cleardata(RESET) aufgerufen!" );
  for (int zeilennummer = 0; zeilennummer < probenzeilenMax; zeilennummer++) { // Datentabelle mit "0.0"-Werten vorbelegen
    for (int ledfarbe = 0; ledfarbe < led_farben_max; ledfarbe++) {
      LUX_werte[ledfarbe][zeilennummer] = 0.0;
      E_werte[ledfarbe][zeilennummer] = 0.0;
      LUXZEROdata[ledfarbe] = 0.0;
      VIS_IRZEROdata[ledfarbe] = 0.0;
      IRZEROdata[ledfarbe] = 0.0;
    }
  }
  probenzeilenIndex = 0;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void setup() {
  // Globale Voreinstellungen
  for (int led = 0; led < led_farben_max; led++) {
    if (led_farben_max == 1) pinMode(ledPin[1] , OUTPUT); // Wenn nur eine LED an GPIO Pin 12/D6 (für einfarbige LED), den Anschluss für die LED als Ausgang konfigurieren
    if (led_farben_max > 1) pinMode(ledPin[led], OUTPUT); // Die Anschlüsse für die LEDs als Ausgänge konfigurieren
  }
  ulReqcount = 0; // Seitenaufrufszähler auf 0 setzten
  Serial.begin(115200); // Serielle Verbindung initialisieren
  Serial.println("start");
  WiFi.mode(WIFI_AP); // WIFI Access Point Modus initialisieren
  if (led_farben_max > 1) {
    for (int led = 0; led < led_farben_max; led++) {
      digitalWrite(ledPin[led], HIGH); // LED einschalten
      delay(2000); // 2000 ms warten
      digitalWrite(ledPin[led], LOW); // LED ausschalten
    }
  }
  else {
    digitalWrite(ledPin[1], HIGH); // LED einschalten
    delay(6000); // 6000 ms warten
    digitalWrite(ledPin[1], LOW); // LED ausschalten
  }
  for (int zeilennummer = 0; zeilennummer < probenzeilenMax; zeilennummer++) { // Arrays mit "0.0"-Werten vorbelegen
    for (int ledfarbe = 0; ledfarbe < led_farben_max; ledfarbe++) {
      LUX_werte[ledfarbe][zeilennummer] = 0.0;
      E_werte[ledfarbe][zeilennummer] = 0.0;
    }
  }
  Serial.println("");
  Serial.println(String(sVersion + String (led_farben_max) + sVersion2));
  // Das Anhängsel für den Access Point Namen (SSID) aus der MAC ID des ESP Moduls generieren.
  // Um den Namen nicht zu lang werden zu lassen, werden hier nur die letzten 2 Bytes verwendet.
  // Der Aufwand dient dazu, um einen möglichst einmaligen Access Point Namen für das jeweils verwendete ESP Modul entstehten zu lassen.
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String((mac[WL_MAC_ADDR_LENGTH - 2] * 256 + mac[WL_MAC_ADDR_LENGTH - 1]), DEC);
  macID.toUpperCase();
  String AP_NameString = ssid + macID;
  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);
  for (int i = 0; i < AP_NameString.length(); i++) AP_NameChar[i] = AP_NameString.charAt(i);
  // WIFI Access Point und den Web Server starten
  WiFi.softAP(AP_NameChar, password);
  server.begin();
  Serial.print("WIFI Access Point gestartet. Name (SSID) : "); Serial.println(AP_NameChar);
  Serial.print("WEB-Server erreichbar unter der IP-Adresse): "); Serial.println(WiFi.softAPIP());
  Serial.println("");
  // Lichtsensor TSL2561
  Wire.pins(sdaPin, sclPin); // Wire.pins(int sda, int scl) // Anschlusspins für das I2C-Protokoll zuordnen
  
  if (!tsl.begin()) { // Sensor initialisieren und im Fehlerfall eine Meldung ausgeben
    Serial.println("Ooops, es konnte kein TSL2561-Sensor erkannt werden ...! (Hardware Fehler) Programm Abbruch!!! Reset nötig!!!");
    while (1);
  }
  displaySensorDetails(); // Zeigt einige Basisinformationen über den Sensor an
  configureSensor(); // Verstärkung und Integrationszeit konfigurieren
  Serial.println("");
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void loop() {
  WiFiClient client = server.available(); // Prüfen ob ein WIFI-Client verbunden ist
  if (!client) return; // Wenn keiner verbunden ist, wieder zum Anfang. Also warten bis der WIFI-Client Daten gesendet hat
  Serial.println("Neuer WIFI-Client"); // Jetzt ist ein Client verbunden ...
  unsigned long ultimeout = millis() + 250;
  while (!client.available() && (millis() < ultimeout))delay(1);
  if (millis() > ultimeout) {
    Serial.println("WIFI-Client Fehler: Connection-Time-Out!");
    return;
  }
  String sRequest = client.readStringUntil('\r'); // Die Erste Zeile der Anfrage lesen
  client.flush();
  if (sRequest == "") { // WIFI-Client stoppen, wenn die Anfrage leer ist
    Serial.println("WIFI-Client Fehler: Leere Anfrage! -> WIFI-Client angehalten");
    client.stop();
    return;
  }
  // "get path"; Das Ende des Pfads ist entweder ein " " oder ein "?" (Syntax z.B. GET /?pin=SENSOR_A HTTP/1.1)
  String sPath = "", sParam = "", sCmd = "";
  String sGetstart = "GET ";
  int iStart, iEndSpace, iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart >= 0) {
    iStart += +sGetstart.length();
    iEndSpace = sRequest.indexOf(" ", iStart);
    iEndQuest = sRequest.indexOf("?", iStart);
    // Den Pfad und die Parameter isolieren
    if (iEndSpace > 0) {
      if (iEndQuest > 0) { // Pfad und Parameter
        sPath = sRequest.substring(iStart, iEndQuest);
        sParam = sRequest.substring(iEndQuest, iEndSpace);
      }
      else { // Pfad aber keine Parameter
        sPath = sRequest.substring(iStart, iEndSpace);
      }
    }
  }
  // Den Befehl isolieren
  if (sParam.length() > 0) {
    int iEqu = sParam.indexOf("=");
    if (iEqu >= 0) {
      sCmd = sParam.substring(iEqu + 1, sParam.length());
      Serial.println(sCmd);
    }
  }
  // Die HTML Seite erzeugen
  String sResponse, sHeader;
  String sResponseStart = "";
  String sResponseTab = "";
  if (sPath != "/") { // Für unpassenden Pfad eine 404-Fehlerseite generieren
    sResponse = "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>Die angeforderte Webseite (URL) gibt es auf diesem Server nicht.</p></body></html>";
    sHeader = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n Content-Type: text/html\r\n Connection: close\r\n\r\n";
  }
  else { // Für passenden Pfad ...
    if (sCmd.length() > 0) { // Auf die Parameter reagieren ...
      Serial.print("Command: "); Serial.println(sCmd);
      if (sCmd.indexOf("READZERO") >= 0) {
        for (int color = 0; color < led_farben_max; color++) {
          if (!readSensor(color)) Serial.println("Sensor Fehler"); // Messung durchführen und Sensordaten auslesen - ggf. Fehler melden
          else {
            LUXZEROdata[color] = LUXdata[color];
            VIS_IRZEROdata[color] = VIS_IRdata[color];
            IRZEROdata[color] = IRdata[color];
          }
        }
      }
      if (sCmd.indexOf("READTSL") >= 0) { // Wenn Button "Probe" gedrückt, ...
        for (int color = 0; color < led_farben_max; color++) {
          if (!readSensor(color)) Serial.println("Sensor Fehler"); // Messung durchführen und Sensordaten auslesen - ggf. Fehler melden
          LUX_werte[color][probenzeilenIndex] = LUXdata[color];
          // Die Extinktionen berechnen
          if (LUXdata[color] > 0.0 & LUXZEROdata[color] > 0.0) {
            E_LUX[color] = -log10((LUXdata[color] * 1.0) / (LUXZEROdata[color] * 1.0));
          }
          else {
            E_LUX[color] = 0.0;
          }
          if (VIS_IRdata[color] > 0.0 & VIS_IRZEROdata[color] > 0.0) {
            E_VIS_IR[color] = -log10((VIS_IRdata[color] * 1.0) / (VIS_IRZEROdata[color] * 1.0));
          }
          else {
            E_VIS_IR[color] = 0.0;
          }
          if (IRdata[color] > 0.0 & IRZEROdata[color] > 0.0) {
            E_IR[color] = -log10((IRdata[color] * 1.0) / (IRZEROdata[color] * 1.0));
          }
          else {
            E_IR[color] = 0.0;
          }
        }
        probenzeilenIndex += 1;
        if (probenzeilenIndex >= probenzeilenMax) probenzeilenIndex = 0;
      }
      if (sCmd.indexOf("CLEARDATA") >= 0) {
        cleardata() ; // Wenn Button "Reset" gedrückt, ...
      }
      if (led_farben_max > 1) {
        if (sCmd.indexOf("GR") >= 0) {
          anzeige = "g" ; // Wenn Button "grün" gedrückt, ...
        }
        if (sCmd.indexOf("RT") >= 0) {
          anzeige = "r" ; // Wenn Button "rot" gedrückt, ...
        }
        if (sCmd.indexOf("BL") >= 0) {
          anzeige = "b" ; // Wenn Button "blau" gedrückt, ...
        }
        if (sCmd.indexOf("ALL") >= 0) {
          anzeige = "a" ; // Wenn Button "alle" gedrückt, ...
        }
      }
      if (sCmd.indexOf("DOWNLOAD") >= 0) {
        download = true; // Wenn Button "Download" gedrückt, ...
      }
    }
    // Die Extinktion mit ggf neuer Leerprobe erneut berechnen
    for (int color = 0; color < led_farben_max; color++) {
      for (int zeilennummer = 0; zeilennummer < probenzeilenMax; zeilennummer++) {
        if (LUX_werte[color][zeilennummer] > 0.0 & LUXZEROdata[color] > 0.0) {
          E_werte[color][zeilennummer] = -log10((LUX_werte[color][zeilennummer] * 1.0) / (LUXZEROdata[color] * 1.0));
        }
        else {
          E_werte[color][zeilennummer] = 0.0;
        }
      }
    }
    ulReqcount++; // Seitenaufrufzähler um 1 raufzählen
    sResponseStart = "<html><head><title>HAW Photometer</title></head><body>";
    sResponseStart += "<font color=\"#FFFFFF\"><body bgcolor=\"#003CA0\">"; // Hintergrundfarbe
    sResponseStart += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponseStart += "<p style=\"text-align: center; font-family: 'Helvetica', sans-serif; font-size: 27; font-weight:bold; \">HAW Photometer &nbsp &nbsp";
    sResponseStart += "<svg xmlns=\"http://www.w3.org/2000/svg\" xml:space=\"preserve\" width=\"27.1229mm\" height=\"7.2751mm\" viewBox=\"0 0 125324 33615\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">";
    sResponseStart += "<defs><style type=\"text/css\"><![CDATA[.fil0 {fill:white}.fnt0 {font-weight:bold;font-size:16300.1px;font-family:'Arial'}]]></style></defs>";
    sResponseStart += "<polygon class=\"fil0\" points=\"0,2484 24461,2484 24461,260 0,260 \"/>";
    sResponseStart += "<polygon class=\"fil0\" points=\"0,11378 24461,11378 24461,9154 0,9154 \"/>";
    sResponseStart += "<polygon class=\"fil0\" points=\"0,20273 24461,20273 24461,18049 0,18049 \"/>";
    sResponseStart += "<polygon class=\"fil0\" points=\"0,29168 24461,29168 24461,26945 0,26945 \"/>";
    sResponseStart += "<polygon class=\"fil0\" points=\"8896,6931 33357,6931 33357,4707 8896,4707 \"/>";
    sResponseStart += "<polygon class=\"fil0\" points=\"8896,15826 33357,15826 33357,13602 8896,13602 \"/>";
    sResponseStart += "<polygon class=\"fil0\" points=\"8896,24721 33357,24721 33357,22497 8896,22497 \"/>";
    sResponseStart += "<polygon class=\"fil0\" points=\"8896,33615 33357,33615 33357,31392 8896,31392 \"/>";
    sResponseStart += "<text x=\"41201\" y=\"11671\" class=\"fil0 fnt0\">HAW</text><text x=\"41201\" y=\"30498\" class=\"fil0 fnt0\">HAMBURG</text>";
    sResponseStart += "</svg>";
    sResponseStart += "</p>";
    sResponseStart += "<p style='text-align: center'><a href=\"?pin=READZERO\"><button style='font-size:26px'>Leerprobe</button></a>";
    sResponseStart += String("&nbsp &nbsp &nbsp &nbsp <a href=\"?pin=READTSL\"> <button style='font-size:26px'>Probe &nbsp" + String(probenzeilenIndex + 1) + "</button></a></p>");
    sResponseStart += "<p style='text-align: center'><table border='0' style=\"font-family: 'Helvetica', sans-serif; color: white; margin-left: auto; margin-right: auto; font-size: 20 \">";
    for (int color = 0; color < led_farben_max; color++) {
      if (led_farben_max > 1) {
        sResponseStart += String("<tr><td>" + farbkennung[color] + "</td><td>:</td><td>" + String(LUXdata[color]) + " lx &nbsp</td><td>" + String(E_LUX[color], 4) + "</td></tr>");
      }
      else {
        sResponseStart += String("<tr><td>" + String(LUXdata[color]) + " lx &nbsp</td><td>" + String(E_LUX[color], 4) + "</td></tr>");
      }
    }
    sResponseStart += "</table></p>";
    sResponseStart += "<p style='text-align: center'>";
    sResponseStart += "Probentabelle anzeigen f&uumlr:<BR>";
    if (led_farben_max > 1) {
      sResponseStart += String("<a href=\"?pin=GR\"><button>" + farbkennung[0] + "</button></a>");
      sResponseStart += String("<a href=\"?pin=RT\"><button>" + farbkennung[1] + "</button></a>");
      sResponseStart += String("<a href=\"?pin=BL\"><button>" + farbkennung[2] + "</button></a>");
      sResponseStart += String("<a href=\"?pin=ALL\"><button>" + farbkennung[3] + "</button></a>");
    }
    sResponseStart += "</p>";
    if (led_farben_max > 1) {
      datentabelle = String(" " + trennzeichen + "(g)" + trennzeichen + "(g)" + trennzeichen + "(r)" + trennzeichen + "(r)" + trennzeichen + "(b)" + trennzeichen + "(b)\r\n");
      datentabelle += String(" " + trennzeichen + "M" + trennzeichen + "E" + trennzeichen + "M" + trennzeichen + "E" + trennzeichen + "M" + trennzeichen + "E\r\n");
      datentabelle += String(" " + trennzeichen + "[lx]" + trennzeichen + "[-]" + trennzeichen + "[lx]" + trennzeichen + "[-]" + trennzeichen + "[lx]" + trennzeichen + "[-]\r\n");
      datentabelle += String("Leerprobe" + trennzeichen + String(LUXZEROdata[0], 2) + trennzeichen + "-" + trennzeichen + String(LUXZEROdata[1], 2) + trennzeichen + "-" + trennzeichen + String(LUXZEROdata[2], 2) + trennzeichen + "-\r\n");
      for (int i = 0; i < probenzeilenMax; i++) {
        datentabelle += ("Probe " + String(i + 1));
        datentabelle += (trennzeichen + (String(LUX_werte[0][i], 2)) + trennzeichen + (String(String(E_werte[0][i], 3))));
        datentabelle += (trennzeichen + (String(LUX_werte[1][i], 2)) + trennzeichen + (String(String(E_werte[1][i], 3))));
        datentabelle += (trennzeichen + (String(LUX_werte[2][i], 2)) + trennzeichen + (String(String(E_werte[2][i], 3))));
        datentabelle += "\r\n";
      }
    }
    else {
      datentabelle = String(" " + trennzeichen + "M" + trennzeichen + "E\r\n");
      datentabelle += String(" " + trennzeichen + "[lx]" + trennzeichen + "[-]\r\n");
      datentabelle += String("Leerprobe" + trennzeichen + String(LUXZEROdata[0], 2) + trennzeichen + "-\r\n");
      for (int i = 0; i < probenzeilenMax; i++) {
        datentabelle += ("Probe " + String(i + 1));
        datentabelle += (trennzeichen + (String(LUX_werte[0][i], 2)) + trennzeichen + (String(String(E_werte[0][i], 3))));
        datentabelle += "\r\n";
      }
    }
    Serial.println(datentabelle);
    if (led_farben_max == 1) {
      anzeige = "g"; // Wenn nur eine einfabige LED verbaut ist, wird immer nur die reduzierte Tabelle angezeigt
    }
    sResponseTab += "<table border='1' width='100%' style=\"font-family: 'Helvetica', sans-serif; color: white;\"><tr><th><a href=\"?pin=CLEARDATA\"> <button>Reset</button></a></th>";
    if (anzeige == "a" or anzeige == "g") sResponseTab += "<th>Messwert(g) [lx]</th> <th>Extinktion(g) [-]</th>";
    if (anzeige == "a" or anzeige == "r") sResponseTab += "<th>Messwert(r) [lx]</th> <th>Extinktion(r) [-]</th>";
    if (anzeige == "a" or anzeige == "b") sResponseTab += "<th>Messwert(b) [lx]</th> <th>Extinktion(b) [-]</th>";
    sResponseTab += "</tr>";
    sResponseTab += "<tr><td>Leerprobe</td>";
    if (anzeige == "a" or anzeige == "g") {
      sResponseTab += String("<td style='text-align: center'>" + String(LUXZEROdata[0]) + "</td><td></td>");
    }
    if (anzeige == "a" or anzeige == "r") {
      sResponseTab += String("<td style='text-align: center'>" + String(LUXZEROdata[1]) + "</td><td></td>");
    }
    if (anzeige == "a" or anzeige == "b") {
      sResponseTab += String("<td style='text-align: center'>" + String(LUXZEROdata[2]) + "</td><td></td>");
    }
    sResponseTab += "</tr>";
    for (int i = 0; i < probenzeilenMax; i++) {
      sResponseTab += "<tr>";
      sResponseTab += String("<td>Probe " + String(i + 1) + "</td>");
      if (anzeige == "a" or anzeige == "g") {
        sResponseTab += String("<td style='text-align: center'>" + String(LUX_werte[0][i], 2) + "</td>");
        sResponseTab += String("<td style='text-align: center'>" + String(E_werte[0][i] , 3) + "</td>");
      }
      if (anzeige == "a" or anzeige == "r") {
        sResponseTab += String("<td style='text-align: center'>" + String(LUX_werte[1][i], 2) + "</td>");
        sResponseTab += String("<td style='text-align: center'>" + String(E_werte[1][i] , 3) + "</td>");
      }
      if (anzeige == "a" or anzeige == "b") {
        sResponseTab += String("<td style='text-align: center'>" + String(LUX_werte[2][i], 2) + "</td>");
        sResponseTab += String("<td style='text-align: center'>" + String(E_werte[2][i] , 3) + "</td>");
      }
      sResponseTab += "</tr>";
    }
    sResponseTab += "</table>";
    sResponseTab += "<p style='text-align: center'><a href=\"?pin=DOWNLOAD\"><button>Download</button></a></p><BR>";
    sResponse += String("<FONT SIZE=-2>Seitenaufrufe: " + String(ulReqcount) + "<BR>"); // Aufrufzähler anzeigen
    sResponse += String(sVersion + String (led_farben_max) + sVersion2 + "<BR>"); // Versionshinweis anzeigen
    sResponse += "</body></html>";
    sHeader = "HTTP/1.1 200 OK\r\n Content-Length: ";
    sHeader += sResponse.length() + sResponseStart.length() + sResponseTab.length();
    sHeader += "\r\n Content-Type: text/html\r\n Connection: close\r\n\r\n";
  }
  /* Die Antwort an den WIFI-Client senden */
  if (download) { // Wenn der Downloadbutton gedrückt wurde, soll eine Datei gestreamt werden ...
    download = false;
    sResponse = "HTTP/1.1 200 OK\r\n";
    sResponse += "Content-Type: text/csv; charset=utf-8 \r\n";
    sResponse += "Content-Transfer-Encoding: binary \r\n";
    sResponse += "Content-Disposition: attachment; filename=\"photometer_daten.csv\" \r\n";
    sResponse += "Pragma: no-cache Expires: 0 \r\n";
    sResponse += "Content-Length:";
    sResponse += String(datentabelle.length());
    sResponse += " \r\n";
    sResponse += " Connection: close";
    sResponse += "\r\n\r\n";
    sResponse += datentabelle;
    client.print(sResponse); // Die Antwort an den WIFI-Client senden
    delay(1000); // 1000 ms warten
  }
  else { // Die HTML Seite ausgeben
    client.print(sHeader);
    client.print(sResponseStart);
    client.print(sResponseTab);
    client.print(sResponse);
  }
  // und den WIFI-Client stoppen
  client.stop();
  Serial.println("WIFI-Client getrennt");
}
