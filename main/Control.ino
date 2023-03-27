#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_adc_cal.h"
// bạn thay đổi thông tin bên dưới cho phù hợp
const char* ssid =         "2.4GHZ_No Internet";
const char* password =    "2.4gkhongcointernet";
const char* mqtt_server = "192.168.20.103"; 
const uint16_t mqtt_port = 1883;
#define IN1 26
#define IN2 27
#define IN3 25
#define IN4 33
#define EN 14
#define LM35_Sensor1    35
#define IN1_b 5
#define IN2_b 18
#define IN3_b 21
#define IN4_b 19
#define EN_b 23
const int freq = 30000;
const int pwmChannel = 0;
const int pwmChannelb = 1;
const int resolution = 8;
int LM35_Raw_Sensor1 = 0;
float LM35_TempC_Sensor1 = 0.0;
float Voltage = 0.0;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(IN1, OUTPUT);  
  pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);  
  pinMode(IN4, OUTPUT);
      pinMode(IN1_b, OUTPUT);  
  pinMode(IN2_b, OUTPUT);
        pinMode(IN3_b, OUTPUT);  
  pinMode(IN4_b, OUTPUT);
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(EN, pwmChannel);
  ledcWrite(pwmChannel, 170);  
  ledcSetup(pwmChannelb, freq, resolution);
  ledcAttachPin(EN_b, pwmChannelb);
  ledcWrite(pwmChannelb, 1);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // Kết nối wifi
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


  if ((char)payload[0] == '1') {
     Serial.println("TIEN");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } 

 if ((char)payload[0] == '2') {
    Serial.println("LUI");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  if ((char)payload[0] == '3') {
    Serial.println("TRAI");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  if ((char)payload[0] == '4') {
    Serial.println("PHAI");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  if ((char)payload[0] == '0') {
    Serial.println("Dung");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
if ((char)payload[0] == '5') {
    digitalWrite(IN3_b, LOW);
    digitalWrite(IN4_b, HIGH);
  }
  if ((char)payload[0] == '6') {
    digitalWrite(IN3_b, LOW);
    digitalWrite(IN4_b, LOW);
  }  
}

void reconnect() {
  // Đợi tới khi kết nối
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Khi kết nối thành công sẽ gửi chuỗi helloworld lên topic event
      client.publish("event", "hello world");
      // ... sau đó sub lại thông tin
      client.subscribe("event");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  // Read LM35_Sensor1 ADC Pin
  LM35_Raw_Sensor1 = analogRead(LM35_Sensor1);  
  // Calibrate ADC & Get Voltage (in mV)
  Voltage = readADC_Cal(LM35_Raw_Sensor1);
  // TempC = Voltage(mV) / 10
  LM35_TempC_Sensor1 = Voltage / 10;
  delay(10);

Serial.println();
  client.loop();
 // Thực hiện 2s gửi dữ liệu helloworld lên broker 1 lần
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("event", msg);
    client.publish("nd", String(LM35_TempC_Sensor1).c_str());
  }
  if (LM35_TempC_Sensor1 > 35.0)
{
    digitalWrite(IN1_b, LOW);
    digitalWrite(IN2_b, HIGH);
}
  else
  {
    digitalWrite(IN1_b, LOW);
    digitalWrite(IN2_b, LOW);
  }  
}
uint32_t readADC_Cal(int ADC_Raw)
{
  esp_adc_cal_characteristics_t adc_chars;
  
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  return(esp_adc_cal_raw_to_voltage(ADC_Raw, &adc_chars));
}
