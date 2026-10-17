# FegSat – CubeSat e Balão Simulado

Repositório com os códigos de firmware usados na simulação da comunicação entre o **satélite principal (CubeSat)** e um **balão simulado**, desenvolvidos pela equipe FegSat (Unesp Guaratinguetá).

## Visão geral

O projeto simula um cenário em que um balão se comunica com o satélite. Como não temos um balão real em todos os testes, o balão é representado por um **ESP32** rodando um código próprio. O satélite principal recebe e processa os dados enviados por ele.

```
 ┌─────────────────┐    pacotes    ┌─────────────────┐
 │  Balão simulado │ ◄───────────► │    CubeSat      │
 │     (ESP32)     │ [comunicação] │  (geral_esp32)  │
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

> A IDE do Arduino exige que cada `.ino` fique dentro de uma pasta com o mesmo nome. Por isso, mantenha a estrutura acima ao baixar ou mover os arquivos.

## Descrição dos códigos

### `balao_zero_debris/`
Código do **balão simulado**, executado em um ESP32. Ele representa o balão na comunicação com o satélite: recebe os pacotes enviados pelo CubeSat e [descreva aqui o que o balão faz: responde, registra dados, envia telemetria etc.].

### `geral_esp32/`
Código principal do **satélite**. Responsável por [descreva as funções: leitura de sensores, gerenciamento da comunicação, armazenamento de dados etc.].

### `cod_zero_debris/`
**Código de teste** para ser gravado no satélite. Sua função é enviar um pacote ao balão, permitindo validar a comunicação entre os dois lados sem precisar rodar todo o firmware principal.

## Hardware

| Componente | Função |
|---|---|
| ESP32 (balão) | Simula o balão |
| [Placa do satélite] | Executa o firmware do CubeSat |
| [Módulo de comunicação] | Link entre satélite e balão |
| [Sensores, se houver] | [Função] |

## Comunicação

- **Tecnologia:** [LoRa / UART / Wi-Fi / ESP-NOW / outra]
- **Formato do pacote:** [descreva os campos, ex.: ID, timestamp, dados, checksum]
- **Frequência / baud rate:** [valor]

## Como usar

1. Instale a [Arduino IDE](https://www.arduino.cc/en/software) (ou PlatformIO).
2. Instale o suporte às placas necessárias (ex.: ESP32 pelo Gerenciador de Placas).
3. Instale as bibliotecas listadas abaixo.
4. Abra o `.ino` desejado, selecione a placa e a porta corretas e faça o upload.

### Ordem sugerida para testar a comunicação

1. Grave `balao_zero_debris` no ESP32 do balão.
2. Grave `cod_zero_debris` no satélite para testar o envio de um pacote.
3. Confira no Monitor Serial se o balão recebeu o pacote.
4. Com a comunicação validada, grave `geral_esp32` no satélite para o funcionamento completo.


## Equipe

Projeto desenvolvido pela **FegSat**, equipe de nanossatélites da Unesp Guaratinguetá.

## Licença

[Defina a licença, ex.: MIT, ou remova esta seção.]
