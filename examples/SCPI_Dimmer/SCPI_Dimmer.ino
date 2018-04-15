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
  my_instrument.ProcessInput(Serial, "\n");
}

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println("Vrekrer,Arduino SCPI Dimmer,#00,v0.3");
}

void SetBrightness(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    brightness = constrain(String(parameters[0]).toInt(), 0, 10);
    analogWrite(ledPin, intensity[brightness]);
  }
}

void GetBrightness(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(String(brightness, DEC));
}

void IncDecBrightness(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  String last_header = String(commands.Last());
  last_header.toUpperCase();
  if (last_header.startsWith("INC")) {
    brightness = constrain(brightness + 1, 0, 10);
  } else { // "DEC"
    brightness = constrain(brightness - 1, 0, 10);
  }
  analogWrite(ledPin, intensity[brightness]);
}
