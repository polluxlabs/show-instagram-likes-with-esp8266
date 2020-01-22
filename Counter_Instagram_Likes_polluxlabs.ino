/*********
  Pollux Labs
  Complete project details at https://polluxlabs.net
  Based on an example from ArduinoJson
*********/

// Enable support for escaped unicode characters (\u0123)
#define ARDUINOJSON_DECODE_UNICODE 1

//Libraries
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

//WiFi credentials
#define WIFI_SSID "your SSID"
#define WIFI_PASSWORD "your password"
//Instagram URL
#define REQUEST_HOST "www.instagram.com"
#define REQUEST_PATH "/spacex/?__a=1" //replace "spacex" with your Instagram account name, keep the /

//likes variables
int likesOld;
int likesNew;

void setup() {

  pinMode (4, OUTPUT); // at ESP8266: D2
  pinMode (12, OUTPUT); // at ESP8266: D6
  pinMode (14, OUTPUT); // at ESP8266: D5
  pinMode (15, OUTPUT); // at ESP8266: D8

  // Initialize Serial Port
  Serial.begin(115200);

  // Connect to Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Connecting to WiFi"));
    delay(500);
  }
}

void loop() {

  // Connect to the server
  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(10000);
  Serial.println(F("Connecting to server..."));
  if (!client.connect(F(REQUEST_HOST), 443)) {
    Serial.println(F("Failed to connect to server"));
    return;
  }

  Serial.println(F("Sending request..."));
  // Send the request path
  client.println(F("GET " REQUEST_PATH " HTTP/1.0"));
  // Send the headers
  client.println(F("Host: " REQUEST_HOST));
  client.println(F("Connection: close"));
  client.println();

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected HTTP response: "));
    Serial.println(status);
    return;
  }

  Serial.println(F("Receive response..."));

  //Jump to the relevant data in JSON
  if (!client.find("\"user\":")) {
    Serial.println(F("Array \"user\" not found in response"));
    return;
  }

  //define memory size for JSON
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(5) + 7 * JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + 5 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(22) + 3974;
  DynamicJsonDocument doc(capacity);

  if (!client.find("\"edge_owner_to_timeline_media\":")) {
    Serial.println(F("\"edge_owner_to_timeline_media\" not found in response"));
    return;
  }

  if (!client.find("\"edges\":")) {
    Serial.println(F("\"edges\" not found in response"));
    return;
  }

  client.find("[");

  // Deserialize the relevant piece of data
  auto err = deserializeJson(doc, client);
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
    return;
  }

  //save likes to variable
  likesNew = doc["node"]["edge_liked_by"]["count"];

  Serial.println(likesNew);

  //flash LED depending on likes count
  if (likesNew > likesOld + 5) {
    for (int light = 0; light <= 255; light++) {
      analogWrite(15, light);
      analogWrite(12, light);
      analogWrite(14, light);
      analogWrite(4, light);
      delay(5);
    }
    delay(300);
    for (int light = 255; light >= 0; light--) {
      analogWrite(15, light);
      analogWrite(12, light);
      analogWrite(14, light);
      analogWrite(4, light);
      delay(5);
    }
    likesOld = likesNew;
  } else if (likesNew > likesOld + 3) {
    for (int light = 0; light <= 255; light++) {
      analogWrite(12, light);
      analogWrite(14, light);
      analogWrite(4, light);
      delay(5);
    }
    delay(300);
    for (int light = 255; light >= 0; light--) {
      analogWrite(12, light);
      analogWrite(14, light);
      analogWrite(4, light);
      delay(5);
    }
    likesOld = likesNew;
  } else if (likesNew > likesOld + 1) {
    for (int light = 0; light <= 255; light++) {
      analogWrite(14, light);
      analogWrite(4, light);
      delay(5);
    }
    delay(300);
    for (int light = 255; light >= 0; light--) {
      analogWrite(14, light);
      analogWrite(4, light);
      delay(5);
    }
    likesOld = likesNew;
  } else if (likesNew > likesOld) {
    for (int light = 0; light <= 255; light++) {
      analogWrite(4, light);
      delay(5);
    }
    delay(300);
    for (int light = 255; light >= 0; light--) {
      analogWrite(4, light);
      delay(5);
    }
    likesOld = likesNew;
  }

  //Yes, you're done :)
  Serial.println("Done.");

  //Time until the next request
  delay(10000);
}
