#include <SPI.h>
#include <RadioLib.h>
#include <WiFi.h>
#include <PubSubClient.h>
//#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SH110X.h>
//#define SCREEN_WIDTH 64
//#define SCREEN_HEIGHT 128
//#define OLED_RESET -1
//Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

unsigned long ti = 0;

SX1280 radio = new Module(5, 21, 33, 22);
struct {
  byte H1 = 0x12;
  byte H2 = 0x34;
  uint16_t ID;
  int16_t data1;
  int16_t data2;
  int16_t data3;
  int16_t data4;
  int16_t data5;
  int16_t data6;
  int16_t data7;
  int16_t data8;
} data_sx1280;

// กำหนดรายละเอียด WiFi
const char* ssid = "PCAT_4G_WIFI";
const char* password = "pcat@1234";

// กำหนดรายละเอียด MQTT Broker
const char* mqtt_server = "mqtt.aj-tay.com";
const int mqtt_port = 1883;             // พอร์ต MQTT
const char* mqtt_user = "Pcat_device";  // ชื่อผู้ใช้สำหรับ MQTT
const char* mqtt_pass = "pcat@1234";    // รหัสผ่านสำหรับ MQTT

WiFiClient espClient;
PubSubClient client(espClient);

const int TX_EN = 13;
const int RX_EN = 32;

const int INPUT_1 = 39;
const int INPUT_2 = 34;
const int INPUT_3 = 25;

const int RELAY = 2;

int INPUT_1_State = 0;
int INPUT_2_State = 0;
int INPUT_3_State = 0;

int State_1 = 0;
int State_2 = 0;
// int State_3 = 0;
// int State_4 = 0;
int State_send = 0;


float id1C = 0;
float id1H = 0;
float id1B = 0;
float id2C = 0;
float id2H = 0;
float id2B = 0;
float id3C = 0;
float id3H = 0;
float id3B = 0;
float id3PH = 0;
float id3N = 0;
float id3P = 0;
float id3K = 0;
float id3EC = 0;


