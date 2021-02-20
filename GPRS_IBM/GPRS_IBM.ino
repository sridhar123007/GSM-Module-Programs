#include <ArduinoJson.h>
#define TINY_GSM_MODEM_SIM800
//#define TINY_GSM_MODEM_SIM900
#define TINY_GSM_RX_BUFFER 650
//#define SerialAT Serial2
#include <TinyGsmClient.h>
#include<ArduinoHttpClient.h>//add https://github.com/arduino-libraries/ArduinoHttpClient/pull/64
#include <SoftwareSerial.h>
SoftwareSerial serialAT(6, 7); // RX, TX

//Your GPRS crendentials
const char apn[] = "airtelgprs.com";//"internet"
const char user[] = "";
const char pass[] = "";


//IBM credentials
#define ORG "zv2k2t"
#define DEVICE_TYPE "hello"
#define DEVICE_ID "1234"
#define TOKEN "12345678"
#define EVENT "Data"

//https://www.ibm.com/support/knowledgecenter/en/SSQPBH/iot/platform/devices/api.html
//https://developer.ibm.com/iotplatform/2017/06/09/http-for-devices/

String server = ORG ".messaging.internetofthings.ibmcloud.com";
const int port = 1883;
int publishInterval = 20000; // 20 seconds
long lastPublishMillis;
int retry = 0;

TinyGsm modem(serialAT);
TinyGsmClient client(modem);
HttpClient http_no_auth(client, server, port);

void check_sim_on() {
  //pinMode(PWR_MODEM_SIM800, OUTPUT);
  Serial.println("check power on modem...");
  while (modem.testAT() == false)
  {
    Serial.print(".");
    delay(3000);
  }
}
void httpPostNoAuth(const char* method, const String & path, const String & data, HttpClient* http) {
  String response;
  int statusCode = 0;

  String url;
  if (path[0] != '/') {
    url = " / ";
  }
  url += path;
  http->setHttpResponseTimeout(60 * 1000);
  http->beginRequest();
  http->post(url);
  http->sendBasicAuth("use-token-auth", TOKEN);
  http->sendHeader("Content-Type", "application/json");
  http->beginBody();
  http->print(data);
  http->endRequest();
//read the staus code and body of the response
  statusCode = http->responseStatusCode();
  Serial.print("Status Code: ");
  Serial.print(statusCode);
  response = http->responseBody();
  Serial.print("Response: ");
  Serial.print(response);
  if (!http->connected()) {
    Serial.println();
    http->stop();//shutdown
    Serial.println("HTTP POST disconnected");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();
  serialAT.begin(115200);
  delay(3000);
  Serial.println("Initaializing modem");
  modem.restart();
//modem.init();
Serial.println(modem.getModemInfo());
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (retry >= 2) {
    check_sim_on();
    Serial.println("Initalizing modem...");
    modem.restart();
    Serial.println(modem.getModemInfo());
    String modemInfo = modem.getModemInfo();
    
    Serial.print("Modem: ");
    Serial.println(modemInfo);
  }
  if (!modem.restart()) {
    Serial.println("SSL is not supported by this modem");
    retry++;
    delay(5000);
    return;
  }
  Serial.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    Serial.println("fail");
    delay(10000);
    return;
  }
  Serial.println(" Ok ");
  if (!modem.isGprsConnected())
  {
    Serial.println(F("Connecting to"));
    Serial.print(apn);
    if (!modem.gprsConnect(apn, user, pass)) {
      Serial.println("fail");
      delay(10000);
      return;
    }
    else Serial.println(" Ok ");
  }
  http_no_auth.connect(server.c_str(), port);
  while (true) {
    if (!http_no_auth.connected()) {//true when ssl
      Serial.println();
      http_no_auth.stop();
      Serial.println("HTTP not connect");
      break;
    }
    else {
      if (millis() - lastPublishMillis > publishInterval)
      {
        lastPublishMillis = millis();
        int sensorValue = analogRead(A0);
        String payload = "{\"d\"{\"adc\":";
        payload += String(sensorValue, DEC);
        payload += "}}";

        Serial.print("Sending payload: ");
        Serial.print(payload);
        String UPDATE_PATH = "/api/v0002/device/types/" DEVICE_TYPE "/devices/" DEVICE_ID "/events/" EVENT;
        httpPostNoAuth("PATCH", UPDATE_PATH, payload, &http_no_auth);
      }
    }
  }
}
