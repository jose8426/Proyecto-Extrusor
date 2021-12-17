#include <math.h>
#include <AccelStepper.h>
#include <LiquidCrystal.h>

nual = 0

// Digitar estado de pantalla
int state = 79;
int directory = 31;

// Lectura de los diémetros del filamento
double oldDiameterReading;
unsigned long oldDiameterReadingTime = millis();

int pullerReadyVariable = 0; // Encendido y apagado del extrusor
int startSequence = 1;  // La corriente regresa de nuevo, cambia a 0 después de presionar el interruptor

// Se reanudan todas las variables de configuración actual en las pantallas, siempre y cuando si así lo desea
int execution = 1;
int initialSetting = 0;
int variable = 0;
int variable1 = 0;
int variable2 = 0;
int variable3 = 0;
int guideVariable = 0;

double upperFilamentBound;
double lowerFilamentBound;


unsigned int diameterDelay = 25000;
unsigned long lastAdjustmentTime = millis();

unsigned long lastInputTime = 0;
int Y = 0; // Para propósitos de aceleración

// Controles de velocidad del motor
int desiredAugerRPM = 35;
int currentAugerRPM = 0;

int pullerRPM = 50;
double pullerPulses;
double pullerPulsesAuto;

int winderRPM = 2;
int winderPulses;

// Diámetro del filamento
double desiredFilamentDiameter;
double filamentDiameter;
int filamentDiameterPin = A3;

// Pines del motor al barreno de la tolva principal 
int augerEnable = 38;
int augerpulses;
int augerStep = A0;  
int augerDIR = A1;

// Control del extractor y pines de salida del motor
int pullerEnable = A2;

int pullerStep = A6;
int pullerDIR = A7;

// Pines del control de temperatura (ventiladores)
int winderEnable = 24;

int winderStep = 26;
int winderDIR = 28;

// Pines Z
int guideEnable = A8;

int guideStep = 46;
int guideDIR = 48;

int guideStart;
int guideStop;
int guidePos;

int filamentDirection = 1;

// Control de temperatura (salida del relay)
int heaterRelayPin = 1;

// Enfriador de temperatura
const int numTempReadings = 20;

double tempReadings[numTempReadings];
int tempIndex = 0;
double tempTotal = 0;
double tempAverage = 0;

// Suavisador de diémetro 
const int numdiameter1Readings = 20;
const int numdiameter2Readings = 20;

double diameter1Readings[numTempReadings];
double diameter2Readings[numTempReadings];

int diameterIndex1 = 0;
int diameterIndex2 = 0;

double diameter1Total = 0;
double diameter2Total = 0;

double diameterAverage1 = 0;
double diameterAverage2 = 0;

// Funciones del motor de "paso a paso"
AccelStepper auger(1, augerStep, augerDIR);
AccelStepper puller(1, pullerStep, pullerDIR);
AccelStepper winder(1, winderStep, winderDIR);
AccelStepper guide(1, guideStep, guideDIR);

// Configuración del codificador
static int pinA = 2;
static int pinB = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;

volatile byte encoderPos = 0;
volatile byte oldEncoderPos = 0;
volatile byte reading = 0;

const byte buttonPin = 14;
byte oldButtonState = HIGH;
const unsigned long debounceTime = 100; 
unsigned long buttonPressTime;
boolean buttonPressed = 0;

// Configuración del interruptor de límite
const byte buttonPin1 = 11;
byte oldButtonState1 = LOW;
unsigned long buttonPressTime1;
boolean buttonPressed1 = 0;

 const byte buttonPin2 = 6;
byte oldButtonState2 = LOW;
unsigned long buttonPressTime2;
boolean buttonPressed2 = 0;

// Soportes de ventilador
bool fanState = true;
bool oldFanState = true;
double fanSpeed = 0;//0-1023 analog output

int fanPin = 10;

// Configuración del LCD
LiquidCrystal lcd(32,47,45,43,41,39,37);

void setup() {
  
  //Serial.begin(9600); //troubleshooting
  //pin declorations
  pinMode(fanPin, OUTPUT);
  pinMode(heaterRelayPin, OUTPUT);
  pinMode(filamentDiameterPin, INPUT);
  
  filamentDiameter = analogRead(filamentDiameterPin);
  filamentDiameter = (filamentDiameter*5)/1023;
  
  auger.setMaxSpeed(50000);
  auger.setAcceleration(500);

  puller.setMaxSpeed(50000);
  puller.setAcceleration(10000);

  winder.setMaxSpeed(500000);
  winder.setAcceleration(100000);
  
  guide.setMaxSpeed(500000);
  guide.setAcceleration(200000);

// Configuración del barreno
  pinMode(augerEnable, OUTPUT);
  pinMode(augerStep, OUTPUT);
  pinMode(augerDIR, OUTPUT);

  digitalWrite(augerEnable, HIGH);
  digitalWrite(augerStep, LOW);
  digitalWrite(augerDIR, HIGH);

// Configuración del arrancador
  pinMode(pullerEnable, OUTPUT);
  pinMode(pullerStep, OUTPUT);
  pinMode(pullerDIR, OUTPUT);

  digitalWrite(pullerEnable, HIGH);
  digitalWrite(pullerStep, LOW);
  digitalWrite(pullerDIR, HIGH);

// Configuración de la devanadora
  pinMode(winderEnable, OUTPUT);
  pinMode(winderStep, OUTPUT);
  pinMode(winderDIR, OUTPUT);

  digitalWrite(winderEnable, HIGH);
  digitalWrite(winderStep, LOW);
  digitalWrite(winderDIR, LOW);

// Guía
  pinMode(guideEnable, OUTPUT);
  pinMode(guideStep, OUTPUT);
  pinMode(guideDIR, OUTPUT);

  digitalWrite(guideEnable, LOW);
  digitalWrite(guideStep, LOW);
  digitalWrite(guideDIR, HIGH);
  
// Establecer valores iniciales
  augerpulses = 200*desiredAugerRPM;
  auger.setSpeed(augerpulses);
  currentAugerRPM = desiredAugerRPM;
  
  pullerPulses = pullerRPM*100;
  puller.setSpeed(pullerPulses);
  winderPulses = winderRPM*20;
  winder.setSpeed(winderPulses);
  
  guide.setSpeed(-2500);

// Inicio de la interfaz del usuario
  lcd.begin(20,4);

  screenUpdate(state,currentTemp);

  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  attachInterrupt(0, PinA, RISING);
  attachInterrupt(1, PinB, RISING);
  
//  Fin de la interfaz dek usuario

// Fin de la configuración
