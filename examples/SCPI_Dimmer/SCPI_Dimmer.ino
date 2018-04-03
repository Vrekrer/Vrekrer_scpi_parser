#include "Arduino.h";
#include "Vrekrer_scpi_parser.h";

SCPI_Parser my_instrument;
int brightness = 0;
const int ledPin = 9;
const int intensity[11] = {0, 3, 5, 9, 15, 24, 38, 62, 99, 159, 255};

void setup()
{
  my_instrument.RegisterCommand("*IDN?", &Identify);
  my_instrument.SetCommandTreeBase("SYSTem:LED");
    my_instrument.RegisterCommand(":BRIGhtness", &SetBrightness);
    my_instrument.RegisterCommand(":BRIGhtness?", &GetBrightness);
    my_instrument.RegisterCommand(":BRIGhtness:INCrease", &IncDecBrightness);
    my_instrument.RegisterCommand(":BRIGhtness:DECrease", &IncDecBrightness);
  
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(LED_BUILTIN, INPUT);
  analogWrite(ledPin, 0);
}

void loop()
{
  char line_buffer[256];
  unsigned char read_length;

  // Read in a line and execute it
  read_length = Serial.readBytesUntil('\n', line_buffer, 256);
  if(read_length > 0) {
    line_buffer[read_length] = '\0'; //Strip the terminator char
    my_instrument.Execute(String(line_buffer));
  }
}

void Identify(SCPI_Commands commands, SCPI_Parameters parameters){
  Serial.println("Vrekrer,Arduino SCPI Dimmer,#00,v0.2");
}

void SetBrightness(SCPI_Commands commands, SCPI_Parameters parameters){
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    brightness = constrain(parameters[0].toInt(), 0, 10);
    analogWrite(ledPin, intensity[brightness]);
  }
}

void GetBrightness(SCPI_Commands commands, SCPI_Parameters parameters) {
  Serial.println(String(brightness, DEC));
}

void IncDecBrightness(SCPI_Commands commands, SCPI_Parameters parameters){
  String last_header = commands[commands.Size() - 1];
  last_header.toUpperCase();
  if (last_header.startsWith("INC")) {
    brightness = constrain(brightness + 1, 0, 10);
  } else { // "DEC"
    brightness = constrain(brightness - 1, 0, 10);
  }
  analogWrite(ledPin, intensity[brightness]);
}
