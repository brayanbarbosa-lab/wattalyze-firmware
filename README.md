<div align="center">

# ⚡ WATTALYZE — Firmware

### Firmware ESP32 para Monitoramento de Energia e Temperatura IoT

[![ESP32](https://img.shields.io/badge/ESP32-Espressif-E7352C?style=for-the-badge&logo=espressif&logoColor=white)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/Arduino_IDE-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![InfluxDB](https://img.shields.io/badge/InfluxDB-Cloud-22ADF6?style=for-the-badge&logo=influxdb&logoColor=white)](https://www.influxdata.com/)
[![WiFi](https://img.shields.io/badge/WiFi-Conectado-00B4D8?style=for-the-badge&logo=wifi&logoColor=white)]()

> Firmware desenvolvido para os módulos físicos do WATTALYZE — conecta sensores IoT à nuvem via Wi-Fi e envia dados em tempo real para o InfluxDB Cloud.

</div>

---

## 📋 Sobre o Firmware

Este repositório contém o código-fonte embarcado para os dois módulos físicos do **WATTALYZE**. O firmware foi desenvolvido para **ESP32** utilizando a **Arduino IDE** e é responsável por coletar dados dos sensores e enviá-los ao **InfluxDB Cloud** via Wi-Fi a cada 10 segundos.

> 🎓 Projeto desenvolvido como **TCC do 3º ano** do curso de Desenvolvimento de Sistemas na **ETEC**.

---

## 🔧 Módulos de Hardware

### 📦 Módulo 1 — Medidor de Consumo de Energia (`sensor_corrente`)
Mede corrente, potência e energia acumulada em tempo real de forma **não invasiva**.

| Componente | Função |
|---|---|
| **ESP32** | Microcontrolador principal com Wi-Fi integrado |
| **SCT-013-000** | Sensor de corrente não invasivo (até 100A) |
| **Conversor 100-240V → 5V** | Alimentação direta da rede elétrica |

**Dados enviados ao InfluxDB:**
- `current` — Corrente RMS (A)
- `instantaneous_power` — Potência instantânea (W)
- `voltage` — Tensão da rede (V)
- `consumption_kwh` — Energia acumulada (kWh)

---

### 📦 Módulo 2 — Monitor Ambiental (`sensor_dht11`)
Monitora temperatura e umidade do ambiente, com exibição em **display LCD I2C**.

| Componente | Função |
|---|---|
| **ESP32** | Microcontrolador principal com Wi-Fi integrado |
| **DHT11** | Sensor de temperatura e umidade |
| **Display LCD I2C 16x2** | Exibe leituras localmente em tempo real |
| **Conversor 100-240V → 5V** | Alimentação direta da rede elétrica |

**Dados enviados ao InfluxDB:**
- `temperature` — Temperatura (°C)
- `humidity` — Umidade relativa (%)

> ⚠️ Ambos os módulos possuem **case físico próprio** desenvolvido pela equipe.

---

## ⚙️ Como Funciona

```
Sensor (SCT-013 / DHT11)
        ↓
      ESP32
        ↓ Wi-Fi (a cada 10s)
   InfluxDB Cloud
        ↓
   Plataforma WATTALYZE
```

1. O ESP32 conecta automaticamente ao Wi-Fi via **WiFiManager**
2. Lê os dados do sensor continuamente
3. A cada **10 segundos** envia os dados para o **InfluxDB Cloud**
4. A plataforma web consome os dados e exibe em tempo real

---

## 🛠️ Como Usar

### Pré-requisitos

- [Arduino IDE](https://www.arduino.cc/en/software) instalada
- Suporte ao ESP32 instalado na Arduino IDE → [tutorial aqui](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)

### 📦 Bibliotecas Necessárias

Instale pela Arduino IDE em **Sketch → Incluir Biblioteca → Gerenciar Bibliotecas**:

| Biblioteca | Módulo |
|---|---|
| **EmonLib** | Sensor de Corrente |
| **DHT sensor library** (Adafruit) | Monitor Ambiental |
| **Adafruit Unified Sensor** | Monitor Ambiental |
| **LiquidCrystal I2C** | Monitor Ambiental (LCD) |
| **WiFiManager** (tzapu) | Ambos |
| **InfluxDB Client for Arduino** | Ambos |

### ⚙️ Configuração

Antes de gravar o firmware, edite as seguintes constantes no arquivo `.ino`:

```cpp
// InfluxDB Cloud
#define INFLUXDB_URL   "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "seu_token_influxdb"
#define INFLUXDB_ORG   "wattalyze"
#define INFLUXDB_BUCKET "Iot"

// Identificadores do dispositivo
#define MAC_CORRENTE   "AA:BB:CC:DD:EE:XX"  // MAC único do dispositivo
#define DEVICE_ID      "sensor_corrente_01"
#define ENVIRONMENT_ID "ambiente_01"
```

> ⚠️ Cada dispositivo deve ter um `DEVICE_ID` e `MAC` únicos, correspondentes ao cadastro na plataforma.

### 🔌 Configuração do Wi-Fi

O firmware usa o **WiFiManager** — na primeira vez que ligar o ESP32, ele cria uma rede Wi-Fi chamada:

- **Módulo Energia:** `wattalyze_energy`
- **Módulo Ambiental:** `wattalyze_dht`

Conecte nessa rede pelo celular ou computador, acesse `192.168.4.1` e configure sua rede Wi-Fi. O ESP32 salva as credenciais e conecta automaticamente nas próximas vezes.

### ▶️ Gravando o Firmware

1. Abra o arquivo `.ino` desejado na **Arduino IDE**
2. Selecione a placa: **Tools → Board → ESP32 Dev Module**
3. Selecione a porta COM correta em **Tools → Port**
4. Clique em **Upload** (→)
5. Abra o **Monitor Serial** (115200 baud) para acompanhar os logs

---

## 📁 Estrutura do Repositório

```
wattalyze-firmware/
├── sensor_corrente/
│   └── sensor_corrente.ino   # Módulo medidor de energia (SCT-013)
└── sensor_dht11/
    └── sensor_dht11.ino      # Módulo ambiental (DHT11 + LCD)
```

---

## 🔮 Roadmap

- [ ] Controle remoto via relé (desligar equipamentos pela plataforma)
- [ ] Módulo com display para o sensor de corrente
- [ ] Versão com bateria para ambientes sem tomada
- [ ] Suporte a MQTT para comunicação mais eficiente

---

## 🗺️ Repositórios

| Repositório | Descrição |
|---|---|
| **[wattalyze](https://github.com/brayanbarbosa-lab/wattalyze)** | API e plataforma web |
| **[wattalyze-firmware](https://github.com/brayanbarbosa-lab/wattalyze-firmware)** | Firmware ESP32 (este repositório) |

---

## 👨‍💻 Equipe

Desenvolvido por alunos do 3º ano de Desenvolvimento de Sistemas da **ETEC**:

| Nome |
|---|
| **Brayan Barbosa Dos Santos** |
| **Samuel Matos Gabriel** |
| **Bianca Da Silva Tavares** |
| **Gabriel Carlos Barbosa** |
| **Miguel Neves Duarte** |

---

## 📄 Licença

MIT License — veja o arquivo [LICENSE](LICENSE) para mais detalhes.

---

<div align="center">

Feito com ⚡ pela equipe **WATTALYZE** — ETEC 2024

</div>
