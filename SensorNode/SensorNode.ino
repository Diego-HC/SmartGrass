#define LED_BUILTIN 2
#define BUTTON_BUILTIN 0
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <DHT.h>

#define moistureSensor A0

#define airVal 639
#define waterVal 268

#define DHTPIN D6
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const char* WIFI_SSID = "ideapad gaming 3";
const char* WIFI_PASSWORD = "12345678";
WiFiServer server(80);

#define FIREBASE_HOST "smartgrass-8493d-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "Xqt9WfMWtQd6TZgEy3AEdBQ1STELEIT5Nh4wdDHN"

long PastTime = 0;
float temp = 0;
int hum = 0;
int cont = 0;
String estado;
float m;
float mPercent;
int numMed;

FirebaseData firebaseData;
FirebaseData tempVal;
FirebaseData humVal;
FirebaseData numMediciones;
FirebaseJson json;

void setup() {
  Serial.begin(115200);

  delay(50);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi is now connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("The server was been started");

  pinMode(LED_BUILTIN, OUTPUT);
      
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  dht.begin();

  pinMode(moistureSensor, INPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  Firebase.setString(firebaseData, "ServerNode", "ON");
}
 
void loop() {
  Firebase.setString(firebaseData, "SensorNode1", "ON"); 
  
  delay(5000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h)){
    Serial.println("\nError humedad");
  }
  else{
    Serial.print("Humedad: ");
    Serial.print(h);
  }

  if (isnan(t)){
    Serial.println("\nError temperatura");
  }
  else{
    Serial.print("Temperatura: ");
    Serial.println(t);
    Firebase.setInt(firebaseData, "Temperature", t);
    Firebase.pushInt(firebaseData, "TempReads", t);
  }

  m = analogRead(moistureSensor);
  mPercent = map(m, airVal, waterVal, 0, 100);
  Serial.print(m);
  Serial.print(" | ");
  if (mPercent >= 100)
  {
    Serial.print(100);  
    mPercent = 100;
  }
  else if (mPercent <= 0)
  {
    Serial.print(0);
    mPercent = 0;
  }
  else
  {
    Serial.print(mPercent);
  }
  Serial.println("%");
  Firebase.setInt(firebaseData, "Humidity", mPercent);
  Firebase.pushInt(firebaseData, "HumReads", mPercent);
  
  Firebase.getFloat(tempVal, "/Temperature");
  temp = tempVal.floatData();

  Firebase.getInt(humVal, "/Humidity");
  hum = humVal.intData();

  Firebase.getInt(numMediciones, "/readingsSN1");
  Firebase.setInt(firebaseData, "readingsSN1", numMediciones.intData() + 1);

  if (hum <= 20){
    estado = "La planta se está regando";
  }
  else {
    estado = "La planta no necesita riego";
  }

  WiFiClient client = server.available();
   
  if (client) {
    Serial.println("Nueva conexion");
    
    // Escribir las cabeceras HTML
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
   
    //Escribir el contenido que vamos a visualizar
    client.print("<html lang=");
    client.print("en");
    client.print("> <head> <title> SmartGrass </title>");
    client.println("<META HTTP-EQUIV=");
    client.print("REFRESH");
    client.println("CONTENT=");
    client.print("3;");
    client.println("> <style> .header {height: 12rem; background: linear-gradient(to bottom, #60953B, #ABE770); text-align: center}");
    client.println(".titulo {padding-top: 1rem; margin: 0; font-family: 'Bree Serif', serif; font-size: 7rem; color: #ffffff}");
    client.println(".variables {display: flex; justify-content: space-evenly; gap: 8rem}");
    client.println(".variable {display: flex; flex-direction: column; align-items: center;}");
    client.println("h2 {font-family: Arial; font-size: 4rem; font-weight: 500;}");
    client.println("h3 {font-family: Arial; font-size: 3.5rem; font-weight: 500;}");
    client.println(".estado {font-family: Arial; font-size: 3rem; font-weight: 600; color: #60953B; text-align: center;}");
    
    // Si la planta se esta regando, cambiar los colores de la interfaz
    if (hum <= 20) {
    client.println(".header {background: linear-gradient(to bottom, #3077B7, #98C9F7)}");
    client.println(".estado {color: #3077B7}");
    }

    client.println("</style> </head>");

    //Body
    client.println("<body>");
    client.println("<div class=");
    client.print("header");
    client.print("> <h1 class=");
    client.print("titulo");
    client.print("> SmartGrass View </h1> </div>");
    client.println("<div class=");
    client.print("variables");
    client.print("> <div class=");
    client.print("variable");
    client.println("> <h2> Temperature </h2>");
    client.println("<h3>");
    client.print(temp);
    client.println("&#8451 </h3> </div>");
    client.println("<div class=");
    client.print("variable");
    client.println("> <h2> Humidity </h2>");
    client.println("<h3>");
    client.print(hum);
    client.println("% </h3> </div> </div>");
    client.println("<p class=");
    client.print("estado");
    client.println(">");
    client.print(estado);
    client.println("</p> </body> </html>");
  }
}
