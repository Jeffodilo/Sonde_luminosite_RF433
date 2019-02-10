// ***
// *** Pinout ATtiny25/45/85:
// *** PDIP/SOIC/TSSOP
// *** =============================================================================================
// ***
// ***        (PCINT5/RESET/ADC0/dW) PB5      [1]*  [8]   VCC
// *** (PCINT3/XTAL1/CLKI/OC1B/ADC3) PB3 A3/3 [2]   [7]   A1/2 PB2 (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2)
// *** (PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4 A2/4 [3]   [6]   1    PB1 (MISO/DO/AIN1/OC0B/OC1A/PCINT1)
// ***                               GND      [4]   [5]   0    PB0 (MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0)
// ***

#include <SoftwareSerial.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include  "x10rf.h"

// ***** including low power consuption libraries // by default in Arduino library *****
#include <avr/sleep.h>    // Sleep Modes
#include <avr/wdt.h>      // Watchdog timer
#include <avr/interrupt.h>

// ***** consuption libraries function define *****
#ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// ***** Define the RX and TX pins. *****
#define RX    3   // *** PB3, Pin 2  // TX FTDI/232
#define TX    4   // *** PB4, Pin 3  // RX FTDI/232
SoftwareSerial Serial(RX, TX);

// ***** define I/O PINS *****
//#define ONE_WIRE_BUS 1 // *** PB1, Pin 6
#define LUX_PIN A1 // *** PB2, Pin 7

// ***** Declaration sonde Dallas DS18B20 *****
//OneWire _oneWire = OneWire(ONE_WIRE_BUS);
//DallasTemperature _sensors = DallasTemperature(&_oneWire);

// *** Declaration transmetteur RF et librarie X10RF *****
#define RF_OUT 0         // Pin number for the 433mhz OOK transmitter
#define reps 4        // Number of times that a RF command must be repeated.
//#define ledpin 13 //BLUE_LED  // Pin for the led that blinks when a command is send. (0 = no blink)
#define rfxLuxID 3 // PARAMETRAGE ID 3 => 0003 dans DOMOTICZ
//#define rfxTempID 4 // PARAMETRAGE ID 3 => 0003 dans DOMOTICZ

x10rf myx10 = x10rf(RF_OUT,0,reps); // x10rf myx10 = x10rf(tx,ledpin,reps) no blink led and send msg reps times

// ***** Time Sleep *****
int timesleep = 15; // Sleep for 8 s  8 x timesleep (ex:75) = 600 s = 10m

void setup()
{
  CLKPR = (1<<CLKPCE);  
  CLKPR = B00000000;  // set the fuses to 8mhz clock-speed.
  
  Serial.begin(9600);
  delay(100);
  Serial.println("Sonde luminosite et temperature en RF433 vers Domoticz");

  //_sensors.begin();

  pinMode(RF_OUT, OUTPUT); // sortie transmetteur
  myx10.begin();
  setup_watchdog(9); // approximately 8 seconds sleep
}

void loop()
{
  
  static uint32_t counter = 1;

  // *** Get the current temperature and display it.
  //_sensors.requestTemperatures();
  //double tempC = _sensors.getTempCByIndex(0);

  // *** Get the current luminosite and display it
  int lux = analogRead(LUX_PIN);
  
  //Serial.print(counter); Serial.print(": "); Serial.print(tempC, 1); Serial.print(" C, "); 
  Serial.print(lux);Serial.print(" lux, ");
  counter++;
    
  // *** Send value in RF
  //tempC = tempC*10;
 //Serial.print(": "); Serial.print(tempC, 1);
 // myx10.RFXmeter(rfxTempID,0,tempC);
  myx10.RFXmeter(rfxLuxID,0,lux);
  Serial.println(" => Envoi RF vers Domoticz");
  delay(100);

  // Sleep for 8 s  8 x 75 = 600 s = 10m
  for (int i=0; i<timesleep; i++){
  system_sleep();
  }
  delay(100);
}

// set system into the sleep state 
// system wakes up when wtchdog is timed out
void system_sleep() {
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_mode();                        // Go to sleep
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {
  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}
  
// Watchdog Interrupt Service / is executed when watchdog timed out 
ISR(WDT_vect) {   
  //wake up
  //count--;
} 
