# FegSat – CubeSat e Balão Simulado

Repositório com os códigos de firmware usados na simulação da comunicação entre o **satélite principal (CubeSat)** e um **balão simulado**, desenvolvidos pela equipe FegSat (Unesp Guaratinguetá).

## Visão geral

O projeto simula um cenário em que um balão se comunica com o satélite. Como nem sempre temos um balão real nos testes, o balão é representado por um **ESP32** rodando um código próprio. O satélite coleta dados de vários sensores, salva tudo em um cartão SD e envia os pacotes por Wi-Fi.

```
 ┌─────────────────┐    pacotes    ┌─────────────────┐
 │  Balão simulado │ ◄───────────► │    CubeSat      │
 │     (ESP32)     │     Wi-Fi     │  (geral_esp32)  │
 └─────────────────┘               └─────────────────┘
```

## Estrutura do repositório

```
.
├── README.md
├── .gitignore
├── balao_zero_debris/
│   └── balao_zero_debris.ino    # Código do balão simulado
├── geral_esp32/
│   └── geral_esp32.ino          # Código do satélite principal
└── cod_zero_debris/
    └── cod_zero_debris.ino      # Código de teste: satélite → balão
```

> A IDE do Arduino exige que cada `.ino` fique dentro de uma pasta com o mesmo nome. Mantenha a estrutura acima ao baixar ou mover os arquivos.

## Descrição dos códigos

### `geral_esp32/` – Satélite principal
Firmware executado em um **ESP32**. A cada **30 segundos** ele:

1. Lê os sensores (pressão, temperatura, umidade, UV, qualidade do ar, aceleração e giroscópio).
2. Monta um pacote em **JSON**.
3. Envia o pacote por **HTTP POST** ao servidor via Wi-Fi.
4. Salva a mesma linha JSON no cartão SD (`/dados_missao.txt`).

Os valores brutos do MS5611 (D1 e D2) são lidos continuamente em segundo plano e enviados no pacote.

### `balao_zero_debris/` – Balão simulado
Código do **balão simulado**, executado em um ESP32. Ele representa o balão na comunicação com o satélite: [descreva o que o balão faz: recebe pacotes, responde, registra dados, atua como servidor etc.].

### `cod_zero_debris/` – Código de teste
Código de teste para ser gravado no satélite. Sua função é **enviar um pacote ao balão**, permitindo validar a comunicação entre os dois lados sem rodar todo o firmware principal.

## Sensores e módulos (satélite)

| Sensor / módulo | Grandeza medida | Interface | Pinos / endereço |
|---|---|---|---|
| **MS5611** | Pressão e temperatura (valores brutos D1/D2, OSR 4096) | I2C (Wire) | Endereço `0x77` |
| **BME680** | Temperatura, pressão e umidade | I2C (Wire) | SDA 21 / SCL 22 |
| **AHT10** | Temperatura e umidade | I2C (Wire) | SDA 21 / SCL 22 |
| **MPU6050** | Acelerômetro e giroscópio (3 eixos) | I2C (Wire) | SDA 21 / SCL 22 |
| **SGP30** | eCO₂ e TVOC (qualidade do ar) | I2C (segundo barramento, Wire2) | SDA 26 / SCL 25 |
| **ML8511** | Radiação UV | Analógica | UVOUT: GPIO 34 / REF 3V3: GPIO 35 |
| **Módulo SD** | Armazenamento dos dados da missão | SPI | SCK 18 / MISO 19 / MOSI 23 / CS 5 |

O barramento I2C principal opera a 100 kHz. O SGP30 usa um segundo barramento I2C (`Wire2`) separado.

## Comunicação

- **Tecnologia:** Wi-Fi (ESP32 como cliente), com envio por **HTTP POST** e conteúdo `application/json`.
- **Rede:** `obsat-test-server`
- **Endereço do servidor:** `http://192.168.4.1`
- **Intervalo de envio:** 30 s

### Formato do pacote

| Campo | Descrição |
|---|---|
| `equipe` | Nome da equipe (`FEGSAT`) |
| `tempo_ms` | Tempo desde a inicialização, em ms |
| `temp_bme` | Temperatura do BME680 (°C) |
| `pressao_bme` | Pressão do BME680 (hPa) |
| `umidade` | Umidade relativa do BME680 (%) |
| `uv_raw` | Leitura analógica do ML8511 (média de 8 amostras) |
| `uv_voltage` | Tensão de saída do ML8511 (V), corrigida pela referência de 3,3 V |
| `eco2` | eCO₂ do SGP30 (ppm) |
| `tvoc` | TVOC do SGP30 (ppb) |
| `giroscopio` | `[gx, gy, gz]` do MPU6050 (valores brutos) |
| `acelerometro` | `[ax, ay, az]` do MPU6050 (valores brutos) |
| `ms5611_raw` | `[pressão, temperatura]` brutos do MS5611 |

## Bibliotecas necessárias

Instale pelo Gerenciador de Bibliotecas da Arduino IDE:

| Biblioteca | Uso |
|---|---|
| `Wire`, `SPI`, `FS`, `SD` | Já incluídas no pacote do ESP32 (I2C, SPI e cartão SD) |
| `WiFi`, `HTTPClient` | Já incluídas no pacote do ESP32 (conexão Wi-Fi e requisições HTTP) |
| **Adafruit Unified Sensor** (`Adafruit_Sensor`) | Base das bibliotecas Adafruit |
| **Adafruit BME680 Library** | BME680 |
| **Adafruit AHTX0** | AHT10 |
| **Adafruit SGP30 Sensor** | SGP30 |
| **MPU6050** (i2cdevlib / Electronic Cats) | MPU6050 |
| **ArduinoJson** (versão 6.x) | Montagem do pacote JSON |
| **Adafruit GFX Library** | Declarada no código, ainda não usada |
| **Adafruit SSD1306** | Declarada no código, ainda não usada |
| **Adafruit MAX31865** | Declarada no código, ainda não usada |

> O código usa `StaticJsonDocument` e `createNestedArray`, que são da **ArduinoJson 6**. Na versão 7 esses recursos estão obsoletos.

Placa: **ESP32** (instale o pacote "esp32" da Espressif no Gerenciador de Placas).

## Como usar

1. Instale a [Arduino IDE](https://www.arduino.cc/en/software).
2. Instale o suporte à placa ESP32 e as bibliotecas listadas acima.
3. Abra o `.ino` desejado, selecione a placa e a porta corretas e faça o upload.
4. Abra o Monitor Serial a **115200 baud** para acompanhar as mensagens de inicialização (`SD OK`, `MS5611 OK`, `BME680 OK` etc.).

### Ordem sugerida para testar a comunicação

1. Grave `balao_zero_debris` no ESP32 do balão.
2. Grave `cod_zero_debris` no satélite para testar o envio de um pacote.
3. Confira no Monitor Serial se o balão recebeu o pacote.
4. Com a comunicação validada, grave `geral_esp32` no satélite para o funcionamento completo.

## Equipe

Projeto desenvolvido pela **FegSat**, equipe de nanossatélites da Unesp Guaratinguetá.


