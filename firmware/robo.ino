#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include "DHT.h"
#include <ArduinoJson.h>

// --- CONFIGURAÇÕES DE REDE E MQTT ---
// As credenciais de Wi-Fi serão solicitadas via Monitor Serial
String ssid;
String password;
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic_command = "robo/comandos";

// --- CONFIGURAÇÕES DO CALLMEBOT ---
String phoneNumber = "5571999291712"; // Ex: 55719...
String apiKey = "7113460";

// --- CONFIGURAÇÕES DO BACKEND (ETAPA 3) ---
// !!! IMPORTANTE: Substitua pelo IP do seu computador !!!
const char* backend_server = "http://172.23.224.1:5000/leituras";

// --- FUNÇÃO PARA ENVIAR ALERTA NO WHATSAPP ---


// --- FUNÇÃO PARA ENVIAR DADOS PARA O BACKEND (ETAPA 3)  ---
void enviarDadosParaBackend(float temp, float hum, int luz, bool pres, float prob) {

  // Cria um documento JSON
  StaticJsonDocument<256> doc;

  // Adiciona os valores ao JSON [cite: 213, 214, 220, 221, 222]
  doc["temperatura_c"] = temp;
  doc["umidade_pct"] = hum;
  doc["luminosidade"] = luz;
  doc["presenca"] = pres ? 1 : 0; // Converte booleano para 1 ou 0
  doc["probabilidade_vida"] = prob;

  // Serializa o JSON para uma string
  String payload;
  serializeJson(doc, payload);

  // Envia os dados via HTTP POST
  HTTPClient http;
  http.begin(backend_server);
  http.addHeader("Content-Type", "application/json");

  Serial.println("Enviando dados dos sensores para o Backend...");
  Serial.println(payload);

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode == 201) { // 201 significa "Criado com sucesso"
    Serial.println("Dados enviados ao backend com sucesso!");
  } else {
    Serial.print("Erro ao enviar dados para o backend! Código: ");
    Serial.println(httpResponseCode);
    // O edital sugere um buffer aqui [cite: 224]
    // (Implementar um buffer é um passo avançado, 
    // por enquanto apenas registramos o erro)
  }

  http.end();
}

// --- MAPEAMENTO DE PINOS ---
#define DHTTYPE DHT11
#define DHTpin 9
const int pirPin = 1;
const int LDRpin = 2;
const int ledRedPin = 3;
const int ledGreenPin = 4;
const int servoRPin = 5; // Motor Direito
const int servoLPin = 6; // Motor Esquerdo

// --- OBJETOS E VARIÁVEIS GLOBAIS ---
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTpin, DHTTYPE);
Servo servoR;
Servo servoL;

bool systemEnabled = true; // Robô começa ligado
unsigned long lastMsg = 0; // Para o timer do loop principal

// Protótipos das funções
void callback(char* topic, byte* payload, unsigned int length);
void controlMotors(String command);
void sendWhatsAppAlert();

/**
 * @brief Procura por redes Wi-Fi, lista no Monitor Serial e pede ao usuário para selecionar uma e inserir a senha.
 */
void setup_wifi_interactive() {
    delay(10);
    Serial.println("\nProcurando redes Wi-Fi...");
    
    // Coloca o ESP32 em modo "Station" (cliente) e inicia a busca
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int n = WiFi.scanNetworks();
    Serial.println("Busca concluída!");

    if (n == 0) {
        Serial.println("Nenhuma rede Wi-Fi encontrada.");
        // Loop infinito, pois não pode continuar sem Wi-Fi
        while(true);
    } else {
        Serial.print(n);
        Serial.println(" redes encontradas:");
        for (int i = 0; i < n; ++i) {
            // Imprime o número da rede e seu nome (SSID)
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(WiFi.SSID(i));
            delay(10);
        }
    }

    // Pede ao usuário para escolher a rede
    Serial.println("\nDigite o número da rede à qual deseja se conectar:");
    while (Serial.available() == 0) {
        // Aguarda a entrada do usuário
    }
    String network_choice_str = Serial.readStringUntil('\n');
    int network_index = network_choice_str.toInt() - 1; // Converte para número e ajusta o índice

    if (network_index < 0 || network_index >= n) {
        Serial.println("Número inválido. Reiniciando...");
        delay(1000);
        ESP.restart();
    }
    
    ssid = WiFi.SSID(network_index);

    // Pede ao usuário para digitar a senha
    Serial.print("Você selecionou a rede: ");
    Serial.println(ssid);
    Serial.println("Digite a senha para esta rede:");
    while (Serial.available() == 0) {
        // Aguarda a entrada do usuário
    }
    password = Serial.readStringUntil('\n');
    password.trim(); // Remove espaços em branco extras

    // Tenta se conectar
    Serial.println("\nConectando...");
    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi conectado!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
}


