#define TINY_GSM_MODEM_SIM800 //Tipo de modem que estamos usando
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#include "DHT.h"

#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);


const char apn[] = "bsnlnet";//"internet"
const char user[] = "mms";
const char pass[] = "mms";


#define ORG "zv2k2t"
#define DEVICE_TYPE "hello"
#define DEVICE_ID "1234"
#define TOKEN "12345678"
#define EVENT "status"
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

//Token de usuário que pegamos no Ubidots
#define TOKEN "12345678"
 
//Tópico onde vamos postar os dados de temperatura e umidade (modemGSM32_gprs é o nome do dispositivo no Ubidots)
#define TOPIC "iot-2/evt/Data/fmt/json"
 
//id do dispositivo que pegamos no painel do Ubidots
#define DEVICE_ID "1234"
 
//URL do MQTT Server
char MQTT_SERVER[] = ORG ".messaging.internetofthings.ibmcloud.com";
 
//Porta padrão do MQTT
#define MQTT_PORT 1883
 
//Intervalo entre os envios e refresh da tela
#define INTERVAL 10000
 
//Canal serial que vamos usar para comunicarmos com o modem. Utilize sempre 1
#include <SoftwareSerial.h>
SoftwareSerial SerialGSM(6, 7); // RX, TX
TinyGsm modemGSM(SerialGSM);
TinyGsmClient gsmClient(modemGSM);
 
//Cliente MQTT, passamos a url do server, a porta
//e o cliente GSM
PubSubClient client(MQTT_SERVER, MQTT_PORT, gsmClient);
 
//Tempo em que o último envio/refresh foi feito
uint32_t lastTime = 0;


void setup() 
{
  Serial.begin(9600);
  setupGSM(); //Inicializa e configura o modem GSM
  connectMQTTServer(); //Conectamos ao mqtt server
  Serial.println("DHTxx test!");

  dht.begin();
  //Espera 2 segundos e limpamos o display
  delay(2000);
}

void setupGSM()
{
  Serial.println("Setup GSM...");
 SerialGSM.begin(9600);
  delay(3000);
 
  //Mostra informação sobre o modem
  Serial.println(modemGSM.getModemInfo());
 
  //Inicializa o modem
  if (!modemGSM.restart())
  {
    Serial.println("Restarting GSM Modem failed");
    delay(10000);
    modemGSM.restart();
    return;
  }
 
  //Espera pela rede
  if (!modemGSM.waitForNetwork()) 
  {
    Serial.println("Failed to connect to network");
    delay(10000);
    modemGSM.restart();
    return;
  }
 
  //Conecta à rede gprs (APN, usuário, senha)
  if (!modemGSM.isGprsConnected())
  {
    Serial.println(F("Connecting to"));
    Serial.print(apn);
  if (!modemGSM.gprsConnect(apn, "", "")) {
    Serial.println("GPRS Connection Failed");
    delay(10000);
    modemGSM.restart();
    return;
  }
  else Serial.println(" Ok ");
  }
 
  Serial.println("Setup GSM Success");
}

void connectMQTTServer() {
  Serial.println("Connecting to MQTT Server...");
  //Se conecta ao device que definimos
  if (client.connect(clientId, authMethod, token)) {
    //Se a conexão foi bem sucedida
    Serial.println("Connected");
  } else {
    //Se ocorreu algum erro
    Serial.print("error = ");
    Serial.println(client.state());
    delay(10000);
    modemGSM.restart();
  }
}

void loop() 
{
  //Faz a leitura da umidade e temperatura
//  readDHT(); 
 
  //Se desconectou do server MQTT
  if(!client.connected())
  {
    //Mandamos conectar
    connectMQTTServer();
  }
 
   //Tempo decorrido desde o boot em milissegundos
  unsigned long now = millis();
 
  //Se passou o intervalo de envio
  if(now - lastTime > INTERVAL)
  {
    //Publicamos para o server mqtt
    publishMQTT();
    //Mostramos os dados no display
    //Atualizamos o tempo em que foi feito o último envio
    lastTime = now;
  }
}

void publishMQTT()
{
  //Cria o json que iremos enviar para o server MQTT
  String msg = createJsonString();
  Serial.print("Publish message: ");
  Serial.println(msg);
  //Publicamos no tópico
  int status = client.publish(TOPIC, msg.c_str());
  Serial.println("Status: " + String(status));//Status 1 se sucesso ou 0 se deu erro
}


String createJsonString() 
{  
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  /*if (isnan(h) || isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }*/
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  
  String data = "{";
      data+="\"humidity\":";
      data+=String(h, 2);
      data+=",";
      data+="\"temperature\":";
      data+=String(t, 2);
      data+="}";
  return data;
}
