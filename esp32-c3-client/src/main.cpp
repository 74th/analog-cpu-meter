#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ssid.h>

const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASSWORD;
WebServer server(80);

#define METER_PIN GPIO_NUM_0
#define METER_PWM_CH 0

void handleRoot(void)
{
    server.send(200, "application/json", "{}");
}

void handleNotFound(void)
{
    server.send(404, "application/json", "{\"message\": \"Not Found.\"}");
}

void handleCPU(void)
{
    if (server.method() != HTTP_POST)
    {
        server.send(405, "application/json", "{\"message\": \"Method Not Allowed.\"}");
        return;
    }
    Serial.println("POST /cpu");

    String requestBody = server.arg("plain");
    Serial.printf("request body: %s\r\n", requestBody.c_str());

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, requestBody);

    if (error)
    {
        Serial.println("failed to deserialize");
        Serial.println("POST /cpu 400");
        server.send(400, "application/json", "{\"message\": \"Bad Request.\"}");
        return;
    }

    int32_t value = doc["value"].as<int32_t>();

    if (value < 0 || 100 < value)
    {
        Serial.println("POST /cpu 400");
        server.send(400, "application/json", "{\"message\": \"Bad Request. Unexpected value.\"}");
        return;
    }

    Serial.printf("input value: %d\n", value);

    // 3.3Vを上限とすると、若干低くなったため、3.1Vとして計算
    float_t analogValue = 256 * 3 / 3.1 * value / 100;
    uint16_t pwmValue = (uint16_t)analogValue;

    Serial.printf("pwm value: %d\n", pwmValue);

    ledcWrite(METER_PWM_CH, analogValue);

    Serial.println("POST /cpu 200");
    server.send(200, "text/json", "{\"message\": \"Success.\"}");
}

void setup()
{
    Serial.begin(9600);

    // PWMの設定
    pinMode(METER_PIN, OUTPUT);
    ledcSetup(METER_PWM_CH, 1000, 8);
    ledcAttachPin(METER_PIN, METER_PWM_CH);
    ledcWrite(METER_PWM_CH, 0);

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
    server.onNotFound(handleNotFound);
    server.begin();
}

void loop()
{
    server.handleClient();
}