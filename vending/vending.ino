#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <AccelStepper.h>

// configurare motoare
const int STEPS_PER_REV = 6400; // DE CALIBRAT

class VendingSlot {
  private:
    AccelStepper motor;
    int enablePin;
  public:
    VendingSlot(int stepPin, int dirPin, int enPin) 
      : motor(AccelStepper::DRIVER, stepPin, dirPin) {
      enablePin = enPin;
    }

    void setup() {
      pinMode(enablePin, OUTPUT);
      digitalWrite(enablePin, HIGH); // initial dezactivat
      motor.setMaxSpeed(2000);
      motor.setAcceleration(800);
    }

    void dispense() {
      digitalWrite(enablePin, LOW); // activeaza driverul
      delay(50);
      long tinta = motor.currentPosition() + STEPS_PER_REV;
      motor.runToNewPosition(tinta); // roatie completa pentru eliberare produs
      digitalWrite(enablePin, HIGH); // dezactivam la finalul executiei
    }
};

// maparea pinii motoarelor
VendingSlot slots[] = {
  VendingSlot(54, 55, 38), // slot 1 pe x
  VendingSlot(60, 61, 56), // slot 2 pe y
  VendingSlot(46, 48, 62), // slot 3 pe z
  VendingSlot(36, 34, 30)  // slot 4 pe e1
};

// configurare interfata
LiquidCrystal_I2C lcd(0x27, 16, 2); 
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
}; 

byte rowPins[ROWS] = {47, 45, 43, 41}; 
byte colPins[COLS] = {39, 37, 35, 33}; [cite: 3]
Keypad tastatura = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

// cod vending
#define MAX_ITEMS 3
int stoc[4] = {MAX_ITEMS, MAX_ITEMS, MAX_ITEMS, MAX_ITEMS}; 
const int sensorPin = 2; 
unsigned long lastCoinTime = 0;
const int waitTime = 200;

int pretCurent = 0;
int monedeIntroduse = 0;
bool asteaptaMonede = false;
char produsSelectat = ' ';
int indexSelectat = -1;

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT); 
  
  lcd.init();
  lcd.backlight(); 

  for (int i = 0; i < 4; i++) {
    slots[i].setup();
  }

  meniuPrincipal(); 
}

void loop() {
  char tasta = tastatura.getKey(); 

  if (tasta) {
    procesareTasta(tasta); 
  }

  if (asteaptaMonede) {
    int sensorState = digitalRead(sensorPin); 
    if (sensorState == HIGH && (millis() - lastCoinTime > waitTime)) { 
      monedeIntroduse++;
      lastCoinTime = millis();
      informatiiProdus(); 
      
      if (monedeIntroduse >= pretCurent) { 
        eliberareProdus(); 
      }
    }
  }
}

void meniuPrincipal() {
  asteaptaMonede = false;
  pretCurent = 0;
  monedeIntroduse = 0;
  indexSelectat = -1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alege un produs"); 
}

void procesareTasta(char tasta) {
  if (tasta == '*') {
    meniuPrincipal();
    return; 
  }

  if (!asteaptaMonede) {
    int index = -1;
    switch (tasta) { 
      case '1': pretCurent = 1; index = 0; break; 
      case '2': pretCurent = 2; index = 1; break; 
      case '3': pretCurent = 3; index = 2; break; 
      case '4': pretCurent = 4; index = 3; break; 
      default: return;
    }
    
    if (stoc[index] > 0) { 
      produsSelectat = tasta;
      indexSelectat = index;
      asteaptaMonede = true;
      monedeIntroduse = 0;
      informatiiProdus(); 
    } else {
      indisponibil(); 
    }
  }
}

void informatiiProdus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Prod. ");
  lcd.print(produsSelectat);
  lcd.print(" Pret: ");
  lcd.print(pretCurent); 
  
  lcd.setCursor(0, 1);
  lcd.print("Credit: ");
  lcd.print(monedeIntroduse);
  lcd.print("/");
  lcd.print(pretCurent); 
}

void indisponibil() {
  lcd.clear();
  lcd.print("Stoc epuizat!");
  lcd.setCursor(0, 1);
  lcd.print("Alege altul"); 
  delay(2000);
  meniuPrincipal();
}

void eliberareProdus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Eliberare...");
  lcd.setCursor(0, 1);
  lcd.print("Va multumim!"); 
  
  // actionarea motorului corespunzator slotului
  if (indexSelectat != -1) {
    slots[indexSelectat].dispense(); 
    stoc[indexSelectat]--; 
  }

  delay(2000); 
  meniuPrincipal(); 
}