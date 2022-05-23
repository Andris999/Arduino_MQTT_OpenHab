#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>

byte mac [6] = {0x54, 0x34, 0x41, 0x30, 0x30, 0x31};
IPAddress ip(192, 168, 88, 40);
IPAddress server(192, 168, 88, 41);

#define DHTPIN 2
#define RELE 3
#define izzo1 30
#define izzo2 32
#define izzo3 34
#define POTI1 A8
#define POTI2 A9
#define POTI3 A10
#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define temperature_topic "szoba1/homerseklet"  //Topic 
#define humidity_topic "szoba1/paratartalom"    //Topic 
#define Tizzo1 "szoba1/izzo"  //Topic 
#define Tizzo2 "szoba2/izzo"    //Topic 
#define Tizzo3 "szoba3/izzo"  //Topic 

char message_buff[100];
DHT dht(DHTPIN, DHTTYPE);
long lastMsg = 0;   
long lastRecu = 0;
int pot1;
int pot2;
int pot3;
//-----------------------------------------------------
//bejövő üzenet
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if((char)payload[0] == 'O' && (char)payload[1] == 'N')
      {
        digitalWrite(RELE, LOW); 
        Serial.println("Turning on");
      }
      else
      {
        digitalWrite(RELE, HIGH);
        Serial.println("Turning off");
      }
}
//--------------------------------------------------------
EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//--------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  dht.begin();
  pinMode(RELE,OUTPUT);
  pinMode(izzo1,OUTPUT);
  pinMode(izzo2,OUTPUT);
  pinMode(izzo3,OUTPUT);
  pinMode(POTI1, INPUT);
  pinMode(POTI2, INPUT);
  pinMode(POTI3, INPUT);
  digitalWrite(RELE, HIGH);
  client.setServer(server, 1883);
  client.setCallback(callback);
  
  Ethernet.begin(mac, ip);
  
  delay(1500);
}
//--------------------------------------------------------
void loop()
{ 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();

    //pot1 = map(analogRead(POTI1), 0, 1023, 0, 1);
    pot1 = analogRead(POTI1);
    delay(10);
    //pot2 = map(analogRead(POTI2), 0, 1023, 0, 1);
    pot2 = analogRead(POTI2);
    delay(10);
    //pot3 = map(analogRead(POTI3), 0, 1023, 0, 1);
    pot3 = analogRead(POTI3);
    delay(10);
    
    if(pot1>511) pot1=HIGH;
    else pot1=LOW;
    if(pot2>511) pot2=HIGH;
    else pot2=LOW;
    if(pot3>511) pot3=HIGH;
    else pot3=LOW;

    digitalWrite(izzo1, pot1);
    digitalWrite(izzo2, pot2);
    digitalWrite(izzo3, pot3);
  
  // percenkent kuld
  if (now - lastMsg > 1000 * 60) {
    lastMsg = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // Oh, nothing to send
    if ( isnan(t) || isnan(h)) {
      Serial.println("Nincs jel, ellenorizd a DHT22 szenzort!");
      return;
    }
    client.publish(temperature_topic, String(t).c_str(), true);   //küldés
    client.publish(humidity_topic, String(h).c_str(), true);
    client.publish(Tizzo1, String(pot1).c_str(), true);
    client.publish(Tizzo2, String(pot2).c_str(), true);
    client.publish(Tizzo3, String(pot3).c_str(), true);
}
  
  
  if (now - lastRecu > 100 ) {
    lastRecu = now;
    client.subscribe("szoba2/rele"); //felíratkozás
  }
  delay(1000);
}
