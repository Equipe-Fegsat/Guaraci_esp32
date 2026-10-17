//=====================================================================
// BIBLIOTECAS
//=====================================================================
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h> // Necessário para montar o JSON estruturado do balão

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
// BME280
//=====================================================================
Adafruit_BME280 bme; // Uso de I2C por padrão

//=====================================================================
// SETUP
//=====================================================================
void setup() {
  Serial.begin(115200);
  
  // Inicializa Wi-Fi
  initWifi();

  // Inicializa I2C (Pinos padrão do ESP32: SDA = 21, SCL = 22)
  Wire.begin(21, 22);
  Wire.setClock(100000);

  Serial.println("==== Inicializando Sensor BME280 ====");
  
  // Inicializa o BME280 no endereço I2C padrão (0x76 ou 0x77)
  if (!bme.begin(0x76)) { 
    Serial.println("Erro ao encontrar o BME280! Verifique as conexões.");
    while (1) { delay(10); }
  }
  
  Serial.println("BME280 OK!");
}

//=====================================================================
// LOOP
//=====================================================================
void loop() {
  unsigned long agora = millis();

  // Envia os dados no intervalo definido (30 segundos)
  if (agora - tempo >= auxiliar) {
    tempo = agora;

    // Leitura dos dados do BME280
    float tempBME = bme.readTemperature();
    float pressaoBME = bme.readPressure() / 100.0; // Convertendo para hPa
    float umidadeBME = bme.readHumidity(); // (Opcional, caso queira guardar na carga)

    Serial.printf("Temp: %.2f °C | Pressão: %.2f hPa | Umidade: %.2f %%\n", tempBME, pressaoBME, umidadeBME);

    // Envio via HTTP POST se estiver conectado ao Wi-Fi
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");

      // Montando o JSON exatamente como o código do balão exige na validação:
      // strs[] = {"equipe","bateria","temperatura","pressao","giroscopio","acelerometro","payload"}
      StaticJsonDocument<512> doc;
      
      doc["equipe"] = 3; // ID numérico da equipe (ex: 1)
      doc["bateria"] = 3.3; // Exemplo de valor pra bateria
      doc["temperatura"] = tempBME;
      doc["pressao"] = pressaoBME;

      // Arrays vazios ou com zeros para preencher as exigências do balão (já que não temos MPU6050)
      JsonArray giro = doc.createNestedArray("giroscopio");
      giro.add(0); giro.add(0); giro.add(0);

      JsonArray acel = doc.createNestedArray("acelerometro");
      acel.add(0); acel.add(0); acel.add(0);

      // Payload extra com dados adicionais (como umidade do BME280)
      JsonArray payload = doc.createNestedArray("payload");
      
      // Transformamos os números em textos bonitinhos antes de colocar no payload
      payload.add("Temperatura: " + String(tempBME, 2) + " C");
      payload.add("Pressao: " + String(pressaoBME, 2) + " hPa");
      payload.add("Umidade: " + String(umidadeBME, 2) + " %");

      String jsonString;
      serializeJson(doc, jsonString);

      int httpResponseCode = http.POST(jsonString);

      if (httpResponseCode > 0) {
        Serial.printf("Dados enviados com sucesso! Código HTTP: %d\n", httpResponseCode);
      } else {
        Serial.printf("Erro no envio. Código HTTP: %d\n", httpResponseCode);
      }

      http.end();
    } else {
      Serial.println("WiFi desconectado. Tentando reconectar...");
      WiFi.reconnect();
    }
  }
}