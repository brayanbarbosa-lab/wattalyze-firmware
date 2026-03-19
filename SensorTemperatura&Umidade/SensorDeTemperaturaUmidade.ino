// Inclusão das bibliotecas necessárias
#include <DHT.h>                      // Biblioteca para o sensor DHT (leitura de temperatura/umidade)
#include <Adafruit_Sensor.h>          // Biblioteca base Adafruit (necessária para o DHT funcionar com padrão unificado)
#include <WiFiManager.h>              // Gerenciamento WiFi
#include <InfluxDbClient.h>           // Cliente InfluxDB
#include <InfluxDbCloud.h>            // Suporte InfluxDB Cloud

// ===================== CONFIGURAÇÕES INFLUXDB =====================
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "gW38IcWzf7zOAqKbXMLQeHdNfQwc40WCmVtovwa420NXeQBj24DYY4iuNjm3vAoDvjpdlebtw7VL2IUsjCdJMQ=="
#define INFLUXDB_ORG "wattalyze"
#define INFLUXDB_BUCKET "Iot"

// Identificadores
#define MAC_DHT "AA:BB:CC:DD:EE:04"           // MAC diferente do sensor de corrente
#define DEVICE_ID "sensor_dht11_01"
#define ENVIRONMENT_ID "ambiente_01"

// Intervalo de envio (ms)
#define SEND_INTERVAL 10000

// ===================== CONFIGURAÇÕES SENSOR =====================
#define PINODHT 12                     // Define o pino digital onde o sensor DHT está conectado
#define TIPODHT DHT11                  // Define o tipo de sensor: DHT11

// Criação do objeto do sensor DHT
DHT dht(PINODHT, TIPODHT);

// ===================== OBJETOS INFLUXDB =====================
InfluxDBClient client(
  INFLUXDB_URL,
  INFLUXDB_ORG,
  INFLUXDB_BUCKET,
  INFLUXDB_TOKEN,
  InfluxDbCloud2CACert
);

// Measurement: "environment"
Point environmentPoint("environment");

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Monitor de Temperatura e Umidade DHT11 ===");
  Serial.println("Iniciando...");

  // Inicializa o sensor DHT
  dht.begin();

  // Conexão Wi-Fi com WiFiManager
  Serial.println("Conectando ao WiFi...");
  WiFiManager wm;
  
  // Descomente a linha abaixo para resetar configurações WiFi (útil para testes)
  // wm.resetSettings();
  
  bool res = wm.autoConnect("wattalyze_dht", "wattalyze123");

  if (!res) {
    Serial.println("Falha ao conectar no WiFi");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }

  // Configura tags fixas
  environmentPoint.addTag("mac", MAC_DHT);
  environmentPoint.addTag("device_id", DEVICE_ID);
  environmentPoint.addTag("environment_id", ENVIRONMENT_ID);

  // Testa conexão InfluxDB
  Serial.print("Conectando ao InfluxDB... ");
  if (client.validateConnection()) {
    Serial.println("OK!");
  } else {
    Serial.print("Falha: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("\n--- Iniciando leituras ---\n");
  Serial.println("Temperatura (°C) | Umidade (%) | Status");
  Serial.println("----------------------------------------");
}

void loop() {
  // Verifica conexão Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado! Reiniciando...");
    delay(1000);
    ESP.restart();
  }

  // Leitura do sensor DHT
  float temperatura = dht.readTemperature();  // Lê a temperatura em graus Celsius
  float umidade = dht.readHumidity();         // Lê a umidade relativa do ar

  // Verifica se houve falha na leitura
  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro na leitura do DHT11!");
    delay(2000);
    return;
  }

  // Mostra no Serial Monitor
  Serial.printf("     %.1f        |    %.1f      | ", temperatura, umidade);

  // Envia a cada intervalo definido
  unsigned long now = millis();
  if (now - lastSend >= SEND_INTERVAL) {
    lastSend = now;

    // Limpa campos anteriores
    environmentPoint.clearFields();

    // Adiciona campos
    environmentPoint.addField("temperature", temperatura);
    environmentPoint.addField("humidity", umidade);

    // Envia para InfluxDB
    if (client.writePoint(environmentPoint)) {
      Serial.println("✓ Enviado");
    } else {
      Serial.print("✗ Erro: ");
      Serial.println(client.getLastErrorMessage());
    }
  } else {
    Serial.println("Aguardando...");
  }

  delay(2000); // Aguarda 2 segundos antes da próxima leitura
}