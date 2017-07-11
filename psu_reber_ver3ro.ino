/* http://nicuflorica.blogspot.ro/2014/03/arduino-ca-multimetru-3.html
 http://www.tehnic.go.ro
 http://www.arduinotehniq.com
 http://nicuflorica.blogspot.ro
amper & voltmeter by niq_ro, 02.2013, Craiova, Romania
vers. 1.3 - see http://nicuflorica.blogspot.ro/2013/02/arduino-ca-multimetru-2.html
actual 3.0 - 06.2017 - https://nicuflorica.blogspot.ro/2017/07/alimentator-reglabil-0-30v0-7a-cu-volt.html
 */

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3f, 16, 2);  // afisaj LCD1602 pe i2c cu adresa 27

// http://arduino.cc/en/Reference/LiquidCrystalCreateChar  // definim un simbol pentru grad celsius
byte grad[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
};

#define Pintensiune A1 // divizorul rezistiv pentru tensiune e legat la intrarea A1 
#define Pincurent A0 // rezistenta inseriata este conectata la A0

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2  // senzorul DS18B20 la pin D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define ventilator 3  // pin comanda ventiulator la D3
#define tiuitoare 4  // pin comanda avertizor la D4

unsigned long tpmasurare;  // variabila pentru memorare timp masurare (in ms)
unsigned long tpintremasurari = 30000;  // temperatura se masoara la 30 secunde

float t;  // variabila pentru temperatura
float temax = 40.0;  // temperatura maxima 40 grade
float dete = 2.0;    // histerezis temperatura

float r2 = 39.; // rezistenta conectata de la + la A1
float r1 = 1. ; // rezistenta conectata de la A1 la GND
float rsunt = 0.055 ; // valoare rezistenta (sunt)

float sumatensiune = 0.;   // valoare insumare tensiune pentru calcul medie
float sumacurent = 0.; // valoare insumare curent pentru calcul medie

int trcurent = 0;
int trtensiune = 0;

float curent = 0.;   // valoare curent
float tensiune = 0.; // valoare tensiune

void setup() {  // parte de program de ruleaza doar lapornire sau dupa restartare placa Arduino

sensors.begin();

pinMode(ventilator, OUTPUT);
digitalWrite(ventilator, LOW);
pinMode(tiuitoare, OUTPUT);
digitalWrite(tiuitoare, LOW);
  
analogReference(INTERNAL); // punem referinta interna de 1,1V;

  lcd.begin();  // initializare afisaj
  lcd.createChar(0, grad);  // crearea simbolului pentru grad Celsius
  lcd.backlight(); // aprindere lunmina de fundal
  
  // afisare text la inceput
  lcd.print("www.tehnic.go.ro");  
  lcd.setCursor(0, 1);
  lcd.print("creat de niq_ro");
  delay(2500);
  lcd.clear();
  
 lcd.print("indicator panou");  
  lcd.setCursor(0, 1);
  lcd.print("tensiune-curent");
  delay (2500);
  lcd.clear();
  
  lcd.setCursor(3, 0);
  lcd.print("Umax = 35V");  
  lcd.setCursor(3, 1);
  lcd.print("Imax = 15A");
  delay (2500);
  lcd.clear();

sensors.requestTemperatures(); // Send the command to get temperatures
t = sensors.getTempCByIndex(0);  
tpmasurare = millis();
}

void loop() {

  // pune in zero variabilele de insumare pentru a calcula ulterior tensiunea medie
  sumatensiune = 0;
  sumacurent = 0;
     
  for (int i=1; i <= 20; i++)
  {
  // citeste treptele de tensiune (0-1023) si adauga la o variabila de cumulare
  trcurent = analogRead(Pincurent);    //  citire valoare pe intrarea analogica 
  sumacurent = sumacurent + trcurent;  // cumuleaza valoarea

  trtensiune = analogRead(Pintensiune); 
  sumatensiune = sumatensiune + trtensiune;
  
  delay (20);  // pauza de 20ms intre masuratori
    }

// calculam valorile medii
sumacurent = sumacurent/20.;
sumatensiune = sumatensiune/20.;

// calculam valorile tensiunii si curentului de la sursa
curent = (float)1.1 / rsunt * sumacurent / 1024.0 ;
tensiune = (float)(r1+r2)/r1 * 1.1 * sumatensiune / 1024.0 ;
tensiune = tensiune - curent*rsunt;  // facem media rezultatelor si scadem caderea de tensiune de pe rezistenta sunt
  
lcd.clear();  
// partea de afisare
    lcd.setCursor(0, 0);
    lcd.print("U=");
    if (tensiune < 10.0) lcd.print(" ");  // daca tensiunea e mai mica de 10V 
    lcd.print(tensiune);
    lcd.print("V");
    
    lcd.setCursor(0, 1);
    lcd.print("I=");
    if (curent < 10.0) lcd.print(" ");  // daca curentul e mai mic de 10A 
    lcd.print(curent);
    lcd.print("A");

    lcd.setCursor(9, 1);
    lcd.print("t=");
    if (t < 10.0) lcd.print(" ");  // daca curentul e mai mic de 10A 
    lcd.print(t,0);  // afisam doar valoarea intreaga
    //   lcd.write(0b11011111);  // caracter asemanatpor cu gradul Celsius
    lcd.write(byte(0));  // simbolul de grad Celsius creat de mine
    lcd.print("C");

if (tensiune < 0.8)   // scurtcircuit   
{  
  lcd.setCursor(9, 0);
  lcd.print("scurt");
  digitalWrite(tiuitoare, HIGH);
  delay(100);
}
 else
{
  lcd.setCursor(9, 0);
  lcd.print("     ");
}

if (millis() - tpmasurare > tpintremasurari)
{
  sensors.requestTemperatures(); // se solicita efectuarea de masuratori
  t = sensors.getTempCByIndex(0) + 0.5;  // se adauga 0.5 grade pentru rotunjire ulterioara
  tpmasurare = millis();  // se memoreaza timpul citirii
}  

if (t > temax)   // daca s-a depasit temperatura maxima
{
  digitalWrite(ventilator, HIGH);   // alimentez ventilatorul
   lcd.setCursor(15, 0);
   lcd.print("*"); 
}
if (t < temax - dete) // daca a scazut temepratura sub pragul acceptat
{
  digitalWrite(ventilator, LOW);  // decuplez alimentarea ventialtorului
     lcd.setCursor(15, 0);
   lcd.print(" "); 
}

 digitalWrite(tiuitoare, LOW);   // fortez oprirea alrmei de scurtcircuit
}
