#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ssid.h>

const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASSWORD;
WebServer server(80);

#define METER_PIN_1 GPIO_NUM_0
#define METER_PIN_2 GPIO_NUM_1
#define METER_PWM_CH_1 0
#define METER_PWM_CH_2 1

#define LOG_INTERVAL_MS 2000

void handleRoot(void)
{
    server.send(200, "application/json", "{}");
}

void handleNotFound(void)
{
    server.send(404, "application/json", "{\"message\": \"Not Found.\"}");
}

void setPWMValue(uint8_t pwm_ch, uint8_t value)
{
    // 3.3Vを上限とすると、若干低くなったため、3.1Vとして計算
    float_t analogValue = 256 * 3 / 3.1 * value / 100;
    uint16_t pwmValue = (uint16_t)analogValue;

    Serial.printf("pwm value: %d\n", pwmValue);

    ledcWrite(pwm_ch, analogValue);
}

void handleSingle(const char *path, uint8_t pwm_ch)
{
    if (server.method() != HTTP_POST)
    {
        server.send(405, "application/json", "{\"message\": \"Method Not Allowed.\"}");
        return;
    }
    Serial.printf("POST /%s\r\n", path);

    String requestBody = server.arg("plain");
    Serial.printf("request body: %s\r\n", requestBody.c_str());

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, requestBody);

    if (error)
    {
        Serial.println("failed to deserialize");
        Serial.printf("POST /%s 400\r\n", path);
        server.send(400, "application/json", "{\"message\": \"Bad Request.\"}");
        return;
    }

    int32_t value = doc["value"].as<int32_t>();

    if (value < 0 || 100 < value)
    {
        Serial.printf("POST /%s 400\r\n", path);
        server.send(400, "application/json", "{\"message\": \"Bad Request. Unexpected value.\"}");
        return;
    }

    Serial.printf("input value: %d\n", value);

    setPWMValue(pwm_ch, value);

    Serial.printf("POST /%s 200\r\n", path);
    server.send(200, "text/json", "{\"message\": \"Success.\"}");
}

void handleCPU(void)
{
    handleSingle("cpu", METER_PWM_CH_1);
}

void handleMEM(void)
{
    handleSingle("mem", METER_PWM_CH_2);
}

void handleValues()
{
    if (server.method() != HTTP_POST)
    {
        server.send(405, "application/json", "{\"message\": \"Method Not Allowed.\"}");
        return;
    }
    Serial.printf("POST /values\r\n");

    String requestBody = server.arg("plain");
    Serial.printf("request body: %s\r\n", requestBody.c_str());

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, requestBody);

    if (error)
    {
        Serial.println("failed to deserialize");
        Serial.printf("POST /values 400\r\n");
        server.send(400, "application/json", "{\"message\": \"Bad Request.\"}");
        return;
    }

    int32_t cpuValue = doc["cpu"].as<int32_t>();

    if (cpuValue < 0 || 100 < cpuValue)
    {
        Serial.printf("POST /values 400\r\n");
        server.send(400, "application/json", "{\"message\": \"Bad Request. Unexpected value.\"}");
        return;
    }

    int32_t memValue = doc["mem"].as<int32_t>();

    if (cpuValue < 0 || 100 < memValue)
    {
        Serial.printf("POST /values 400\r\n");
        server.send(400, "application/json", "{\"message\": \"Bad Request. Unexpected value.\"}");
        return;
    }

    Serial.printf("input value: cpu:%d mem:%d\n", cpuValue, memValue);

    setPWMValue(METER_PWM_CH_1, cpuValue);
    setPWMValue(METER_PWM_CH_2, memValue);

    Serial.printf("POST /values 200\r\n");
    server.send(200, "text/json", "{\"message\": \"Success.\"}");
}

void setup()
{
    Serial.begin(9600);

    // PWMの設定
    pinMode(METER_PIN_1, OUTPUT);
    ledcSetup(METER_PWM_CH_1, 1000, 8);
    ledcAttachPin(METER_PIN_1, METER_PWM_CH_1);
    ledcWrite(METER_PWM_CH_1, 0);
    pinMode(METER_PIN_2, OUTPUT);
    ledcSetup(METER_PWM_CH_2, 1000, 8);
    ledcAttachPin(METER_PIN_2, METER_PWM_CH_2);
    ledcWrite(METER_PWM_CH_2, 0);

    // WiFiの接続
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
    Serial.print("IP:");
    Serial.println(WiFi.localIP());

    // Webサーバの開始
    server.on("/", handleRoot);
    server.on("/cpu", HTTP_POST, handleCPU);
    server.on("/mem", HTTP_POST, handleMEM);
    server.on("/values", HTTP_POST, handleValues);
    server.onNotFound(handleNotFound);
    server.begin();
}

unsigned long lastLogTimeMS = 0;

void loop()
{
    unsigned long now = millis();

    server.handleClient();

    if (now > lastLogTimeMS + LOG_INTERVAL_MS)
    {
        lastLogTimeMS = now;

        Serial.printf("[%12d]IP:%s \r\n", now, WiFi.localIP());
        Serial.printf("[%12d]CPU: %d, MEM: %d\r\n", now, ledcRead(METER_PWM_CH_1), ledcRead(METER_PWM_CH_2));
    }
}