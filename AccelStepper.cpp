#include "AccelStepper.h"

void AccelStepper::moveTo(long absolute)
{
    _targetPos = absolute;
    computeNewSpeed();
}

void AccelStepper::move(long relative)
{
    moveTo(_currentPos + relative);
}

// Implementa pasos de acuerdo a la velocidad actual
// Debes llamar a esto al menos una vez por paso
// devuelve verdadero si ocurrió un paso
boolean AccelStepper::runSpeed()
{
    unsigned long time = millis();
  
    if (time > _lastStepTime + _stepInterval)
    {
	if (_speed > 0)
	{
	    // sentido horario
	    _currentPos += 1;
	}
	else if (_speed < 0)
	{
	    // anti horario  
	    _currentPos -= 1;
	}
	step(_currentPos & 0x3); Bottom 2 bits (igual que el mod 4, pero funciona con números + y -) 

	_lastStepTime = time;
	return true;
    }
    else
	return false;
}

long AccelStepper::distanceToGo()
{
    return _targetPos - _currentPos;
}

long AccelStepper::targetPosition()
{
    return _targetPos;
}

long AccelStepper::currentPosition()
{
    return _currentPos;
}

// Útil durante las inicializaciones o después del posicionamiento inicial
void AccelStepper::setCurrentPosition(long position)
{
    _currentPos = position;
}

void AccelStepper::computeNewSpeed()
{
    setSpeed(desiredSpeed());
}

// Entrena y devuelve una nueva velocidad.
// Las subclases pueden anular si quieren
// Implementar aceleración, desaceleración y velocidad máxima
// La velocidad negativa es en sentido antihorario
// Se llama:
// después de cada paso
// después de que el usuario cambie:
//   máxima velocidad
// aceleración
// posición de destino (relativa o absoluta)
float AccelStepper::desiredSpeed()
{
    long distanceTo = distanceToGo();

    // Velocidad máxima posible que aún puede desacelerar en la distancia disponible
    float requiredSpeed;
    if (distanceTo == 0)
	return 0.0; // Estamos aqui
    else if (distanceTo > 0) // sentido horario
	requiredSpeed = sqrt(2.0 * distanceTo * _acceleration);
    else  // anti horario
	requiredSpeed = -sqrt(2.0 * -distanceTo * _acceleration);

    if (requiredSpeed > _speed)
    {
	//Se necesita acelerar en el sentido delas agujas del reloj
	if (_speed == 0)
	    requiredSpeed = sqrt(2.0 * _acceleration);
	else
	    requiredSpeed = _speed + abs(_acceleration / _speed);
	if (requiredSpeed > _maxSpeed)
	    requiredSpeed = _maxSpeed;
    }
    else if (requiredSpeed < _speed)
    {
	// Se necesita acelerar en el sentido  anti horario delas agujas del reloj
	if (_speed == 0)
	    requiredSpeed = -sqrt(2.0 * _acceleration);
	else
	    requiredSpeed = _speed - abs(_acceleration / _speed);
	if (requiredSpeed < -_maxSpeed)
	    requiredSpeed = -_maxSpeed;
    }
//  Serial.println(requiredSpeed);
    return requiredSpeed;
}

// Hacer funcionar el motor para implementar la velocidad y la aceleración con el fin de proceder a la posición de destino
// Debe llamar a esto al menos una vez por paso, preferiblemente en su bucle principal
// Si el motor está en la posición deseada, el costo es muy pequeño
// devuelve verdadero si todavía estamos corriendo a la posición
boolean AccelStepper::run()
{
    if (_targetPos == _currentPos)
	return false;
    
    if (runSpeed())
	computeNewSpeed();
    return true;
}

AccelStepper::AccelStepper(uint8_t pins, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4)
{
    _pins = pins;
    _currentPos = 0;
    _targetPos = 0;
    _speed = 0.0;
    _maxSpeed = 1.0;
    _acceleration = 1.0;
    _stepInterval = 0;
    _lastStepTime = 0;
    _pin1 = pin1;
    _pin2 = pin2;
    _pin3 = pin3;
    _pin4 = pin4;
    enableOutputs();
}

AccelStepper::AccelStepper(void (*forward)(), void (*backward)())
{
    _pins = 0;
    _currentPos = 0;
    _targetPos = 0;
    _speed = 0.0;
    _maxSpeed = 1.0;
    _acceleration = 1.0;
    _stepInterval = 0;
    _lastStepTime = 0;
    _pin1 = 0;
    _pin2 = 0;
    _pin3 = 0;
    _pin4 = 0;
    _forward = forward;
    _backward = backward;
}

