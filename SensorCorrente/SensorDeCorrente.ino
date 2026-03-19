#include "EmonLib.h"
#include <WiFiManager.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Instância do sensor de energia
EnergyMonitor emon1;

// ===================== CONFIGURAÇÕES =====================
// Configuração InfluxDB Cloud
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "gW38IcWzf7zOAqKbXMLQeHdNfQwc40WCmVtovwa420NXeQBj24DYY4iuNjm3vAoDvjpdlebtw7VL2IUsjCdJMQ=="
#define INFLUXDB_ORG "wattalyze"
#define INFLUXDB_BUCKET "Iot"  // ATUALIZADO: "IOT" → "Iot"

// Configurações do sensor
const float TENSAO_REDE = 220.0;
const float CALIBRACAO = 1.0;
const int PINO_CORRENTE = 34;

// Identificadores
#define MAC_CORRENTE "AA:BB:CC:DD:EE:03"
#define DEVICE_ID "sensor_corrente_01"
#define ENVIRONMENT_ID "ambiente_01"

// Intervalo de envio (ms)
#define SEND_INTERVAL 10000

// ===================== OBJETOS =====================
InfluxDBClient client(
  INFLUXDB_URL,
  INFLUXDB_ORG,
  INFLUXDB_BUCKET,  // Agora usando o bucket "Iot"
  INFLUXDB_TOKEN,
  InfluxDbCloud2CACert
);

// Measurement: "energy"
Point energyPoint("energy");

unsigned long lastSend = 0;
unsigned long tempo_anterior = 0;
double energia_total_kwh = 0;
bool primeira_leitura = true;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Monitor de Energia SCT-013-030 com EmonLib ===");

  // Conexão Wi-Fi
  WiFiManager wm;
  bool res = wm.autoConnect("wattalyze_energy", "wattalyze123");

  if (!res) {
    Serial.println("Falha ao conectar no WiFi");
    ESP.restart();
  } else {
    Serial.println("WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }

  // Inicializa o sensor
  emon1.current(PINO_CORRENTE, CALIBRACAO);

  // Configura tags fixas
  energyPoint.addTag("mac", MAC_CORRENTE);
  energyPoint.addTag("device_id", DEVICE_ID);
  energyPoint.addTag("environment_id", ENVIRONMENT_ID);

  // Testa conexão InfluxDB
  if (client.validateConnection()) {
    Serial.println("Conexão com InfluxDB estabelecida!");
  } else {
    Serial.print("Falha na conexão com InfluxDB: ");
    Serial.println(client.getLastErrorMessage());
  }

  tempo_anterior = millis();
  Serial.println("\n--- Iniciando leituras ---\n");
  Serial.println("Tempo   | Corrente (A) | Potência (W) | Energia (kWh)");
  Serial.println("-----------------------------------------------------");
}

void loop() {
  // Verifica conexão Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado! Reiniciando...");
    ESP.restart();
  }

  // Corrente RMS
  double corrente_rms = emon1.calcIrms(1480);

  // Potência instantânea
  double potencia_w = corrente_rms * TENSAO_REDE;

  // Cálculo da energia acumulada CORRIGIDO
  unsigned long tempo_atual = millis();
  
  if (!primeira_leitura) {
    // Calcula o tempo decorrido em horas
    unsigned long delta_tempo_ms = tempo_atual - tempo_anterior;
    double delta_tempo_horas = delta_tempo_ms / 3600000.0; // ms para horas
    
    // Calcula a energia consumida neste intervalo (em kWh)
    double energia_intervalo_kwh = (potencia_w * delta_tempo_horas) / 1000.0;
    energia_total_kwh += energia_intervalo_kwh;
    
    Serial.print("Delta tempo: "); Serial.print(delta_tempo_ms); Serial.print("ms | ");
    Serial.print("Energia intervalo: "); Serial.print(energia_intervalo_kwh, 10); Serial.println(" kWh");
  } else {
    primeira_leitura = false;
  }
  
  tempo_anterior = tempo_atual;

  // Exibe no Serial Monitor COM MAIS CASAS DECIMAIS
  unsigned long tempo_seg = millis() / 1000;
  int minutos = tempo_seg / 60;
  int segundos = tempo_seg % 60;

  Serial.printf("%02d:%02d | %.4f A    | %.3f W     | %.8f kWh\n",
                minutos, segundos, corrente_rms, potencia_w, energia_total_kwh);

  // Envia a cada intervalo definido
  unsigned long now = millis();
  if (now - lastSend >= SEND_INTERVAL) {
    lastSend = now;

    // Limpa campos anteriores
    energyPoint.clearFields();

    // Adiciona campos
    energyPoint.addField("current", corrente_rms);
    energyPoint.addField("instantaneous_power", potencia_w);
    energyPoint.addField("voltage", TENSAO_REDE);
    energyPoint.addField("consumption_kwh", energia_total_kwh);

    // DEBUG detalhado COM MAIS CASAS DECIMAIS
    Serial.println("=== DEBUG ENVIO INFLUXDB ===");
    Serial.printf("Current: %.4f A\n", corrente_rms);
    Serial.printf("Power: %.3f W\n", potencia_w);
    Serial.printf("Voltage: %.1f V\n", TENSAO_REDE);
    Serial.printf("Consumption: %.10f kWh\n", energia_total_kwh);
    Serial.println("============================");

    // Envia para InfluxDB
    Serial.print("Enviando dados para InfluxDB... ");
    if (client.writePoint(energyPoint)) {
      Serial.println("✓ Sucesso!");
    } else {
      Serial.print("✗ Erro: ");
      Serial.println(client.getLastErrorMessage());
    }
  }

  delay(1000); // Leitura a cada 1 segundo para cálculo mais preciso
}