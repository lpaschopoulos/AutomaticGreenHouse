// ενσωμάτωση βιβλιοθήκες:
#include <Wire.h>                      
#include <LiquidCrystal_I2C.h> // ενσωμάτωση βιβλιοθήκης sda to sda(pin20)of the arduino mega, scl to scl(pin21) of the arduino mega
#include <OneWire.h>   // αισθητήρας θερμοκρασίας
#include <DallasTemperature.h>  // αισθητήρας θερμοκρασίας
#include "DHT.h" // αισθητήρας εσωτερικής θερμοκρασίας και υγρασίας
#include <AccelStepper.h> // Include the AccelStepper Library
#define DHTPIN 2     // συνδεδεμένο στο ποδαράκι 2 pin2 
#define DHTTYPE DHT11   // DHT 11
#define FULLSTEP 4 // ορισμός βήματος κινητήρα
#define STEP_PER_REVOLUTION 2048 // η τιμή αυτή είναι απο το datasheet. 


const int dry = 800; // ορισμός αισθητήρα υγρασίας. 800= ξερό
const int wet = 200; //ορισμός αισθητήρα υγρασίας. 200= υγρό
int tG, tE;  // οι μεταβλητές απο τους δύο αισθητήρες θερμοκρασίας (tG=θερμοκρασία εδάφους, tE=θερμοκρασία εξωτερικού χώρου)
int motion_1 = 4; // το ποδαράκι(pin4) στο οποίο είναι συνδεδεμένος ο αισθητήρας κίνησης 1
int motion_2 = 5; // το ποδαράκι(pin5) στο οποίο είναι συνδεδεμένος ο αισθητήρας κίνησης 2
int statemotion_1 = LOW; // από προεπιλογή, δεν ανιχνεύτηκε κίνηση για τον αισθητήρα κίνησης 1
int statemotion_2 = LOW; //από προεπιλογή, δεν ανιχνεύτηκε κίνηση για τον αισθητήρα κίνησης 2
int valmotion_1 = LOW; // μεταβλητή για την αποθήκευση της κατάστασης του αισθητήρα κίνησης 1 (τιμή)
int valmotion_2 = LOW; // μεταβλητή για την αποθήκευση της κατάστασης του αισθητήρα κίνησης 2 (τιμή)
int Led_String_1=30; //οδηγει στο ρελε για την λεντοταινια 1
int Led_String_2=32; //οδηγει στο ρελε για την λεντοταινια 2
int Led_Button_1=6; //pin6 για το κουμπι(pushbutton) της λεντοταινιας 1
int Led_Button_2=7; //pin7 για το κουμπι(pushbutton) της λεντοταινιας 2
int state_button_Led_1 = LOW; // μεταβλητή για την αποθήκευση της κατάστασης του κουμπιού(pushbutton) Led της λεντοταινιας 1
int state_button_Led_2 = LOW; // μεταβλητή για την αποθήκευση της κατάστασης του κουμπιού(pushbutton) Led της λεντοταινιας 2
int Light_Sensor_Pin = A1; // η είσοδος του αισθητήρα φωτός στο ποδαρακι (A1)
float light;
int light_value; // τιμή του αισθητήρα φωτός 
int state_motor = LOW; // ο κινητηρας βρισκεται στην θεση 'κλειστα παραθυρα'
int SoilHumidityPin = A0;  // η είσοδοσ του αισθητήρα στο ποδαράκι (Α0)
int SoilHumidity;    // ορίζει την αναλογική τιμή του αισθητήρα που διαβάζεται από το Arduino
int limit = 300;    // ορίζει ένα όριο (σε αυτήν την περίπτωση, εάν η τιμή του αισθητήρα είναι μεγαλύτερη από το όριο, τότε θα ανάψει ένα LED)
int persantageHumidity;

AccelStepper stepper(FULLSTEP, 11, 9, 10, 8); // Ορισμός των pin  IN1-IN3-IN2-IN4 για την σωστη συνεχεια του βηματος

DHT dht(DHTPIN, DHTTYPE); // αισθητήρας εσωτερικής θερμοκρασίας και υγρασίας
LiquidCrystal_I2C lcd(0x27, 20, 4); // οθόνη lcd

// ΑΙΣΘΗΤΗΤΑΣ ΘΕΡΜΟΚΡΑΣΙΑΣ
OneWire ds_1(52);  //Αισθητήρας θερμοκρασίας εδάφους (tG)
OneWire ds_2(53); // Αισθητήρας εξωτερικής θερμοκρασίας (tE)
//OneWire oneWire(ONE_WIRE_BUS);


//DallasTemperature sensors(&oneWire);
DallasTemperature sensor_1(&ds_1);
DallasTemperature sensor_2(&ds_2);