void reconnect() {
    while (!client.connected()) {
        Serial.print("Tentando conectar ao MQTT...");
        if (client.connect("ESP32_Robo_Explorador")) {
            Serial.println("conectado!");
            client.subscribe(mqtt_topic_command);
        } else {
            Serial.print("falhou, rc=");
            Serial.print(client.state());
            Serial.println(" tentando novamente em 5 segundos");
            delay(5000);
        }
    }
}

// --- FUNÇÃO DE CALLBACK: RECEBE OS COMANDOS DO CONTROLE REMOTO ---
void callback(char* topic, byte* payload, unsigned int length) {
    String command;
    for (int i = 0; i < length; i++) {
        command += (char)payload[i];
    }
    Serial.print("Comando recebido: ");
    Serial.println(command);

    if (command == "DESLIGAR") {
        systemEnabled = false;
        controlMotors("Parado"); // Para os motores imediatamente
    } else {
        systemEnabled = true;
        controlMotors(command);
    }
}

// --- CONTROLE DOS MOTORES ---
void controlMotors(String command) {
    if (command == "FRENTE") {
        servoL.write(180);
        servoR.write(0);
    } else if (command == "TRAS") {
        servoL.write(0);
        servoR.write(180);
    } else if (command == "ESQUERDA") {
        servoL.write(90);
        servoR.write(0);
    } else if (command == "DIREITA") {
        servoL.write(180);
        servoR.write(90);
    } else if (command == "PARADO") {
        servoL.write(90);
        servoR.write(90);
    }
}

// --- FUNÇÃO PARA ENVIAR ALERTA NO WHATSAPP ---
void sendWhatsAppAlert() {
    String message = "Alerta! Alta probabilidade de vida detectada no planeta.";
    String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
        Serial.println("Mensagem de alerta enviada com sucesso!");
    } else {
        Serial.print("Erro ao enviar mensagem de alerta! Código: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}


void setup() {
    Serial.begin(115200);
    
    pinMode(pirPin, INPUT);
    pinMode(ledRedPin, OUTPUT);
    pinMode(ledGreenPin, OUTPUT);
    pinMode(LDRpin, INPUT);

    dht.begin();
    
    servoL.attach(servoLPin);
    servoR.attach(servoRPin);
    controlMotors("Parado");// Garante que os motores comecem parados

    // Chama a nova função interativa de conexão Wi-Fi
    setup_wifi_interactive();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 2000) {
        lastMsg = now;

        // --- LEITURA DOS SENSORES ---
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();
        int lightValue = analogRead(LDRpin);
        bool presenceState = digitalRead(pirPin);
        int probability = 0;

        // --- CÁLCULO DA PROBABILIDADE DE VIDA ---
        if (systemEnabled) {
            if (temperature >= 15 && temperature <= 30) {
                probability += 25;
            }
            if (humidity >= 40 && humidity <= 70) {
                probability += 25;
            }
            if (lightValue > 2000) {  
                probability += 20;
            }
            if (presenceState) {
                probability += 30;
            }
        }
        
        // --- LÓGICA DE DECISÃO E ATUADORES ---
        Serial.println("--- Relatório do Robô ---");
        Serial.print("Temperatura: "); Serial.print(temperature); Serial.println(" °C");
        Serial.print("Umidade: "); Serial.print(humidity); Serial.println(" %");
        Serial.print("Intensidade da Luz: "); Serial.println(lightValue);
        Serial.print("Sensor de Presença: "); Serial.println(presenceState ? "Presença detectada" : "Sem presença");
        Serial.print("Probabilidade de Vida: "); Serial.print(probability); Serial.println(" %");

        // --- ENVIO DOS DADOS PARA O BACKEND (ETAPA 3)  ---
        // Envia os dados lidos para o servidor Python
        // (Verifica se os dados são válidos antes de enviar, 
        //   sensores DHT às vezes falham a leitura "nan")
        if (!isnan(temperature) && !isnan(humidity)) {
        enviarDadosParaBackend(temperature, humidity, lightValue, presenceState, probability);
        } else {
        Serial.println("Leitura inválida dos sensores, não enviando ao backend.");
        }

        if (!systemEnabled) {
            digitalWrite(ledGreenPin, LOW);
            digitalWrite(ledRedPin, HIGH);
            Serial.println("Estado do Robô: DESLIGADO");
        } else if (probability > 75) {
            digitalWrite(ledGreenPin, LOW);
            digitalWrite(ledRedPin, HIGH);
            Serial.println("Estado do Robô: ALERTA! Alta probabilidade de vida detectada!");
            sendWhatsAppAlert();
        } else {
            digitalWrite(ledGreenPin, HIGH);
            digitalWrite(ledRedPin, LOW);
            Serial.println("Estado do Robô: Exploração normal. Nenhum indício relevante detectado.");
        }
        Serial.println("---------------------------\n");
    }
}
