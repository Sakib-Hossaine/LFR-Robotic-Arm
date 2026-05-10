// IR Sensors - 3 Sensor Array
#define IR_LEFT 34
#define IR_CENTER 32
#define IR_RIGHT 18

// Motor Driver L298N
#define IN1 12
#define IN2 14
#define IN3 27
#define IN4 26
#define ENA 13
#define ENB 25

/* ---------- rest of pins ---------- */
// Ultrasonic Sensor
#define TRIG_PIN 21
#define ECHO_PIN 19 

// Motor Speed
#define DRIVE_SPEED 60

// Sensor values - ADJUST THESE BASED ON YOUR TEST
bool IR_ON_BLACK = LOW;  // Change if needed
bool IR_ON_WHITE = HIGH; // Change if needed

int irLeft, irCenter, irRight;
bool isPaused = false;
unsigned long pauseStartTime = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(IR_LEFT, INPUT);
  pinMode(IR_CENTER, INPUT);
  pinMode(IR_RIGHT, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopMotors();

  Serial.println("=== 3-IR LINE FOLLOWER CAR ===");
}

void loop()
{
  // Read sensors
  irLeft = digitalRead(IR_LEFT);
  irCenter = digitalRead(IR_CENTER);
  irRight = digitalRead(IR_RIGHT);

  // Print values every 500ms
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500)
  {
    Serial.print("L:");
    Serial.print(irLeft);
    Serial.print(" C:");
    Serial.print(irCenter);
    Serial.print(" R:");
    Serial.print(irRight);

    // SIMPLIFIED LOGIC:
    // Move if center sensor is DIFFERENT from outer sensors
    if (irCenter != irLeft && irCenter != irRight)
    {
      Serial.println(" -> MOVING (Center different from sides)");
    }
    else if (irLeft == irCenter && irCenter == irRight)
    {
      Serial.println(" -> ALL SAME (All sensors see same surface)");
    }
    else
    {
      Serial.println(" -> STOPPED");
    }

    lastPrint = millis();
  }

  // Check if paused
  if (isPaused)
  {
    // Check if 3 seconds have passed
    if (millis() - pauseStartTime >= 3000)
    {
      Serial.println("RESUMING - 3 seconds complete");
      isPaused = false;
    }
    else
    {
      return;
    }
  }

  // SIMPLIFIED MOVEMENT LOGIC:
  // Move when center sensor is different from both outer sensors
  if (irCenter != irLeft && irCenter != irRight)
  {
    // Center is different - likely on the line
    driveForward();
  }
  // All three same (all black or all white) - pause
  else if (irLeft == irCenter && irCenter == irRight)
  {
    if (!isPaused)
    {
      Serial.println("ALL SAME - PAUSING 3 SECONDS");
      stopMotors();
      isPaused = true;
      pauseStartTime = millis();
    }
  }
  else
  {
    stopMotors();
  }

  delay(50);
}

void driveForward()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, DRIVE_SPEED);
  analogWrite(ENB, DRIVE_SPEED);
}

void stopMotors()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
