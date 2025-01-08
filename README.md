# Phoniekoffer

Der Phoniekoffer ist eine günstige Alternative auf Basis eines ESP32 zu einer Toniebox oder einer Phoniebox 
Ich habe das Projekt Phoniekoffer genannt, weil ich das erste Modell in einen kleinen Kinderkoffer gebaut habe. Bei dem Gehäuse sind der Phantasie natürlich keine Grenzen gesetzt. 

## Komponenten

ESP32\
DF Player Mini\
RFID Leser RC522\
3 Watt PC Lautsprecher\
SD Karte

Dazu kommen noch Kleinteile wie Jumperkabel, Buttons und RFID Tags.
Zusätzlich wird eine Powerbank benötigt.

## Verdrahtungsschema

## Installation
Für die Installation wird die Adruino IDE benötigt, mit der man den Sketch auf den ESP32 hochladen kann. 

Zusätzlich müssen in der IDE folgende Bibliotheken installiert sein.

MFRC522v2 (Achtung! Die Bibliothek MFRC522 funktioniert nicht)\
Button2\
DFRobotDFPlayerMini\
LittleFS

Für die http Verbindung mit dem Phoniekoffer gibt es 2 Optionen:

1. Einbinden in das lokale WLAN
2. Zugriff über Access Point

Um den Phoniekoffer in das lokale WLAN einzubinden, muss in dem Sketch die Variablen 
const char* ssid = "WLAN SSID";
const char* password = "WLAN Passwort";
angepasst werden.

Die IP Adresse des Phoniekoffers wird dann auf dem seiellen Monitor der Adruino IDE angezeigt. Sie ist dann unter http://IP_DER_PHONIEBOX zu erreichen.

Wenn der Phoniekoffer keine WLAN Verbindung aufbauen kann, erstellt er automatisch einen lokalen Access Point. Es existiert dann ein WLAN mit der SSID ESP32 und dem Passwort 12345678. Wenn man sich mit diesem WLAN verbindet kann man die Phoniebox unter der Adresse http://192.168.4.1 erreichen. 

> [!IMPORTANT]
> Ich habe die Erfahrungen gemacht, dass man die Adresse bei einem Handy nur aufrufen kann, wenn man die mobilen Daten ausschaltet. Bei eingeschalteten Mobilen Daten läuft der Aufruf auf ein Timeout

## Konfiguration

### SD Karte

Die SD Karte muss in FAT32 formatiert sein.

Auf der Karte werden Ordner für die MP3 Dateien mit einer vierstelligen Zahl angelegt. Nach der Zahl kann ein beliebiger Text folgen
0001_Kinderlieder1
0002_Kinderlieder2
...

In diesen Ordnern können bis zu 99 mp3 Dateien abgelegt werden. Diese müssen folgendes Format besitzen.
001_beliebiger_text.mp3
002_beliebiger_text.mp3
003_beliebiger_text.mp3
...

Dabei dürfen keine fortlaufenden Nummern ausgelassen werden.

Zusätzlich muss auf der Karte ein Ordner mp3 angelegt werden. In diesem Ordner können einzelne Dateien abgespielt werden. Die Dateien müssen das gleiche Format wie in den anderen Ordnern besitzen.

### RFID Tags zuweisen

Die RFID Tags werden über die Web Oberfläche des ESP32 zugewiesen. Dafür ruft man im Browser die IP Adresse des Phoniekoffers auf. Wenn man nun einen RFID-Tag auflegt, wird nach kurzer Zeit die Nummer des Tags mit einem Stift dahinter angezeigt.

Wenn man auf den Stift klickt, kann man nun in einem Textfeld angeben, was abgespielt werden soll. 

Bei einem dreistelligen Wert, wir der entsprechende Ordner Lied für Lied abgespielt.
001 = Order 0001

Bei einem einstelligen Wert, wir das entsprechende Lied aus dem Ordner mp3 abgespielt.
4 = Lied 004 aus dem Ordner mp3

Diese Wert speichern. Beim erneuten Auflegen des RFID Tags wird das gewünschte Lied abgespielt.





