#include <Servo.h>
#include <math.h>

// Servo
Servo myservo;

// HC-SR04 ultrasonic sensor
const int trigPin = 9;
const int echoPin = 10;

// HW-493 laser module
const int laserPin = 12;

long duration;
float distance;
float height;
int servoSetting;
bool servoIncreasing = true;

const float MAX_DETECTION_RANGE = 200.0;
const float LASER_ACTIVATION_MIN_RANGE = 1.0;
const float LASER_ACTIVATION_MAX_RANGE = 50.0;


const float SENSOR_BEAM_ANGLE = 15.0; // Opening angle of ultrasonic wave cone
const float SENSOR_BEAM_ANGLE_RAD = SENSOR_BEAM_ANGLE * M_PI / 180.0; // Conversion to radians

void setup() {
  Serial.begin(115200);
  Serial.println("Radar System");

  myservo.attach(11);
  myservo.write(0);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, LOW);
}

bool laserActive = false;
unsigned long laserStartTime = 0;
bool autoMode = false;
bool servoStopped = false;

void loop() {
  getDistance();
  readSerialCommand();
  outputDistance();

  if (distance >= LASER_ACTIVATION_MIN_RANGE && distance <= LASER_ACTIVATION_MAX_RANGE && !laserActive) {
    activateLaser();
    servoStopped = true;
  } else if (laserActive && (millis() - laserStartTime >= 2000)) {
    deactivateLaser();
    servoStopped = false;
  }

  if (!laserActive && !servoStopped) {
    if (autoMode) {
      updateServoAuto();
    }
  }

  delay(50);
}

void activateLaser() {
  laserActive = true;
  laserStartTime = millis();
  digitalWrite(laserPin, HIGH);
  Serial.println("LASER_ACTIVATED");
}

void deactivateLaser() {
  laserActive = false;
  digitalWrite(laserPin, LOW);
  Serial.println("LASER_DEACTIVATED");
}

void getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  
  distance = (duration / 2.0) / 29.1;
  
  if (distance > MAX_DETECTION_RANGE) {
    distance = 0;
    height = 0; // If distance is out of range, set height to 0
  } else {
        height = distance * tan(SENSOR_BEAM_ANGLE_RAD); // Calculate the height based on the cone opening angle
  }
}

void readSerialCommand() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "AUTO") {
      autoMode = true;
      laserActive = false;
      digitalWrite(laserPin, LOW);
    } else if (command == "MANUAL") {
      autoMode = false;
      laserActive = false;
      digitalWrite(laserPin, LOW);
    } else {
      int angle = command.toInt();
      if (angle >= 0 && angle <= 180 && !autoMode) {
        myservo.write(angle);
        servoSetting = angle;
      }
    }
  }
}

void outputDistance() {
  if (distance <= 200) {
    Serial.print(servoSetting);
    Serial.print(",");
    Serial.print(distance);
    Serial.print(",");
    Serial.println(height);
  }
}

void updateServoAuto() {
  if (servoIncreasing) {
    servoSetting += 1;
    if (servoSetting >= 180) {
      servoSetting = 180;
      servoIncreasing = false;
    }
  } else {
    servoSetting -= 1;
    if (servoSetting <= 0) {
      servoSetting = 0;
      servoIncreasing = true;
    }
  }
  myservo.write(servoSetting);
}
