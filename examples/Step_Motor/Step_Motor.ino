/////////////////////////////////////////////////////////////////////////////////////////////
// Simple 3 stepper motor contoller based on the Vrekrer_scpi_parser. ///////////////////////
// This program based on the Arduino UNO R3 /////////////////////////////////////////////////
// This program moves X, Y, Z seperately. Applications maybe limited by its nature. /////////
// Sorry for the crude code, but it's working anyway. Enjoy!  ///////////////////////////////
// - Wooseung, codenamezoo@gmail.com ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include<Stepper.h>
#include "Arduino.h"
#include "Vrekrer_scpi_parser.h"
#include "Vrekrer_scpi_parser.cpp"

SCPI_Parser my_instrument;

// Stepper motor setting for X-axis control, axis 0.
int stepIN1PinX = 2;
int stepIN2PinX = 3;
int stepIN3PinX = 4;
int stepIN4PinX = 5;

int stepsPerRevolutionX = 2048; // steps for 1 revolution in X-axis motor. 
// The order of pin configuration may vary upon your motor driver or driving method. 
//Stepper stepperX(stepsPerRevolutionX, stepIN1PinX, stepIN2PinX, stepIN3PinX, stepIN4PinX);
Stepper stepperX(stepsPerRevolutionX, stepIN1PinX, stepIN3PinX, stepIN2PinX, stepIN4PinX);

// Stepper motor setting for Y-axis control, axis 1.
int stepIN1PinY = 6;
int stepIN2PinY = 7;
int stepIN3PinY = 8;
int stepIN4PinY = 9;

int stepsPerRevolutionY = 2048; // steps for 1 revolution in Y-axis motor. 
// The order of pin configuration may vary upon your motor driver or driving method. 
//Stepper stepperY(stepsPerRevolutionY, stepIN1PinY, stepIN2PinY, stepIN3PinY, stepIN4PinY);
Stepper stepperY(stepsPerRevolutionY, stepIN1PinY, stepIN3PinY, stepIN2PinY, stepIN4PinY);

// Stepper motor setting for Z-axis control, axis 2.
int stepIN1PinZ = 10;
int stepIN2PinZ = 11;
int stepIN3PinZ = 12;
int stepIN4PinZ = 13;

int stepsPerRevolutionZ = 2048; // steps for 1 revolution in Z-axis motor. 
// The order of pin configuration may vary upon your motor driver or driving method. 
//Stepper stepperZ(stepsPerRevolutionZ, stepIN1PinZ, stepIN2PinZ, stepIN3PinZ, stepIN4PinZ);
Stepper stepperZ(stepsPerRevolutionZ, stepIN1PinZ, stepIN3PinZ, stepIN2PinZ, stepIN4PinZ);

int currentStep[3] = {0, 0, 0}; // identified last location of motor control. 
int currentSpeed[3] = {10, 10, 10}; // motor speed in revolution per minute (rpm)


void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println("Arduino SCPI Motor Contoller,#00,v1.0");
}

/////////////////////////////////////////////////////////////////////////////////////////////
// X-axis control sequences /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void MoveXAxis(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int moveStep = String(parameters[0]).toInt();
    stepperX.step(moveStep);
    currentStep[0] += moveStep;
  }
}

void SetXLocation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int setCurrentStep = String(parameters[0]).toInt();
    currentStep[0] = setCurrentStep;
  }
}

void GetXLocation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(String(currentStep[0], DEC));
}

void SetXSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int setCurrentSpeed = String(parameters[0]).toInt();
    stepperX.setSpeed(setCurrentSpeed);
    currentSpeed[0] = setCurrentSpeed;
  }
}

void GetXSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(String(currentSpeed[0], DEC));
}
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Y-axis control sequences /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void MoveYAxis(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int moveStep = String(parameters[0]).toInt();
    stepperY.step(moveStep);
    currentStep[1] += moveStep;
  }
}

void SetYLocation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int setCurrentStep = String(parameters[0]).toInt();
    currentStep[1] = setCurrentStep;
  }
}

void GetYLocation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(String(currentStep[1], DEC));
}

void SetYSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int setCurrentSpeed = String(parameters[0]).toInt();
    stepperY.setSpeed(setCurrentSpeed);
    currentSpeed[1] = setCurrentSpeed;
  }
}

void GetYSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(String(currentSpeed[0], DEC));
}
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
// Z-axis control sequences /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void MoveZAxis(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int moveStep = String(parameters[0]).toInt();
    stepperZ.step(moveStep);
    currentStep[2] += moveStep;
  }
}

void SetZLocation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int setCurrentStep = String(parameters[0]).toInt();
    currentStep[2] = setCurrentStep;
  }
}

void GetZLocation(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(String(currentStep[2], DEC));
}

void SetZSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // For simplicity no bad parameter check is done.
  if (parameters.Size() > 0) {
    int setCurrentSpeed = String(parameters[0]).toInt();
    stepperZ.setSpeed(setCurrentSpeed);
    currentSpeed[2] = setCurrentSpeed;
  }
}

void GetZSpeed(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.println(String(currentSpeed[2], DEC));
}
///////////////////////////////////////////////////////////////////////////////

void setup()
{
  stepperX.setSpeed(currentSpeed[0]);
  stepperY.setSpeed(currentSpeed[1]);
  stepperZ.setSpeed(currentSpeed[2]);  
  my_instrument.RegisterCommand("*IDN?", &Identify);
  my_instrument.SetCommandTreeBase("MOTOr"); // motor control tree
    my_instrument.RegisterCommand(":XMOVe", &MoveXAxis);
    my_instrument.RegisterCommand(":XLOCation", &SetXLocation);
    my_instrument.RegisterCommand(":XLOCation?", &GetXLocation);
    my_instrument.RegisterCommand(":XSPEed", &SetXSpeed);
    my_instrument.RegisterCommand(":XSPEed?", &GetXSpeed);
    my_instrument.RegisterCommand(":YMOVe", &MoveYAxis);
    my_instrument.RegisterCommand(":YLOCation", &SetYLocation);
    my_instrument.RegisterCommand(":YLOCation?", &GetYLocation);
    my_instrument.RegisterCommand(":YSPEed", &SetYSpeed);
    my_instrument.RegisterCommand(":YSPEed?", &GetYSpeed);
    my_instrument.RegisterCommand(":ZMOVe", &MoveZAxis);
    my_instrument.RegisterCommand(":ZLOCation", &SetZLocation);
    my_instrument.RegisterCommand(":ZLOCation?", &GetZLocation);
    my_instrument.RegisterCommand(":ZSPEed", &SetZSpeed);
    my_instrument.RegisterCommand(":ZSPEed?", &GetZSpeed);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


}

void loop()
{
  my_instrument.ProcessInput(Serial, "\n");
}