void setup() {

  
 // LCD ΟΘΟΝΗ
  lcd.init();
  lcd.backlight();
  
  // ξεκινάνε οι αισθητήρες
  sensor_1.begin(); //tG
  sensor_2.begin(); //tE
  dht.begin(); // αισθητήρας εσωτερικής θερμοκρασίας και υγρασίας

  //stepper.setMaxSpeed(1000.0);   // set the maximum speed
  stepper.setAcceleration(100); // ορισμός επιτάχυνσης
  stepper.setSpeed(800);         // ορισμός αρχικής ταχύτητας
  //stepper.setCurrentPosition(0); // ορισμός θέσης
  //stepper.moveTo(STEP_PER_REVOLUTION); // ορισμός στοχευμένης θέσης 64 βήματα <=> μια περστροφή


  Serial.begin(9600);     // διαβάζουμε τα δεδομένα στην σειριακή παρακολούθηση

  pinMode(Light_Sensor_Pin, INPUT); //pin(A1) για αισθητήρα φωτός
  pinMode(Led_String_1, OUTPUT);   // ορίζει σαν έξοδο την λεντοταινια 1
  pinMode(Led_String_2, OUTPUT);   // ορίζει σαν έξοδο την λεντοταινια 2
  pinMode(motion_1, INPUT); // ορίζουμε τον αισθητήρα κίνησης 1 ως είσοδο
  pinMode(motion_2, INPUT); // ορίζουμε τον αισθητήρα κίνησης 2 ως είσοδο
  pinMode(Led_Button_1, INPUT); // ορίζουμε το pin6 σαν εισοδο για το κουμπι της λεντοταινιας 1
  pinMode(Led_Button_2, INPUT); // ορίζουμε το pin7 σαν εισοδο για το κουμπι της λεντοταινιας 2
  
}

