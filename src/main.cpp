#include <Arduino.h>
#include <DHTesp.h>
#include <BH1750.h>
#define LED_GREEN  4
#define LED_YELLOW 5
#define LED_RED    18
#define PUSH_BUTTON 23
#define DHT_PIN 19

int g_nCount=0;
DHTesp dht;
BH1750 lightMeter;
SemaphoreHandle_t xSemaphore;

IRAM_ATTR void isrPushButton() {
  g_nCount++;
  xSemaphoreGive(xSemaphore);
}

void taskReadSensor(void *pvParameters) {
  dht.setup(DHT_PIN, DHTesp::DHT11);

  Wire.begin();
  lightMeter.begin(); 
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);

  while (1) {
    digitalWrite(LED_GREEN, HIGH);
    float fHumidity = dht.getHumidity();
    float fTemperature = dht.getTemperature();
    float lux = lightMeter.readLightLevel();
    Serial.printf("Humidity: %.2f, Temperature: %.2f, Light: %.2f \n",
       fHumidity, fTemperature, lux);
    digitalWrite(LED_YELLOW, (fHumidity > 80)?HIGH:LOW);
    digitalWrite(LED_GREEN, LOW);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void taskPushButtonCounter(void *pvParameters) {
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);

  while (1) {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    digitalWrite(LED_RED, HIGH);
    Serial.printf("Counter: %d \n", g_nCount); fflush(stdout);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    digitalWrite(LED_RED, LOW);
  }
}

void setup() {
  Serial.begin(460800);
  xSemaphore = xSemaphoreCreateBinary();
  // init Leds
  pinMode(LED_BUILTIN, OUTPUT);

  // init push button interrupt
  pinMode(PUSH_BUTTON, INPUT_PULLUP);
  attachInterrupt(PUSH_BUTTON, isrPushButton, FALLING);
  xTaskCreatePinnedToCore(taskReadSensor, "taskReadSensor", configMINIMAL_STACK_SIZE+2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskPushButtonCounter, "taskPushButtonCounter", configMINIMAL_STACK_SIZE+2048, NULL, 2, NULL, 0);
  Serial.println("System ready");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  vTaskDelay(50 / portTICK_PERIOD_MS);
  digitalWrite(LED_BUILTIN, LOW);
  vTaskDelay(950 / portTICK_PERIOD_MS);  
}