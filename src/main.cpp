#include <PubSubClient.h>
#include <BH1750.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SafeString.h"
#define ONE_WIRE_BUS 18 
#define LED 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempsensors(&oneWire);

 

//---- WiFi settings
const char* ssid = "Phee";
const char* password = "paengpaeng";

//---- MQTT Broker settings
const char* mqtt_server = "66d6b91771ff4fc7bb664c04cc3e7fbb.s2.eu.hivemq.cloud";
const char* mqtt_username = "ICERUS";
const char* mqtt_password = "Projectyear3";
const int mqtt_port =8883;
const char* boxId = "124";

const int watering = 27;
const int light1 = 13;
const int light2 = 12;
const int light3 = 14;

const int AirValue = 3371;   //replace this value with Value_1
const int WaterValue = 1705;  //replace this value with Value_2

const int SensorPin = 35;

BH1750 lightMeter(0x23);
 

WiFiClientSecure espClient;   // for no secure connection use WiFiClient instead of WiFiClientSecure 
//WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];


int light_sensor = 0;
int rh_sensor = 0;
int rh_value = 0;
float temp_value = 0;

int defaultCount = 0;
int led_status = 0;

const char* light_sensor_topic= "sensor/light";
const char*  rh_sensor_topic="sensor/rh";
const char* temp_sensor_topic= "sensor/temp";


const char* command1_topic="sensor/watering";
const char* command2_topic="sensor/led";





static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";


// This void is called every time we have a message from the broker
void callback(char* topic, byte* payload, unsigned int length) {
  //String incommingMessage = "";
  createSafeString(incommingMessage, 50);
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  createSafeString(recievedId, 20);
  createSafeString(command,20);

  size_t nextIdx = 0;
  nextIdx = incommingMessage.stoken(recievedId, nextIdx, ",");
  nextIdx++; //step over delimiter
  nextIdx = incommingMessage.stoken(command, nextIdx, ",");
  nextIdx++; //step over delimiter
  recievedId.trim();
  command.trim();
  
  Serial.print("\nMessage arrived ["+String(topic)+"]");
  Serial.print(incommingMessage);
  Serial.print("\n");
  Serial.print(recievedId);
  Serial.print("\n");
  Serial.print(command);
  Serial.print("\n");

  if(!recievedId.equals(boxId)){
    Serial.println("BoxId Mismatch");
    return;

  }

  if (defaultCount == 0) {
    digitalWrite(watering, LOW);
    digitalWrite(light1, LOW);
    digitalWrite(light2, LOW);
    digitalWrite(light3, LOW);
    incommingMessage = "";
    defaultCount = 1;
    return;
  }
  
  //--- check the incomming message (waterpump)
    if( strcmp(topic,command1_topic) == 0){
      Serial.println("Watering System ");
     if (command.equals("on")) {
       digitalWrite(watering, HIGH);
       Serial.println("ON");  // Turn the pump on 
       //delay(2000);
       //digitalWrite(watering, LOW);
       //Serial.println("OFF");   
     }
     else {
       digitalWrite(watering, LOW);
       Serial.println("OFF");  // Turn the pump off 
     }
  }

   //  check for other commands (led)
   else  if( strcmp(topic,command2_topic) == 0){
     if (command.equals("high")) {
       if (led_status == 0){
        digitalWrite(light1, LOW);
        digitalWrite(light2, HIGH);
        digitalWrite(light3, LOW);
        delay(1500);

        digitalWrite(light1, HIGH);
        digitalWrite(light2, LOW);
        digitalWrite(light3, HIGH);
        delay(1500);

        digitalWrite(light1, HIGH);
        digitalWrite(light2, HIGH);
        digitalWrite(light3, HIGH);
        Serial.println("HIGH LED");
        led_status = 3;
       }
       else if (led_status == 1) {
        digitalWrite(light1, HIGH);
        digitalWrite(light2, LOW);
        digitalWrite(light3, HIGH);
        delay(1500);

        digitalWrite(light1, HIGH);
        digitalWrite(light2, HIGH);
        digitalWrite(light3, HIGH);
        Serial.println("HIGH LED");
        led_status = 3;

       }
       else if (led_status == 2) {
        digitalWrite(light1, HIGH);
        digitalWrite(light2, HIGH);
        digitalWrite(light3, HIGH);
        Serial.println("HIGH LED");
        led_status = 3;
       }
       else {

       }
        

       } 
      else if (command.equals("med")){
        if (led_status == 0) {
        digitalWrite(light1, LOW);
        digitalWrite(light2, HIGH);
        digitalWrite(light3, LOW);
        delay(1500);

        digitalWrite(light1, HIGH);
        digitalWrite(light2, LOW);
        digitalWrite(light3, HIGH);
        Serial.println("MED LED");
        led_status = 2;
        }
        else if (led_status == 1 || led_status == 3) {
        digitalWrite(light1, HIGH);
        digitalWrite(light2, LOW);
        digitalWrite(light3, HIGH);
        Serial.println("MED LED");
        led_status = 2;
        }
        else {

        }

      }
      else if (command.equals("low")) {
        digitalWrite(light1, LOW);
        digitalWrite(light2, HIGH);
        digitalWrite(light3, LOW);
        Serial.println("LOW LED");
        led_status = 1;

      }
      else {
        digitalWrite(light1, LOW);
        digitalWrite(light2, LOW);
        digitalWrite(light3, LOW);
        Serial.println("OFF");
        led_status = 0;
      }
  }
  else {

  }
  
}
//==========================================
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}


//=====================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      digitalWrite(LED,HIGH);
      client.subscribe(command1_topic);   // subscribe the topics here
      client.subscribe(command2_topic);   // subscribe the topics here
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//================================================ setup
//================================================
void setup() {
  Serial.begin(9600);
  Serial.println("BoxId= "+String(boxId));
  pinMode(watering, OUTPUT);
  pinMode(light1, OUTPUT);
  pinMode(light2, OUTPUT);
  pinMode(light3, OUTPUT);
  pinMode(LED,OUTPUT);

  digitalWrite(watering, LOW);
  digitalWrite(light1, LOW);
  digitalWrite(light2, LOW);
  digitalWrite(light3, LOW);

  Wire.begin();
  tempsensors.begin();
  lightMeter.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }
  while (!Serial) delay(1);
  setup_wifi();
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output


  #ifdef ESP8266
    espClient.setInsecure();
  #else   // for the ESP32
    espClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  #endif
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

//======================================= publising as strings
void publishMessage(const char* topic, String payload , boolean retained){
  client.publish(topic, (byte*) payload.c_str(), 10, true);
  Serial.println("Message publised ["+String(topic)+"]: "+payload);
}


//================================================ loop
//================================================
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  //----  publish sensor values every 10 sec
  unsigned long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;
    
    if (lightMeter.measurementReady()) {
    light_sensor= lightMeter.readLightLevel();   
    }
    rh_sensor = analogRead(SensorPin);
    Serial.println(rh_sensor);
    rh_value = map(rh_sensor, AirValue, WaterValue, 0, 100);
      Serial.println(rh_value);

    tempsensors.requestTemperatures();
    temp_value = tempsensors.getTempCByIndex(0);
    Serial.println(temp_value);

    publishMessage(light_sensor_topic,String(boxId)+","+String(light_sensor),true);    
    publishMessage(rh_sensor_topic,String(boxId)+","+String(rh_value)+"%",true);
    publishMessage(temp_sensor_topic,String(boxId)+","+String(temp_value),true);
    
  }
}

//=======================================  







