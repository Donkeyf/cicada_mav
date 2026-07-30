void setup() {
  // USB serial to PC
  Serial.begin(115200);

  // Hardware UART on pins 0/1
  Serial1.begin(115200);

  while (!Serial) {
    ; // wait for USB serial connection
  }

  Serial.println("UART bridge started");
}

void loop() {
  // Read UART and print to PC
  while (Serial1.available()) {
    char c = Serial1.read();
    Serial.write((int)c);
  }
}
