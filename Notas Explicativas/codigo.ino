#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHT.h"

// Inicializa a conexão Wifi com o roteador
//Informações de login da rede
#define WIFI_SSID "sua_rede_aqui"
#define WIFI_PASSWORD "sua_senha_aqui"
//Token do bot
#define BOTtoken "000000000:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // Seu token do telegram

#define LED_PIN D3
#define RELAY_PIN D6
#define DHT_PIN D5
#define DHTTYPE DHT11
//Definição dos pinos do LDR e LED
const int LED_PIN_LDR = 15; //D8
const int LDR_PIN = 5; //D1

#define BOT_SCAN_MESSAGE_INTERVAL 1000 //Intervalo para obter novas mensagens (1000 ms = 1 s)
long lastTimeScan;  // Ultima vez que buscou mensagem
bool ledStatus = false; // Estado do LED
bool relayStatus = false; // Estado do Relê

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
DHT dht(DHT_PIN, DHTTYPE);
//Tempo em que foi feita a última checagem
uint32_t lastCheckTime = 0;

//Quantidade de usuários que podem interagir com o bot (Deixa o bot seguro)
#define SENDER_ID_COUNT 2 //Ids dos usuários que podem interagir com o bot. 
//É possível verificar seu id pelo monitor serial ao enviar uma mensagem para o bot
String validSenderIds[SENDER_ID_COUNT] = {"id_aqui", "id_aqui"};

// Trata as novas mensagens que chegaram
void handleNewMessages(int numNewMessages) {
  Serial.println("Tratando novas Mensagens");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    // Pessoa que está enviando a mensagem
    String from_name = bot.messages[i].from_name;    
    if (from_name == "") from_name = "Convidado";

    // Tratamento para cada tipo de comando a seguir.

    if (text == "/ledligado") {
      digitalWrite(LED_PIN, HIGH);   // liga o LED (HIGH é o nível da voltagem)
      ledStatus = true;
      bot.sendMessage(chat_id, "Led está ligado", "");
    }

    if (text == "/leddesligado") {
      ledStatus = false;
      digitalWrite(LED_PIN, LOW);    // desliga o LED (LOW é o nível da vontagem)
      bot.sendMessage(chat_id, "Led está desligado", "");
    }

    if (text == "/relayligado") {
      digitalWrite(RELAY_PIN, HIGH);
      relayStatus = true;
      bot.sendMessage(chat_id, "Relê está ligado", "");
    }

    if (text == "/relaydesligado") {
      relayStatus = false;
      digitalWrite(RELAY_PIN, LOW);
      bot.sendMessage(chat_id, "Relê está desligado", "");
    }

    if (text == "/status") {
      String message = "Led está ";
      if(ledStatus){
        message += "ligado";
      }else{
        message += "desligado";
      }
      message += ". \n";
      message += "Relê está ";
      if(relayStatus){
        message += "ligado";
      }else{
         message += "desligado";
      }
      message += ". \n";
      bot.sendMessage(chat_id, message, "Markdown");      
    }

    if( text == "/ambiente") {
      float humidity = dht.readHumidity();
      float temperature = dht.readTemperature();
      String message = "A temperatura é de " + String(temperature, 2) + " graus celsius.\n";
      message += "A umidade relativa do ar é de " + String(humidity, 2)+ "%.\n\n";      
      bot.sendMessage(chat_id, message, "Markdown");
    }

    // Cria teclado com as opções de comando
    if (text == "/opcoes") {
      String keyboardJson = "[[\"/ledligado\", \"/leddesligado\"],[\"/relayligado\", \"/relaydesligado\"],[\"/ambiente\",\"/status\"],[\"/opcoes\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Escolha uma das opções", "", keyboardJson, true);
    }
    
    // Comando de inicio de conversa no telegram
    if (text == "/start") {
      String welcome = from_name + ", bem vindo ao Bot do Projeto Integrador II.\n";
      welcome += "Para interagir com a casa, use um dos comandos a seguir.\n\n";
      welcome += "/ledligado : para ligar o Led \n";
      welcome += "/leddesligado : para desligar o Led \n";
      welcome += "/relayligado : para ligar o Relê \n";
      welcome += "/relaydesligado : para desligar o Relê \n";
      welcome += "/ambiente : saber a temperatura e umidade do ambiente \n";
      welcome += "/status : para saber o status dos sensores e atuadores \n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

void setupWifi(){  

  // Tentativa de conexão com o Wifi:
  Serial.print("Conectando o Wifi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Endereço de IP: ");
  Serial.println(WiFi.localIP());
}

void setupPins(){
  pinMode(LED_PIN, OUTPUT); 
  pinMode(RELAY_PIN, OUTPUT); 
  delay(10);
  digitalWrite(LED_PIN, LOW); 
  digitalWrite(RELAY_PIN, LOW); 

  dht.begin();
}

void setup() {
  pinMode(LED_PIN_LDR, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  client.setInsecure();
  Serial.begin(115200); // Velocidade do monitor

  setupWifi();
  setupPins();

  lastTimeScan = millis();
}

void loop() {
// Verificação do LDR (sensor passivo)
int ldrStatus = digitalRead(LDR_PIN);

  if (ldrStatus == 1) {

  digitalWrite(LED_PIN_LDR, LOW);
  //Serial.println("Sem presença de luz no LDR, LED está ligado"); // Mensagem enviada ao monitor (controle de funcionamento)
  //Serial.print(ldrStatus);

  }

  else {

  digitalWrite(LED_PIN_LDR, HIGH);
  //Serial.println("LED está desligado"); // Mensagem enviada ao monitor (controle de funcionamento)
  //Serial.println(ldrStatus);

  }
  // Bot do Telegram
  if (millis() > lastTimeScan + BOT_SCAN_MESSAGE_INTERVAL)  {
    Serial.print("Checando comandos - ");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);  
    Serial.println(numNewMessages);
    while(numNewMessages) {
      Serial.println("Leitura de comando efetuada");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeScan = millis();
  }
  yield();
  delay(10);
}
