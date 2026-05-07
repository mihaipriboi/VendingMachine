#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define MAX_ITEMS 3
LiquidCrystal_I2C lcd(0x27, 16, 2); 

const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

int stoc[4]={MAX_ITEMS, MAX_ITEMS, MAX_ITEMS, 0};

// pinii tastaturii
byte rowPins[ROWS] = {47, 45, 43, 41}; 
byte colPins[COLS] = {39, 37, 35, 33};
Keypad tastatura = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// senzor monede
const int sensorPin = 2;
unsigned long lastCoinTime = 0;
const int waitTime = 200; 

int pretCurent = 0;
int monedeIntroduse = 0;
bool asteaptaMonede = false;
char produsSelectat = ' ';

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);

  lcd.init();
  lcd.backlight();
  
  meniuPrincipal();
}

void loop() {
  char tasta = tastatura.getKey();

  if (tasta) {
    procesareTasta(tasta);
  }

  if (asteaptaMonede) {
    int sensorState = digitalRead(sensorPin);
    
    if (sensorState == HIGH&&(millis() - lastCoinTime > waitTime)) {
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
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alege un produs");
}

void procesareTasta(char tasta) {
  //* anuleaza selectia
  if (tasta == '*') {
    meniuPrincipal();
    return;
  }

  if (!asteaptaMonede) {
    int index=-1;
    switch (tasta) {
      case '1': pretCurent = 1; index=0; break; 
      case '2': pretCurent = 2; index=1; break; 
      case '3': pretCurent = 3; index=2; break; 
      case '4': pretCurent = 4; index=3; break; 
      default: return; 
    }
    
    if(stoc[index]>0){
      produsSelectat=tasta;
      asteaptaMonede=true;
      monedeIntroduse=0;
      informatiiProdus();
    } else{
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
  lcd.print(monedeIntroduse);
  lcd.print("/");
  lcd.print(pretCurent);
}

void indisponibil(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Stoc epuizat!");
  lcd.setCursor(0,1);
  lcd.print("Alege alt produs");
  delay(3000);
  meniuPrincipal();
}

void eliberareProdus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Eliberare...");
  lcd.setCursor(0, 1);
  lcd.print("Va multumim!");
  
  // cod motoare
  delay(3000); 
  stoc[produsSelectat - '1']--;
  meniuPrincipal(); 
}
