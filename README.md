# ESP8266_Photometer
Photometerprojekt für das Esp8266 mit dem tsl2561 Sensor

Projekt ursprünglich von:
https://www.haw-hamburg.de/fileadmin/user_upload/FakLS/08LABORE/BPA/WIFI_Fotometer_20181030_DE.pdf?fbclid=IwAR3fcKV85bLY09iHQA-1UmcZagogU0tTfPw5Inxcci2pbUYMri8uTsYmVGM


HOWTO:
1) Open Arduino IDE
2) Open File-->Preferences
3) Copy "http://arduino.esp8266.com/stable/package_esp8266com_index.json" into "Additional Board Manager URLs"
4) Open the Sketchbook location folder (see Preferences)
5) Download the files from this Project (via github) into the sketchbook location folder
6) Press File-->Open and open the photometer.ino file from "sketch location folder"/photometer/.
7) Under Tools --> Board --> "Board Managers" search for esp8266 and install version 2.4.0
8) Connect the Board and Upload the Code.