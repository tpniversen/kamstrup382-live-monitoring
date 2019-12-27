/*
 NodeMCU DEVKIT v1.0
 Read analog signal connected to A0
*/
#include <Redis.h>
#include <ESP8266WiFi.h>
     
const char* ssid     = "Iversen";
const char* password = "LockCityRocks";  
int wifiStatus;

// Redis details
#define REDISHOST "192.168.1.14"
#define REDISPORT 6379
#define REDIS_PASSWORD "reTaOijS7ICdoqB5l0e0uYBHwegQJasqcgyMyKWhClWBp7rNEizsWWpJ6jQWa0qfHoe1oWhe3ZYAZLDh"
Redis redis(REDISHOST, REDISPORT);
const char* listKey = "poweragent:flash";
char redisTime[18];

int sensorPin = A0;                 // References A0 analog pin
int sensorValue;                    // 10-bit equivalent value of analog signal
float sensorValuePercentage;
float sensorValuePercentageDiff;
float movingAvg = 0.0;
int movingAvgWindow = 20;
byte threshold = 15;

int elMeasures[10];
int delayMs = 50;
//char sequence[40];
int randomNum = 0;
unsigned long sampleNum = 0;

void setup() {
  Serial.begin(115200);             // Communication at 115200 Bd with Serial Monitor
  delay(10);

  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  wifiStatus = WiFi.status();
  if(wifiStatus == WL_CONNECTED){
     Serial.println("");
     Serial.println("ESP is connected!");  
     Serial.println("Your IP address is: ");
     Serial.println(WiFi.localIP());
     Serial.println("Your MAC address is: ");
     Serial.println(WiFi.macAddress().c_str());
  }

  if (redis.begin(REDIS_PASSWORD)){
    Serial.println("Connected to the Redis server!");
  } 
  else {
    Serial.println("Failed to connect to the Redis server!");
    return;
  }

  Serial.println(redis.get("test"));

  Serial.println("");
  Serial.println("Read of analog value on A0 pin.");
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  randomNum = rand() % 50;
  if(randomNum == 5) {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(delayMs/2);                         // wait for 50 milliseconds
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage LOW
  }
  // Let's measure analog value
  sensorValue = analogRead(sensorPin);
  elMeasures[sampleNum%10] = sensorValue;
  if(movingAvg==0.0) { movingAvg = (float)sensorValue; }
  movingAvg = ((movingAvg/(float)movingAvgWindow)*(movingAvgWindow-1)) + ((float)sensorValue/(float)movingAvgWindow);
  sensorValuePercentage = ((float)sensorValue/movingAvg)*100.0;
  sensorValuePercentageDiff = sensorValuePercentage-100.0;

  if(abs(sensorValuePercentageDiff)>threshold) {
    Serial.println("");
    Serial.print("Flash detected!!!: ");
    Serial.print(" Sample#: ");
    Serial.print(sampleNum);
    Serial.print(" SensorValue: ");
    Serial.print(sensorValue);
    Serial.print(" Average: ");
    Serial.print(movingAvg);
    Serial.print(" PercentageDiff: ");
    Serial.println(sensorValuePercentageDiff);
    Serial.print("Insert to list: ");
    redis.time().toCharArray(redisTime,18);
    Serial.println(redis.rpush(listKey, redisTime));
  }
  
  if(sampleNum%10 == 9) {
    Serial.println("");
    Serial.print("Flushing samples:");
    for (int i = 0; i < 10; i++) {
      Serial.print(elMeasures[i]);
      Serial.print(' ');
    }
  }
  
  //Serial.print("SET foo bar: ");
  //if (redis.set("foo", "bar"))
  //{
  //  Serial.println("ok!");
  //}
  //else
  //{
  //  Serial.println("err!");
  //}

  if(randomNum == 5) {
    delay(delayMs/2);
  } else {
    delay(delayMs);                       // Wait for 0.1 second
  }
  
  sampleNum = (sampleNum + 1);
}