void loop() {

  // Περιμένετε μερικά δευτερόλεπτα μεταξύ των μετρήσεων.
  delay(500);

  
  //ΑΙΣΘΗΤΗΡΑΣ ΥΓΡΑΣΙΑΣ
  SoilHumidity = analogRead(SoilHumidityPin);      // ανάγνωση υγρασίας που υπάρχει στο έδαφος
  //persantageHumidity = map (SoilHumidity, wet, dry, 100, 0); // να οριζει απο το 0 εως το 100 την υγρασία
  persantageHumidity = 100 * ( SoilHumidity - dry) / (wet - dry); //μετατροπή της υγρασίας σε ποσοστό
  //Serial.println("Analog Value");   // εμφάνισε " ΑΝΑΛΟΓΙΚΗ ΤΙΜΗ: "
  //Serial.println(persantageHumidity);         // εμφάνισε μόνο την τιμή του αισθητήρα υγρασίας
  
  
  //ΑΙΣΘΗΤΗΡΑΣ ΘΕΡΜΟΚΡΑΣΙΑΣ
  //Serial.print(" Requesting temperatures..."); //Ζητάει θερμοκρασία ο αισθητήρας θερμοκρασίας. εμφανιση στην σειριακη οθονη
  //sensors.requestTemperatures(); // Ζητάει θερμοκρασία ο αισθητήρας θερμοκρασίας
   //Serial.print(sensors.getTempCByIndex(0)); //εμφάνισε στον υπολογιστή
   //Serial.print(sensors.getTempCByIndex(1)); //εμφάνισε στον υπολογιστή
   //temp1 = sensors.getTempCByIndex(0);
   //temp2 = sensors.getTempCByIndex(1);

  light_value = analogRead(Light_Sensor_Pin); //ανάγνωση ευαισθησίας του φωτός
  light = light_value * 0.0976;// υπολογισμός ποσοστού ευαισθησία του φωτός

  float tI = dht.readTemperature();  // Ανάγνωση εσωτερικής θερμοκρασίας ως Κελσίου (προεπιλογή) (pin2)
  int hum = dht.readHumidity(); // Ανάγνωση εσωτερικής υγρασίας (pin2)

  sensor_1.requestTemperatures();
  tG = sensor_1.getTempCByIndex(0); // διαδικασία εκκίνησης για την εμφάνιση της θερμοκρασίας εδάφους (pin52)
  sensor_2.requestTemperatures();
  tE = sensor_2.getTempCByIndex(0); // διαδικασία εκκίνησης για την εμφάνιση της εξωτερικής θερμοκρασίας (pin53) 
  
  
  
  //LCD ΟΘΟΝΗ
  lcd.setCursor(0, 0); // lcd.setCursor(colum, row) (στήλη, σειρά)
  // αισθητήρας υγρασίας εδάφους
  lcd.print("MoisG:");  // αυτό που εμφανίζει στην οθόνη lcd. Moisture Ground
  lcd.print(persantageHumidity);
  lcd.print("%");
  // αισθητήρας εσωτερικής υγρασίας
  lcd.print(" Hum:"); //Humidity
  lcd.print(hum);
  lcd.print("%");
  // αισθητήρας θερμοκρασίας εδάφους
  lcd.setCursor(0, 1);
  lcd.print("tG:"); //temperature ground
  lcd.print(tG);
  lcd.print("C ");
  //αισθητήρας εσωτερικής θερμοκρασίας
  lcd.print("tI:"); //temperature interior
  lcd.print(tI);
  lcd.print("C");
  //αισθητήρας εξωτερικής θερμοκρασίας
  lcd.setCursor(0,2);
  lcd.print("tE:"); //temperature exterior
  lcd.print(tE);




if (tE>=10 && tE<45){
  if (constrain (light, 51, 100) && state_motor == LOW ) {
    stepper.moveTo(STEP_PER_REVOLUTION);
    stepper.run();
    state_motor == HIGH;
  }
  if (constrain(light, 0, 50){
    //ανεμιστηρες σε χαμηλες στροφες ολο το βραδυ
  }
if (tE>=-5 && tE<10){
  if (constrain (light, 51, 100)&& tI>15 && hum>50) {
    //ανεμιστηρες σε χαμηλες στροφες 
  }
  else{
    // do something
  }
  if (constrain (light, 0, 50){
    // everything must be closed
  }
 } 
}

if (tI>25 && hum>50){
  stepper.moveTo(STEP_PER_REVOLUTION);
  stepper.run();
  state_motor == HIGH;
  // ανεμιστηρες στο φουλ
}


if (persantageHumidity < 80){
  // ποτισμα
}

if (hum > 60) {
  //ανεμιστηρες μετριες στροφες
}

  state_button_Led_1 = digitalRead(Led_Button_1); //διαβάστε την κατάσταση της τιμής του pushbuttonLED 1 (pin6)
  state_button_Led_2 = digitalRead(Led_Button_2); //διαβάστε την κατάσταση της τιμής του pushbuttonLED 2 (pin7)
if (state_button_Led_1 == HIGH)   { // ελέγξτε αν είναι πατημένο το κουμπί1. Εάν είναι, η κατάσταση είναι HIGH:
    digitalWrite(Led_String_1, LOW); // turn LED1 ON:
    lcd.setCursor(8,2);
    lcd.print("L1:"); 
    lcd.print("ON");
    delay(50);
  } else {
    digitalWrite(Led_String_1, HIGH);  // turn LED1 OFF:
    lcd.setCursor(8,2);
    lcd.print("L1:"); 
    lcd.print("OFF");
    delay(50);
  }
if (state_button_Led_2 == HIGH)   { // ελέγξτε αν είναι πατημένο το κουμπί2. Εάν είναι, η κατάσταση είναι HIGH:
    digitalWrite(Led_String_2, LOW); // turn LED2 ON:
    lcd.setCursor(14,2);
    lcd.print("L2:"); 
    lcd.print("ON");
    delay(50);
  } else {
    digitalWrite(Led_String_2, HIGH); // turn LED2 OFF:
    lcd.setCursor(14,2);
    lcd.print("L2:"); 
    lcd.print("OFF");
    delay(50);
  }  

if (constrain(light, 0, 50) ){ // (0-50) ειναι νυχτα
  if(state_button_Led_1 == LOW || state_button_Led_2 == LOW)  {
    valmotion_1 = digitalRead(motion_1); // ανάγνωση της τιμής του αισθητήρα κίνησης 1
    valmotion_2 = digitalRead(motion_2); // ανάγνωση της τιμής του αισθητήρα κίνησης 2
    if (valmotion_1 == HIGH || state_button_Led_1 == HIGH ) { // ελέγχει αν ο αισθητήρας κίνησης 1 είναι HIGH
      digitalWrite(Led_String_1, LOW);   // ανάψτε το LED1
      delay(100); // καθυστέρηση
      if (statemotion_1 == LOW) {
        lcd.setCursor(8,2);
        lcd.print("L1:"); // Το LED1 ειναι ON
        lcd.print("ON"); // Motion detected
        statemotion_1 = HIGH; // ενημέρωση μεταβλητής κατάστασης σε HIGH
      }
     }
    else if ( valmotion_1 == LOW && state_button_Led_1 == LOW ) {
      digitalWrite(Led_String_1, HIGH); // LED1 OFF
      delay(100);             // καθυστέρηση
      if (statemotion_1 == HIGH){
        lcd.setCursor(8,2);
        lcd.print("L1:"); //σβησε το LED1
        lcd.print("OFF"); //Motion stopped
        statemotion_1 = LOW;       // ενημέρωση μεταβλητής κατάστασης σε LOW
      }
    }
   if (valmotion_2 == HIGH || state_button_Led_2 == HIGH) { // ελέγχει αν ο αισθητήρας κίνησης 1 είναι HIGH
      digitalWrite(Led_String_2, LOW);   // ανάψτε το LED2
      delay(100); // καθυστέρηση
      if (statemotion_2 == LOW) {
        lcd.setCursor(14,2);
        lcd.print("L2:"); //Το LED2 ειναι ON
        lcd.print("ON"); //Motion detected
        statemotion_2 = HIGH; // ενημέρωση μεταβλητής κατάστασης σε HIGH
      }
    }
    else if ( valmotion_2 == LOW && state_button_Led_2 == LOW ) {
      digitalWrite(Led_String_2, HIGH); // LED2 OFF
      delay(100);             // καθυστέρηση
      if (statemotion_2 == HIGH){
        lcd.setCursor(14,2);
        lcd.print("L2:"); //σβησε το LED2
        lcd.print("OFF"); //Motion stopped
        statemotion_2 = LOW;       // ενημέρωση μεταβλητής κατάστασης σε LOW
      }
    }
  }
}



delay(50); 
}
  
