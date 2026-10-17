
//=====================================================================
// BIBLIOTECAS
//=====================================================================
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MAX31865.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP30.h>
#include <MPU6050.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include <SD.h>
#include <FS.h>
#include <WiFi.h>
//=====================================================================
// CONFIG WiFi
//=====================================================================
const char* ssid = "obsat-test-server";
const char* password = "obsatpass";
unsigned long tempo = 0;
unsigned long auxiliar = 30000; // envia a cada 30 segundos
String serverUrl = "http://192.168.4.1";
void initWifi() {
Serial.println("Conectando ao WiFi...");
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
Serial.println("\nWiFi conectado!");
Serial.print("IP: ");
Serial.println(WiFi.localIP());
}
//=====================================================================
// SEGUNDO I2C — SGP30
//=====================================================================
TwoWire Wire2 = TwoWire(1);
//=====================================================================
// MS5611
//=====================================================================
#define MS5611_ADDR 0x77
#define CMD_RESET 0x1E
#define CMD_ADC_READ 0x00
#define CMD_CONV_D1 0x40
#define CMD_CONV_D2 0x50
#define OSR_4096 0x08
#define CMD_PROM_READ 0xA0
uint16_t C[7];
uint32_t D1 = 0, D2 = 0;
void ms5611WriteCmd(uint8_t cmd) {
Wire.beginTransmission(MS5611_ADDR);
Wire.write(cmd);
Wire.endTransmission();
}
uint32_t ms5611ReadADC() {
Wire.beginTransmission(MS5611_ADDR);
Wire.write(CMD_ADC_READ);
Wire.endTransmission();
Wire.requestFrom(MS5611_ADDR, (uint8_t)3);
if (Wire.available() < 3) return 0;
return ((uint32_t)Wire.read() << 16) |
((uint32_t)Wire.read() << 8) |
Wire.read();
}
bool ms5611Reset() {
ms5611WriteCmd(CMD_RESET);
delay(3);
return true;
}
bool ms5611ReadPROM() {
for (uint8_t i = 0; i < 6; i++) {
uint8_t cmd = CMD_PROM_READ + (i + 1) * 2;
Wire.beginTransmission(MS5611_ADDR);
Wire.write(cmd);
if (Wire.endTransmission() != 0) return false;
Wire.requestFrom(MS5611_ADDR, (uint8_t)2);
if (Wire.available() < 2) return false;
C[i + 1] = ((uint16_t)Wire.read() << 8) | Wire.read();
}
return true;
}
void ms5611StartConv(uint8_t c) {
ms5611WriteCmd(c + OSR_4096);
}
void waitConvDelay() {
delay(10);
}
//=====================================================================
// BME680
//=====================================================================
Adafruit_BME680 bme;
//=====================================================================
// ML8511
//=====================================================================
int UVOUT = 34;
int REF_3V3 = 35;
//=====================================================================
// AHT10
//=====================================================================
Adafruit_AHTX0 aht;
//=====================================================================
// MPU6050
//=====================================================================
MPU6050 mpu;
//=====================================================================
// SGP30
//=====================================================================
Adafruit_SGP30 sgp;
float baseTVOC = 0;
float baseeCO2 = 0;
//=====================================================================
// FUNÇÕES AUXILIARES
//=====================================================================
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
return (x - in_min) * (out_max - out_min) /
(in_max - in_min) + out_min;
}
int mediaLeituraAnalogica(int pino) {
int soma = 0;
for (int i = 0; i < 8; i++) {
soma += analogRead(pino);
}
return soma / 8;
}
//=====================================================================
// ARMAZENAMENTO EM SD
//=====================================================================
#define SD_CS 5
File arquivoSD;
void salvarNoSD(const String &linha) {
arquivoSD = SD.open("/dados_missao.txt", FILE_APPEND);
if (arquivoSD) {
arquivoSD.println(linha);
arquivoSD.close();
} else {
Serial.println("Erro ao escrever no SD");
}
}
//=====================================================================
// SETUP
//=====================================================================
void setup() {
Serial.begin(115200);
SPI.begin(18, 19, 23, SD_CS);
initWifi();
Wire.begin(21, 22);
Wire.setClock(100000);
Serial.println("Escaneando barramento I2C...");
for (uint8_t addr = 1; addr < 127; addr++) {
Wire.beginTransmission(addr);
if (Wire.endTransmission() == 0) {
Serial.print("Dispositivo encontrado em 0x");
Serial.println(addr, HEX);
}
else Serial.println("Erro no I2C");
}
Serial.println("==== Inicializando Sensores ====");
if (!SD.begin(SD_CS)) Serial.println("Falha ao iniciar SD");
else Serial.println("SD OK");
if (!ms5611Reset() || !ms5611ReadPROM())
Serial.println("Erro MS5611");
else
Serial.println("MS5611 OK");
if (!bme.begin())
Serial.println("Erro BME680");
else
Serial.println("BME680 OK");
pinMode(UVOUT, INPUT);
pinMode(REF_3V3, INPUT);
if (!aht.begin()) Serial.println("Erro AHT10");
mpu.initialize();
mpu.setXGyroOffset(0);
mpu.setYGyroOffset(0);
mpu.setZGyroOffset(0);
mpu.setZAccelOffset(0);
if (mpu.testConnection())
Serial.println("MPU6050 OK");
else
Serial.println("Erro MPU6050");
Wire2.begin(26, 25);
if (sgp.begin(&Wire2))
Serial.println("SGP30 OK");
else
Serial.println("Erro SGP30");
}
//=====================================================================
// LOOP
//=====================================================================
void loop() {
unsigned long agora = millis();
if (agora - tempo >= auxiliar) {
tempo = agora;
bme.performReading();
//BME680
float tempBME = bme.temperature;
float pressaoBME = bme.pressure / 100.0;
float umidadeBME = bme.humidity;
//MS5611
uint32_t pressaoRaw = D1;
uint32_t tempRaw = D2;
//ML5611
int uvLevel = mediaLeituraAnalogica(UVOUT);
int refLevel = mediaLeituraAnalogica(REF_3V3);
float outputVoltage = 3.3 * uvLevel / refLevel;
//MPU6050
int16_t ax, ay, az, gx, gy, gz;
mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
//SGP30
if (sgp.IAQmeasure()) {
baseeCO2 = sgp.eCO2;
baseTVOC = sgp.TVOC;
}
if (WiFi.status() == WL_CONNECTED) {
HTTPClient http;
http.begin(serverUrl);
http.addHeader("Content-Type", "application/json");
StaticJsonDocument<256> doc;
doc["equipe"] = "FEGSAT";
doc["tempo_ms"] = millis();
doc["temp_bme"] = tempBME;
doc["pressao_bme"] = pressaoBME;
doc["umidade"] = umidadeBME;
doc["uv_raw"] = uvLevel;
doc["uv_voltage"] = outputVoltage;
doc["eco2"] = baseeCO2;
doc["tvoc"] = baseTVOC;
JsonArray g = doc.createNestedArray("giroscopio");
g.add(gx);
g.add(gy);
g.add(gz);
JsonArray a = doc.createNestedArray("acelerometro");
a.add(ax);
a.add(ay);
a.add(az);
JsonArray ms = doc.createNestedArray("ms5611_raw");
ms.add(pressaoRaw);
ms.add(tempRaw);
String jsonString;
serializeJson(doc, jsonString);
http.POST(jsonString);
http.end();
salvarNoSD(jsonString);
}
}
ms5611StartConv(CMD_CONV_D1);
waitConvDelay();
D1 = ms5611ReadADC();
ms5611StartConv(CMD_CONV_D2);
waitConvDelay();
D2 = ms5611ReadADC();
}