void setup() {
  pinMode(TX_EN, OUTPUT);
  pinMode(RX_EN, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(INPUT_1, INPUT);
  pinMode(INPUT_2, INPUT);
  pinMode(INPUT_3, INPUT);
  digitalWrite(TX_EN, LOW);
  digitalWrite(RX_EN, HIGH);
  digitalWrite(RELAY, LOW);  //ปกติ


  Serial.begin(115200);
  delay(250);

  // เชื่อมต่อ WiFi
  setup_wifi();

  // กำหนดเซิร์ฟเวอร์ MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //  display.begin(0x3C, true);
  //  display.clearDisplay();
  //  display.setRotation(1);
  //  display.setTextSize(2);
  //  display.setTextColor(SH110X_WHITE);

  Serial.print(F("[SX1280] Initializing ... "));
  int state = radio.begin(2400.0 + (1 * 7), 203.125, 9, 7);  //ch (x * 7)
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
  radio.setDio1Action(setFlag);
  //radio.setRfSwitchPins(33, 32); /////
  radio.startReceive();
}

//////////////////////////////////////////////////////
void setup_wifi() {
  delay(10);
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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  digitalWrite(LED_BUILTIN, HIGH);

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "Smart_PCAT/R_RELAY1") {
    if (messageTemp == "On") {
      State_1 = 1;
      Serial.println("Relay ON");
    } else if (messageTemp == "Off") {
      State_1 = 0;
      Serial.println("Relay OFF");
    }
  }
  if (String(topic) == "Smart_PCAT/R_RELAY2") {
    if (messageTemp == "On") {
      State_2 = 1;
      Serial.println("Relay ON");
    } else if (messageTemp == "Off") {
      State_2 = 0;
      Serial.println("Relay OFF");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // ใช้ชื่อผู้ใช้และรหัสผ่านในการเชื่อมต่อ
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Subscribe to a topic
      client.subscribe("Smart_PCAT/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
/////////////////////////////////////////////////////


volatile bool receivedFlag = false;
void setFlag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}

void loop() {

  ti = millis();
  if (ti >= 43200000) {
    ESP.restart();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  // Example: publish a message to a topic
  //client.publish("esp32/test", "Hello from ESP32");
  //delay(2000);

  INPUT_1_State = digitalRead(INPUT_1);
  INPUT_2_State = digitalRead(INPUT_2);
  INPUT_3_State = digitalRead(INPUT_3);

  String payload;

  ///////////////////////////////////////////รับสวิตซ์
  if (INPUT_1_State == 0 && State_1 == 0) {
    delay(10);
    payload = "On";
    client.publish("Smart_PCAT/SW1", payload.c_str());
    Serial.println("SW1 : 1");
    delay(1000);
    State_1 = 1;
  } else if (INPUT_1_State == 0 && State_1 == 1) {
    delay(10);
    payload = "Off";
    client.publish("Smart_PCAT/SW1", payload.c_str());
    Serial.println("SW1 : 0");
    delay(1000);
    State_1 = 0;
  }

  if (INPUT_2_State == 0 && State_2 == 0) {
    delay(10);
    payload = "On";
    client.publish("Smart_PCAT/SW2", payload.c_str());
    Serial.println("SW2 : 1");
    delay(1000);
    State_2 = 1;
  } else if (INPUT_2_State == 0 && State_2 == 1) {
    delay(10);
    payload = "Off";
    client.publish("Smart_PCAT/SW2", payload.c_str());
    Serial.println("SW2 : 0");
    delay(1000);
    State_2 = 0;
  }


  if (State_1 == 1 || State_2 == 1) {
    if (State_send == 0) {
      digitalWrite(RELAY, HIGH);
      payload = "On";
      State_send = 1;
      client.publish("Smart_PCAT/RELAY", payload.c_str());
    }
  }
  if (State_1 == 0 && State_2 == 0) {
    if (State_send == 1) {
      digitalWrite(RELAY, LOW);
      payload = "Off";
      State_send = 0;
      client.publish("Smart_PCAT/RELAY", payload.c_str());
    }
  }


  ///////////////////////////////////////////รับRF
  if (receivedFlag) {
    // reset flag

    receivedFlag = false;
    uint8_t str[sizeof(data_sx1280)];
    int state = radio.readData(str, sizeof(data_sx1280));
    //uint8_t str[10];
    //int state = radio.readData(str, 10);

    if (state == RADIOLIB_ERR_NONE) {
      if (str[0] == 0x12 && str[1] == 0x34) {
        memcpy(&data_sx1280, str, sizeof(data_sx1280));
        if (data_sx1280.ID == 1) {
          id1C = (float)data_sx1280.data1 * 0.01f;
          id1H = ((float)data_sx1280.data2 * 0.01f) * 1.4;
          id1B = map(((float)data_sx1280.data3), 300, 420, 0, 100);

          if (id1H >= 100) {
            id1H = 100;
          }
          if (id1B >= 100) {
            id1B = 100;
          }


          String payload;
          payload = "ID1";
          client.publish("Melon/sensor1ID", payload.c_str());
          delay(1);
          payload = String(id1C, 2);
          client.publish("Melon/sensor1id1C", payload.c_str());
          delay(1);
          payload = String(id1H, 2);
          client.publish("Melon/sensor1id1H", payload.c_str());
          delay(1);
          payload = String(id1B, 2);
          client.publish("Melon/sensor1id1B", payload.c_str());
          delay(1);

          Serial.print("ID1 = ");
          Serial.print(id1C);
          Serial.print("C  ");
          Serial.print(id1H);
          Serial.print("%  ");
          Serial.print(id1B);
          Serial.println("%");
        }
        if (data_sx1280.ID == 2) {
          id2C = (float)data_sx1280.data1 * 0.01f;
          id2H = ((float)data_sx1280.data2 * 0.01f) * 1.4;
          id2B = map(((float)data_sx1280.data3), 300, 420, 0, 100);

          if (id2H >= 100) {
            id2H = 100;
          }
          if (id2B >= 100) {
            id2B = 100;
          }

          String payload;
          payload = "ID2";
          client.publish("Melon/sensor2ID", payload.c_str());
          delay(1);
          payload = String(id2C, 2);
          client.publish("Melon/sensor2id1C", payload.c_str());
          delay(1);
          payload = String(id2H, 2);
          client.publish("Melon/sensor2id1H", payload.c_str());
          delay(1);
          payload = String(id2B, 2);
          client.publish("Melon/sensor2id1B", payload.c_str());
          delay(1);

          Serial.print("ID2 = ");
          Serial.print(id2C);
          Serial.print("C  ");
          Serial.print(id2H);
          Serial.print("%  ");
          Serial.print(id2B);
          Serial.println("%");

          // id2RF = radio.getRSSI();
          // Serial.print("RF = ");
          // Serial.print(id2RF);
          // Serial.println(" dBm");
        }
        if (data_sx1280.ID == 3) {
          id3C = (float)data_sx1280.data1 * 0.01f;
          id3H = ((float)data_sx1280.data2 * 0.01f) * 2;
          id3B = map(((float)data_sx1280.data3), 300, 420, 0, 100);
          id3PH = (float)data_sx1280.data4 * 0.01f;  // PH
          id3N = (float)data_sx1280.data5 * 0.01f;   // N
          id3P = (float)data_sx1280.data6 * 0.01f;   // P
          id3K = (float)data_sx1280.data7 * 0.01f;   // K
          id3EC = (float)data_sx1280.data8 * 0.01f;  // EC


          if (id3H >= 100) {
            id3H = 100;
          }
          if (id3B >= 100) {
            id3B = 100;
          }

          String payload;
          payload = "ID3";
          client.publish("Melon/sensor3ID", payload.c_str());
          delay(1);
          payload = String(id3C, 2);
          client.publish("Melon/ ", payload.c_str());
          delay(1);
          payload = String(id3H, 2);
          client.publish("Melon/sensor3H", payload.c_str());
          delay(1);
          payload = String(id3B, 2);
          client.publish("Melon/sensor3B", payload.c_str());
          delay(1);
          payload = String(id3PH, 2);
          client.publish("Melon/sensor3PH", payload.c_str());
          delay(1);
          payload = String(id3N, 2);
          client.publish("Melon/sensor3N", payload.c_str());
          delay(1);
          payload = String(id3P, 2);
          client.publish("Melon/sensor3P", payload.c_str());
          delay(1);
          payload = String(id3K, 2);
          client.publish("Melon/sensor3K", payload.c_str());
          delay(1);
          payload = String(id3EC, 2);
          client.publish("Melon/sensor3EC", payload.c_str());
          delay(1);

          Serial.print("ID3 = ");
          Serial.print(id3C);
          Serial.print("C  ");
          Serial.print(id3H);
          Serial.print("%  ");
          Serial.print(id3B);
          Serial.print("%  ");
          Serial.print(id3PH);
          Serial.print("   ");
          Serial.print(id3N);
          Serial.print("   ");
          Serial.print(id3P);
          Serial.print("   ");
          Serial.print(id3K);
          Serial.print("   ");
          Serial.print(id3EC);
          Serial.println("   ");
        }

        delay(10);
      }
    }
    radio.startReceive();
  }
  delay(1);
}