void AccelStepper::setMaxSpeed(float speed)
{
    _maxSpeed = speed;
    computeNewSpeed();
}

void AccelStepper::setAcceleration(float acceleration)
{
    _acceleration = acceleration;
    computeNewSpeed();
}

void AccelStepper::setSpeed(float speed)
{
    _speed = speed;
    _stepInterval = abs(1000.0 / _speed);
}

float AccelStepper::speed()
{
    return _speed;
}

// Las subclases pueden anular
void AccelStepper::step(uint8_t step)
{
    switch (_pins)
    {
        case 0:
            step0();
            break;
	case 1:
	    step1(step);
	    break;
    
	case 2:
	    step2(step);
	    break;
    
	case 4:
	    step4(step);
	    break;  
    }
}

// Función de paso de 0 pines (es decir, para uso funcional)
void AccelStepper::step0()
{
  if (_speed > 0) {
    _forward();
  } else {
    _backward();
  }
}

// Función de paso de 1 pin (es decir, para controladores paso a paso)
// Se pasa el número de paso actual (0 a 3)
// Las subclases pueden anular
void AccelStepper::step1(uint8_t step)
{
    digitalWrite(_pin2, _speed > 0); // Direccion
    // Precaucion 200ns setup time 
    digitalWrite(_pin1, HIGH);
    // Precaucion, El ancho de pulso de paso mínimo para 3967 es 1microsec
    // Retraso 1 microsec
    delayMicroseconds(1);
    digitalWrite(_pin1, LOW);
}

// Función de paso de 2 pines
// Se pasa el número de paso actual (0 a 3)
// Las subclases pueden anular
void AccelStepper::step2(uint8_t step)
{
    switch (step)
    {
	case 0: /* 01 */
	    digitalWrite(_pin1, LOW);
	    digitalWrite(_pin2, HIGH);
	    break;

	case 1: /* 11 */
	    digitalWrite(_pin1, HIGH);
	    digitalWrite(_pin2, HIGH);
	    break;

	case 2: /* 10 */
	    digitalWrite(_pin1, HIGH);
	    digitalWrite(_pin2, LOW);
	    break;

	case 3: /* 00 */
	    digitalWrite(_pin1, LOW);
	    digitalWrite(_pin2, LOW);
	    break;
    }
}

// Función de paso de 4 pines
// Se pasa el número de paso actual (0 a 3)
// Las subclases pueden anular
void AccelStepper::step4(uint8_t step)
{
    switch (step)
    {
	case 0:    // 1010
	    digitalWrite(_pin1, HIGH);
	    digitalWrite(_pin2, LOW);
	    digitalWrite(_pin3, HIGH);
	    digitalWrite(_pin4, LOW);
	    break;

	case 1:    // 0110
	    digitalWrite(_pin1, LOW);
	    digitalWrite(_pin2, HIGH);
	    digitalWrite(_pin3, HIGH);
	    digitalWrite(_pin4, LOW);
	    break;

	case 2:    //0101
	    digitalWrite(_pin1, LOW);
	    digitalWrite(_pin2, HIGH);
	    digitalWrite(_pin3, LOW);
	    digitalWrite(_pin4, HIGH);
	    break;

	case 3:    //1001
	    digitalWrite(_pin1, HIGH);
	    digitalWrite(_pin2, LOW);
	    digitalWrite(_pin3, LOW);
	    digitalWrite(_pin4, HIGH);
	    break;
    }
}


// Evita el consumo de energía en las salidas
void    AccelStepper::disableOutputs()
{   
  if (! _pins) return;

    digitalWrite(_pin1, LOW);
    digitalWrite(_pin2, LOW);
    if (_pins == 4)
    {
	digitalWrite(_pin3, LOW);
	digitalWrite(_pin4, LOW);
    }
}

void    AccelStepper::enableOutputs()
{
    if (! _pins) return;

    pinMode(_pin1, OUTPUT);
    pinMode(_pin2, OUTPUT);
    if (_pins == 4)
    {
	pinMode(_pin3, OUTPUT);
	pinMode(_pin4, OUTPUT);
    }
}

// Bloquea hasta alcanzar la posición de destino
void AccelStepper::runToPosition()
{
    while (run())
	;
}

boolean AccelStepper::runSpeedToPosition()
{
    return _targetPos!=_currentPos ? AccelStepper::runSpeed() : false;
}

// Bloquea hasta que se alcanza la nueva posición de destino
void AccelStepper::runToNewPosition(long position)
{
    moveTo(position);
    runToPosition();
}

