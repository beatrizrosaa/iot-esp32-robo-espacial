#include <WiFi.h>
#include <PubSubClient.h>

// ---- Wi-Fi ----
const char* ssid = "CIMATEC-VISITANTE";
const char* password = "";


// ---- MQTT Broker ----
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

// ---- Pinos ----
const int joyX = 34;
const int joyY = 35;
const int buttonPin = 25;
const int ledVerde = 26;
const int ledVermelho = 27;

bool ligado = true;

void conectarWiFi() {
  Serial.print("Conectando-se ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nConectado ao WiFi!");
}

void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao Broker MQTT...");
    if (client.connect("ESP32ControleWokwi")) {
      Serial.println("conectado!");
      client.publish("/robo/comando", "Controle remoto conectado");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  digitalWrite(ledVerde, HIGH);
  digitalWrite(ledVermelho, LOW);

  conectarWiFi();
  client.setServer(mqtt_server, mqtt_port);
  conectarMQTT();
}

void loop() {
  if (!client.connected()) conectarMQTT();
  client.loop();

  int x = analogRead(joyX);
  int y = analogRead(joyY);

  String comando = "parar";
  if (digitalRead(buttonPin) == LOW) {
    ligado = false;
    comando = "desligar";
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledVermelho, HIGH);
    Serial.println("Comando enviado: DESLIGAR");
    client.publish("/robo/comandos", comando.c_str());
    delay(1000);
  }

  if (ligado) {
    if (x > 3500) comando = "DIREITA";
    else if (x < 1500) comando = "ESQUERDA";
    else if (y > 3500) comando = "FRENTE";
    else if (y < 1500) comando = "TRAS";

    client.publish("/robo/comandos", comando.c_str());
    Serial.println("Comando MQTT: " + comando);
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledVermelho, LOW);
  }

  delay(300);
}
