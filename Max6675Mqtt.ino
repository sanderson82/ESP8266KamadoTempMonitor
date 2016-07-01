#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <max6675.h>
#include <Wire.h>

// How often do we collect data
#define COLLECTION_PERIOD_IN_MS 30000

#define SO_PIN 12
#define CS_PIN 13
#define CLK_PIN 14
 
MAX6675 ktc(CLK_PIN, CS_PIN, SO_PIN);

const char* ssid = "put your ssid here";
const char* password = "put your wifi pass here";

const char* mqtt_temp_c_topic = "put temp c topic here";
const char* mqtt_temp_f_topic = "put temp f topic here";

// The mqtt server.  This one is public and great for testing
const char* mqtt_server = "broker.hivemq.com";

// Set a broker client name.  This should be unique otherwise you will 
// have mqtt connection issues
const char* mqtt_client_name = "put unique client name here";

WiFiClient espClient;
PubSubClient client(espClient);

// The last message time
long lastMsg = 0;

char tempC[8];
char tempF[8];

int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client_name)) {
      Serial.println("connected");
      break;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  Serial.print("Connecting to");
  Serial.println(mqtt_server); 
  client.setServer(mqtt_server, 1883);
}

void loop() {

  if (!client.connected()) {
    Serial.println("Not connected, reconnecting");
    reconnect();
  }

  client.loop();

  long now = millis();
  if (now - lastMsg > COLLECTION_PERIOD_IN_MS) {
    lastMsg = now;
    ++value;

    Serial.print("C = "); 
    Serial.print(ktc.readCelsius());
    Serial.print("\t F = ");
    Serial.println(ktc.readFahrenheit());
   
    dtostrf(ktc.readFahrenheit(), 4, 4, tempF);
    dtostrf(ktc.readCelsius(), 4, 4, tempC);

    client.publish(mqtt_temp_f_topic, tempF);
    client.publish(mqtt_temp_c_topic, tempC);
  }
}
