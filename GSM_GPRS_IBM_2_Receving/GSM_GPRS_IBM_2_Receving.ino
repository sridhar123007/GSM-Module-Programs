#define TINY_GSM_MODEM_SIM800 //Tipo de modem que estamos usando
//#define TINY_GSM_MODEM_SIM900
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#include "DHT.h"

#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);
#define LED_PIN 13
int ledStatus = LOW;


const char apn[] = "airtelgprs.com";//"internet"
const char user[] = "mms";
const char pass[] = "mms";


#define ORG "zv2k2t"
#define DEVICE_TYPE "hello"
#define DEVICE_ID "1234"
#define TOKEN "12345678"
//#define EVENT "status"
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

//Token de usuário que pegamos no Ubidots
#define TOKEN "12345678"
 
//Tópico onde vamos postar os dados de temperatura e umidade (modemGSM32_gprs é o nome do dispositivo no Ubidots)
#define TOPIC "iot-2/evt/Data/fmt/json"
char recvtopic[] = "iot-2/cmd/home/fmt/String";// cmd  REPRESENT command type AND COMMAND IS TEST OF FORMAT STRING

String command;
//String data="";
 
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


//void callback(char* recvtopic, byte* payload, unsigned int payloadLength); 
void mqttCallback(char* recvtopic, byte* payload, unsigned int len);
//Cliente MQTT, passamos a url do server, a porta
//e o cliente GSM
PubSubClient client(MQTT_SERVER, MQTT_PORT, mqttCallback, gsmClient);
 
//Tempo em que o último envio/refresh foi feito
uint32_t lastTime = 0;
 
float humidity; //Variável onde iremos armazenar o valor da umidade
float temperature; //Variável onde iremos armazenar o valor da temperatura

#define led1 3
#define led2 4

void setup() 
{
  Serial.begin(9600);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  setupGSM(); //Inicializa e configura o modem GSM
  connectMQTTServer(); //Conectamos ao mqtt server
  Serial.println("DHTxx test!");
  initManagedDevice();
  dht.begin();
  //Espera 2 segundos e limpamos o display
  delay(2000);
  //client.setCallback(mqttCallback);
}

void setupGSM()
{
  Serial.println("Setup GSM...");
  //Inicializamos a serial onde está o modem
 //SerialGSM.begin(9600, SERIAL_8N1, 4, 2, false);
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
  initManagedDevice();
}

void loop() 
{
 
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
    //publishMQTT();
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
//subscribing 
void initManagedDevice() {
  if (client.subscribe(recvtopic)) {
   
    Serial.println("subscribe to cmd OK");
  } else {
    Serial.println("subscribe to cmd FAILED");
  }
}

/*void callback(char* recvtopic, byte* payload, unsigned int payloadLength) {
  
  Serial.print("callback invoked for recvtopic: ");
  Serial.println(recvtopic);

  for (int i = 0; i < payloadLength; i++) {
    
    command+= (char)payload[i];
  }
  
  Serial.print("data: "+ command);
  control_func();
  command= "";
}

void control_func()
{
  
   
  if(command== "lightoff")
 {

 digitalWrite(led1,LOW);
      digitalWrite(led2,LOW);
     Serial.println(".......lights are off..........");
    
  }
  else if(command== "lighton")
  {
     digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
     Serial.println(".......lights are on..........");

  }
}*/

void mqttCallback(char* recvtopic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(recvtopic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();

  // Only proceed if incoming message's topic matches
  if (String(recvtopic) == "lighton") {
    ledStatus = !ledStatus;
    //mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
  }
}
