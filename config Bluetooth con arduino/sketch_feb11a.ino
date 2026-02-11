#include <SoftwareSerial.h>

// RX, TX (Arduino)
SoftwareSerial bt(10, 11);

// Envia comando AT "pelado" y espera respuesta
void sendAT(const char *cmd, unsigned long wait_ms = 800) {
  Serial.print(">> ");
  Serial.println(cmd);

  // Enviar sin CR ni LF
  for (const char *p = cmd; *p; p++) {
    bt.write(*p);
  }

  delay(wait_ms);

  Serial.print("<< ");
  while (bt.available()) {
    Serial.write(bt.read());
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // Baud ACTUAL del HC-06 (config histórica)
  bt.begin(57600);

  Serial.println("=== Configuracion HC-06 ===");
  Serial.println("IMPORTANTE: el modulo NO debe estar conectado al celular");
  delay(500);

  // Test AT
  sendAT("AT");

  // Configuracion
  sendAT("AT+NAMEDimmer_BL");  // Nombre del dispositivo
  sendAT("AT+PIN1111");        // PIN

  // Si quisieras cambiar baud, por ejemplo a 115200:
  // sendAT("AT+BAUD8");
  // delay(1000);
  // bt.end();
  // bt.begin(115200);

  Serial.println("=== Configuracion finalizada ===");
  Serial.println("Apagar y prender el modulo para usarlo en modo DATA");
}

void loop() {
  // Puente libre para debug si queres
  if (Serial.available())
    bt.write(Serial.read());

  if (bt.available())
    Serial.write(bt.read());
}
