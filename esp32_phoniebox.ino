/////////////////////////////////////////////////////////////////
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Button2.h"
#include "DFRobotDFPlayerMini.h"
#include "FS.h"
#include "LittleFS.h"

/////////////////////////////////////////////////////////////////

#define BUTTON1_PIN  15
#define BUTTON2_PIN  13
#define BUTTON3_PIN  14

Button2 button1;
Button2 button2;
Button2 button3;

/////////////////////////////////////////////////////////////////

MFRC522DriverPinSimple ss_pin(21); // Configurable, see typical pin layout above.

MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
MFRC522 mfrc522{driver};  // Create MFRC522 instance.

int chipId;

int mp3_volume = 20;
bool mp3_pause = false;

const char* ssid = "WLAN SSID";
const char* password = "WLAN Passwort";
const char* lssid = "ESP32";
const char* lpassword = "12345678";

int wifiTry = 5;
int counter = 0;

DFRobotDFPlayerMini mp3Player;

WebServer server(80);

HardwareSerial mySoftwareSerial(2); 

void setup() {
  Serial.begin(115200);
  delay(50);

  //WLAN initialisieren
  Serial.print("Verbinden mit ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && counter < wifiTry) {
   delay(500);
   counter = counter + 1;
   Serial.print(".");
  }
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Konnte keine Verbindung mit dem WLAN aufbauen.");
    Serial.println("ESP32 wird als Access Point konfiguriert.");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(lssid, lpassword);
    Serial.print("IP Adresse: ");
    Serial.println(WiFi.softAPIP());
  }else{
    Serial.println("WiFi verbunden");
    Serial.println("IP Adresse: ");
    Serial.println(WiFi.localIP());  
  }

  //Dateisystem initialisieren
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS Mount Failed");
    return;
  }

  if(!LittleFS.exists("/rfid")){
    Serial.println("Verzeichnis rfid erstellt");
    LittleFS.mkdir("/rfid");
  }

  // HTTP Server initialisieren
  server.on("/", handle_root);
  server.on("/chipId", handle_chipId);
  server.on("/edit", handle_edit);
  server.on("/save", handle_save);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  //MP3 Player initialisieren
  mySoftwareSerial.begin(9600, SERIAL_8N1, 16, 17);
  if (!mp3Player.begin(mySoftwareSerial)) { 
    Serial.println(F("MP3 Player startet nicht:"));
    Serial.println(F("1.Überprüfen Sie die Vebindungen!"));
    Serial.println(F("2.Ist die SD Karte eingesteckt?"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  mp3Player.volume(mp3_volume);  
  mp3Player.playMp3Folder(1);  
    
  //Buttons initialisieren
  button1.begin(BUTTON1_PIN);
  button2.begin(BUTTON2_PIN);
  button3.begin(BUTTON3_PIN);

  button1.setTapHandler(tap);
  button2.setTapHandler(tap);
  button3.setTapHandler(tap);

  // Card Reader initialisieren
  mfrc522.PCD_Init();  // MFRC522 initalisieren.
  Serial.println(F("Legen Sie eine Karte auf..."));   


}

/////////////////////////////////////////////////////////////////

void loop() {

  server.handleClient();
  
  button1.loop();
  button2.loop();
  button3.loop();

if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }else{
  // Select one of the cards.
    Serial.println("neue Karte");
    if ( !mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    int myChipId = 0;
    
    Serial.print(F("Card UID:"));
     for ( byte i = 0; i < mfrc522.uid.size; i++ ){
        myChipId=( ( myChipId + mfrc522.uid.uidByte[i] ) * 10 );
     }

    Serial.println(myChipId);
    chipId = myChipId;
    playMusic();

    mfrc522.PICC_HaltA();
    delay(5000);

  }  
}

/////////////////////////////////////////////////////////////////

void tap(Button2& btn) {
    if (btn == button1) {

      Serial.println("2 clicked");
      if(mp3_pause){
        mp3Player.start();
        mp3_pause = false;
      }else{
        mp3Player.pause();
        mp3_pause = true;        
      }
    } else if (btn == button2) {
      Serial.println("1 clicked");
      if(mp3_volume <= 30){
        mp3_volume = mp3_volume + 1;
        mp3Player.volume(mp3_volume);
      }
    }else if (btn == button3) {
      
      Serial.println("3 clicked");
      if(mp3_volume > 0){
        mp3_volume = mp3_volume - 1;
        mp3Player.volume(mp3_volume);
      }
    }
}


void playMusic(){
  File file = LittleFS.open("/rfid/" + String(chipId), "r");
  String fileContent = "";
  while ( file.available() ) {
    fileContent += (char)file.read();
  }
  Serial.println(fileContent + "/" + fileContent.length());
  if(fileContent.length() == 1){
    mp3Player.playMp3Folder(fileContent.toInt());
  }
  if(fileContent.length() == 2){
    mp3Player.loopFolder(fileContent.toInt());
  }

  
}

void handle_root() {
  Serial.println("Startseite");
  server.send(200, "text/html", SendHTML()); 
}

void handle_save(){
   String id = server.arg("chipId"); 
   String path = server.arg("play"); 
   File file1 = LittleFS.open("/rfid/" + id, "w");
   // Daten in die Datei schreiben
   file1.print(path);
   delay(2000);
   // Schließen der Datei
   file1.close();
   handle_root();     
}

void handle_edit(){
  server.send(200, "text/html", SendEditHTML(server.arg("chipId"))); 
}

void handle_chipId(){
  server.send(200, "text/html", UpdateChipId()); 
}

void handle_NotFound(){
  server.send(200, "text/text", "Seite nicht gefunden"); 
}

String UpdateChipId(){

  String ptr ="<h3 id=\"cardId\">Card Id: " + String(chipId) + "\n";
  if(chipId > 0){
    ptr +="<a href=\"edit?chipId=" + String(chipId) + "\"><img src=\"data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAACXBIWXMAAAB2AAAAdgFOeyYIAAAAGXRFWHRTb2Z0d2FyZQB3d3cuaW5rc2NhcGUub3Jnm+48GgAAAI5JREFUOI3t0iEOwlAQRdFTQYLEdQUsA49oWASGgEFiQSBZAWwBjyJpEOwAVbYCiP9raug4BFdOct9kMq8QZ44xhthF5TVqDDDCISIv8cQb5xyyj8gVSjxySC2d01tuKdFI53xlhVlnNsXiL/+aPMGxM6uk//diKzVrE93ccskBL5wim6GQatngjitukYAPlhoh+WBViUUAAAAASUVORK5CYII=\"></a>\n";
  }
  return ptr;
}

String SendHTML(){

  File dir = LittleFS.open("/rfid");
  
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Phoniekoffer</h1>\n";
  ptr +="<div id=\"cardId\">\n";
  ptr +="<h3>Card Id: " + String(chipId) + "\n";
  if(chipId > 0){
    ptr +="<a href=\"edit?chipId=" + String(chipId) + "\"><img src=\"data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAACXBIWXMAAAB2AAAAdgFOeyYIAAAAGXRFWHRTb2Z0d2FyZQB3d3cuaW5rc2NhcGUub3Jnm+48GgAAAI5JREFUOI3t0iEOwlAQRdFTQYLEdQUsA49oWASGgEFiQSBZAWwBjyJpEOwAVbYCiP9raug4BFdOct9kMq8QZ44xhthF5TVqDDDCISIv8cQb5xyyj8gVSjxySC2d01tuKdFI53xlhVlnNsXiL/+aPMGxM6uk//diKzVrE93ccskBL5wim6GQatngjitukYAPlhoh+WBViUUAAAAASUVORK5CYII=\"></a>\n";
  }
  ptr +="</div>\n";
    File file = dir.openNextFile();
    while (file) {
      ptr += "<a href=\"delete?file=" + String(file.name()) + "\">";
      ptr += String(file.name()) + ":";
      File file1 = LittleFS.open("/rfid/" + String(file.name()), "r");
      String fileContent = "";
      while ( file1.available() ) {
        fileContent += (char)file1.read();
      }

      // Ausgabe des Dateiinhalts
      ptr += " " + fileContent + "</a></br>\n";

      // Schließen der Datei
      file1.close();
      file = dir.openNextFile();    
  }
  ptr +="<script>\n";
  ptr +="function loadDoc() {\n";
  ptr +=" const xhttp = new XMLHttpRequest();\n";
  ptr +=" xhttp.onload = function() {\n";
  ptr +=" document.getElementById(\"cardId\").innerHTML = this.responseText;\n";
  ptr +="}\n";
  ptr +=" xhttp.open(\"GET\", \"/chipId\", true);\n";
  ptr +=" xhttp.send();\n";
  ptr +="}\n";
  ptr +="var interval = setInterval(loadDoc, 5000);\n";
  
  ptr +="</script>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String SendEditHTML(String RfidId){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Card Id: " + RfidId + "\n";
  ptr +="<form method=\"get\" action=\"save\">\n";
   ptr +="<input type=\"hidden\" name=\"chipId\" value=\"" + RfidId + "\">\n";
  ptr +="<input type=\"text\" name=\"play\">\n";
  ptr +="<input type=\"submit\" vlaue=\"speichern\">\n";
  ptr +="</form>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
