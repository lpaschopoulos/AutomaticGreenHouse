// ενσωμάτωση βιβλιοθήκες:
#include <Wire.h>                      
#include <LiquidCrystal_I2C.h> // ενσωμάτωση βιβλιοθήκης οθόνης sda to sda(pin20)of the arduino mega, scl to scl(pin21) of the arduino mega
#include <OneWire.h>   //ενσωμάτωση βιβλιοθήκης του αισθητήρα θερμοκρασίας tG/tE
#include <DallasTemperature.h>  // αισθητήρας θερμοκρασίας
#include "DHT.h" // αισθητήρας εσωτερικής θερμοκρασίας και υγρασίας
#include <AccelStepper.h> //ενσωμάτωση βιβλιοθήκης βηματικού κινητήρα

#define DHTPIN 2     // συνδεδεμένο στο ποδαράκι 2 pin2 
#define DHTTYPE DHT11   // DHT 11
#define FULLSTEP 4 // ορισμός βήματος κινητήρα
#define STEP_PER_REVOLUTION 4096 // η τιμή αυτή είναι απο το datasheet

AccelStepper stepper(FULLSTEP, 11, 9, 10, 8); // Ορισμός των pin  IN1-IN3-IN2-IN4 για την σωστή συνέχεια του βήματος του κινητήρα
DHT dht(DHTPIN, DHTTYPE); // αισθητήρας εσωτερικής θερμοκρασίας και υγρασίας
LiquidCrystal_I2C lcd(0x27, 20, 4); // οθόνη lcd
// ΑΙΣΘΗΤΗΡΑΣ ΘΕΡΜΟΚΡΑΣΙΑΣ
OneWire ds_1(52);  //Αισθητήρας θερμοκρασίας εδάφους (tG)
OneWire ds_2(53); // Αισθητήρας εξωτερικής θερμοκρασίας (tE)
//DallasTemperature sensors(&oneWire);
DallasTemperature sensor_1(&ds_1);
DallasTemperature sensor_2(&ds_2);

const int dry = 800; // ορισμός αισθητήρα υγρασίας. 800= ξερό
const int wet = 200; //ορισμός αισθητήρα υγρασίας. 200= υγρό
const int fanSpeedLow=25; //χαμηλή ταχύτητα στροφών
const int fanSpeedMed=128; //μέση ταχύτητα στροφών
const int fanSpeedHigh=250; //υψηλή ταχύτητα στροφών
const int fanRelay=31; //εντολή στο ρελέ να δουλέψουν οι ανεμιστήρες
const int Led_Button_1=12; //pin12 για το κουμπι(pushbutton) της λεντοταινιας 1
const int Led_Button_2=6; //pin6 για το κουμπι(pushbutton) της λεντοταινιας 2
const int fanSpeedControl=3; // ορισμός του pin3 για τον έλεγχο των στροφών των ανεμιστήρων
int tG, tE;  // οι μεταβλητές απο τους δύο αισθητήρες θερμοκρασίας (tG=θερμοκρασία εδάφους, tE=θερμοκρασία εξωτερικού χώρου)
int motion_1 = 4; // το ποδαράκι(pin4) στο οποίο είναι συνδεδεμένος ο αισθητήρας κίνησης 1
int motion_2 = 5; // το ποδαράκι(pin5) στο οποίο είναι συνδεδεμένος ο αισθητήρας κίνησης 2
int Led_String_1=30; //οδηγει στο ρελε για την λεντοταινια 1
int Led_String_2=32; //οδηγει στο ρελε για την λεντοταινια 2
int Light_Sensor_Pin = A1; // η είσοδος του αισθητήρα φωτός στο ποδαρακι (A1)
int light;//αισθητήρας φωτός 0-50 είναι νύχτα και 50-100 είναι μέρα
int light_value; // τιμή του αισθητήρα φωτός
int SoilHumidityPin = A0;  // η είσοδοσ του αισθητήρα στο ποδαράκι (Α0)
int SoilHumidity;    // ορίζει την αναλογική τιμή του αισθητήρα που διαβάζεται από το Arduino
int persantageHumidity; // τιμή του ποσοστού της υγρασίας του εδάφους
int waterpump=33; //εντολή για το ρελέ της αντλίας νερού
int hum; // τιμή του ποσοστού εσωτερικής υγρασίας
int tI; // τιμή εσωτερικής θερμοκρασίας
int value_motion_1; // τιμή αισθητήρα κίνησης 1 (HIGH)=παρουσία (LOW)=απουσία
int value_motion_2;// τιμή αισθητήρα κίνησης 2 (HIGH)=παρουσία (LOW)=απουσία
int state_button_Led_1; // κατάσταση του μπουτόν 1 που οδηγεί στην λεντοταινία 1 
int state_button_Led_2; // κατάσταση του μπουτόν 1 που οδηγεί στην λεντοταινία 1
unsigned long lcdUpdateInterval = 1000; // Ανανέωση της lcd οθόνης κάθε 1sec
unsigned long previousLcdUpdateMillis = 0; 
unsigned long ledUpdateInterval = 5; // Ανανέωση των led κάθε 5msec
unsigned long previousLedUpdateMillis = 0;
bool pumpOn = false; // παρακολούθηση κατάστασης της αντλίας νερού


void setup() {
  // ξεκίνημα της LCD οθόνης
  lcd.init();
  lcd.backlight();
  // ξεκινάνε οι αισθητήρες θερμοκρασίας
  sensor_1.begin(); //tG του εδάφους
  sensor_2.begin(); //tE του εξωτερικού χώρου
  dht.begin(); // αισθητήρας εσωτερικής θερμοκρασίας και υγρασίας
  stepper.setMaxSpeed(2000.0);   // ορισμός μέγιστης ταχύτητας του βηματικού κινητήρα
  stepper.setAcceleration(100); // ορισμός επιτάχυνσης του βηματικού κινητήρα
  stepper.setSpeed(1000);         // ορισμός αρχικής ταχύτητας του βηματικού κινητήρα
  stepper.setCurrentPosition(0); // ορισμός θέσης του βηματικού κινητήρα

  pinMode(Light_Sensor_Pin, INPUT); //pin(A1) για αισθητήρα φωτός
  pinMode(waterpump, OUTPUT); // ορίζει σαν έξοδο το pin που οδηγεί στο ρελέ για την αντλία νερού
  digitalWrite(waterpump, HIGH); //αρχική κατάσταση του ρελέ της αντλίας νερου HIGH δηλαδή OFF
  pinMode(Led_String_1, OUTPUT);   // ορίζει σαν έξοδο την λεντοταινια 1
  digitalWrite(Led_String_1, HIGH); //αρχική κατάσταση του Led_String_1 HIGH δηλαδή OFF στο ρελέ(τα φώτα κλειστά)
  pinMode(Led_String_2, OUTPUT);   // ορίζει σαν έξοδο την λεντοταινια 2
  digitalWrite(Led_String_2, HIGH); //αρχική κατάσταση του Led_String_2 HIGH δηλαδή OFF στο ρελέ(τα φώτα κλειστά)
  pinMode(motion_1, INPUT); // ορίζουμε τον αισθητήρα κίνησης 1 ως είσοδο
  pinMode(motion_2, INPUT); // ορίζουμε τον αισθητήρα κίνησης 2 ως είσοδο
  pinMode(Led_Button_1, INPUT); // ορίζουμε το pin12 σαν εισοδο για το κουμπι της λεντοταινιας 1
  pinMode(Led_Button_2, INPUT); // ορίζουμε το pin6 σαν εισοδο για το κουμπι της λεντοταινιας 2
  pinMode(fanSpeedControl, OUTPUT); // ορισμός του fanSpeedControl σαν έξοδο
  pinMode(fanRelay, OUTPUT); // ορίζει σαν έξοδο το pin που οδηγεί στο ρελέ για τους ανεμιστήρες
  analogWrite(fanSpeedControl, fanSpeedLow); // αρχική ταχύτητα ανεμιστήρων σε 0% ταχύτητα. 0(off) και 255(full speed)
  Serial.begin(9600); // διαβάζουμε τα δεδομένα στην σειριακή παρακολούθηση
}
void loop() {
  state_button_Led_1 = digitalRead(Led_Button_1);// διαβάζουμε την κατάσταση του μπουτόν 1 της λεντοταινίας 1
  state_button_Led_2 = digitalRead(Led_Button_2);// διαβάζουμε την κατάσταση του μπουτόν 2 της λεντοταινίας 2
  value_motion_1 = digitalRead(motion_1); // διαβάζουμε την κατάσταση του αισθητήρα κίνησης 1
  value_motion_2 = digitalRead(motion_2);// διαβάζουμε την κατάσταση του αισθητήρα κίνησης 2
  unsigned long currentMillis = millis();
    if (currentMillis - previousLedUpdateMillis >= ledUpdateInterval) {
    updateLED();
    previousLedUpdateMillis = currentMillis;
    }
  tI = dht.readTemperature();  // Ανάγνωση εσωτερικής θερμοκρασίας ως Κελσίου (προεπιλογή) (pin2)
  hum = dht.readHumidity(); // Ανάγνωση εσωτερικής υγρασίας (pin2)
  int currentPosition = stepper.currentPosition();// έλεγχος της τρέχουσας θέσης του μοτέρ
  sensor_1.requestTemperatures();
  tG = sensor_1.getTempCByIndex(0); // διαδικασία εκκίνησης για την εμφάνιση της θερμοκρασίας εδάφους (pin52)
  sensor_2.requestTemperatures();
  tE = sensor_2.getTempCByIndex(0); // διαδικασία εκκίνησης για την εμφάνιση της εξωτερικής θερμοκρασίας (pin53) 
  //ΑΙΣΘΗΤΗΡΑΣ ΥΓΡΑΣΙΑΣ
  SoilHumidity = analogRead(SoilHumidityPin);      // ανάγνωση υγρασίας που υπάρχει στο έδαφος
  persantageHumidity = 100 * ( SoilHumidity - dry) / (wet - dry); //μετατροπή της υγρασίας σε ποσοστό
  light_value = analogRead(Light_Sensor_Pin); // (0-50) ειναι νυχτα
  light = light_value * 0.0976;
  
    if (tI<27){
      analogWrite(fanSpeedControl, fanSpeedLow);
      lcd.setCursor(8,3);
      lcd.print("10% ");
    }else if (tI>=27&&tI<36){
      analogWrite(fanSpeedControl, fanSpeedMed);
      lcd.setCursor(8,3);
      lcd.print("50% ");
    }else if(tI>=36){
      analogWrite(fanSpeedControl, fanSpeedHigh);
      lcd.setCursor(8,3);
      lcd.print("100% "); 
    }
       
    if (hum>=85 && currentPosition == 0){
      stepper.runToNewPosition(5120);
    }else if (hum<85 && currentPosition == 5120 ){
      stepper.runToNewPosition(0);
    }
    
  if (persantageHumidity <= 17 && !pumpOn) {
    // Η υγρασία είναι κάτω από 17%, και η αντλία δεν είναι ήδη ενεργοποιημένη
    digitalWrite(waterpump, LOW); // Ενεργοποιήστε την αντλία νερού
    pumpOn = true; //  κατάσταση της αντλίας σε ενεργοποιημένη
  } else if (persantageHumidity >= 71 && pumpOn) {
    // Η υγρασία είναι 71% ή μεγαλύτερη και η αντλία είναι ενεργοποιημένη αυτήν τη στιγμή
    digitalWrite(waterpump, HIGH); // Απενεργοποιήστε την αντλία νερού
    pumpOn = false; // κατάσταση της αντλίας σε απενεργοποιημένη
  }

        
    if (light>=0 && light<50){// έλεγχος εαν είναι μέρα ή νύχτα (0-50 είναι νύχτα)
      lcd.setCursor(13, 3);
      lcd.print("Night");
    }else if (light>=50 && light<=100){ // αλλίως εάν ειναι μέρα (50-100 είναι μέρα)
      lcd.setCursor(13, 3);
      lcd.print("Day  ");//γράψε μέρα Day
    }

    if (currentMillis - previousLcdUpdateMillis >= lcdUpdateInterval) {
    updateLCD(); // ενημέρωση της οθόνης LCD σε τακτά χρονικά διαστήματα ώστε να αποφεύγεται η υπερβολική ενημέρωση 
    previousLcdUpdateMillis = currentMillis;
    }
}
void updateLED() {
    if ((value_motion_1 == HIGH && (light>=0 && light<50)) || state_button_Led_1 == HIGH) { //έλεγχος εαν η κατάσταση του αισθητήρα κίνησης 1 είναι HIGH και είναι νύχτα ή η κατάσταση του μπουτόν 1 ειναι HIGH
      digitalWrite(Led_String_1, LOW); // ενεργοποίησε την λεντοταινία 1
      lcd.setCursor(10, 2);
      lcd.print("ON ");
    } else if (value_motion_1 == LOW || state_button_Led_1 == LOW) { //αλλίως εάν η κατάσταση του αισθητήρα κίνησης 1 είναι LOW ή η κατάσταση του μπουτόν 1 ειναι LOW
      digitalWrite(Led_String_1, HIGH); // απενεργοποίηση της λεντοταινίας 1
      lcd.setCursor(10, 2);
      lcd.print("OFF");
    }
    if ((value_motion_2 == HIGH && (light>=0 && light<50)) || state_button_Led_2 == HIGH) { //έλεγχος εαν η κατάσταση του αισθητήρα κίνησης 2 είναι HIGH και είναι νύχτα ή η κατάσταση του μπουτόν 2 ειναι HIGH
      digitalWrite(Led_String_2, LOW); // ενεργοποίησε την λεντοταινία 2
      lcd.setCursor(17, 2);
      lcd.print("ON ");
    } else if (value_motion_2 == LOW || state_button_Led_2 == LOW) { //αλλίως εάν η κατάσταση του αισθητήρα κίνησης 2 είναι LOW ή η κατάσταση του μπουτόν 2 ειναι LOW
      digitalWrite(Led_String_2, HIGH);  // απενεργοποίηση της λεντοταινίας 2
      lcd.setCursor(17, 2);
      lcd.print("OFF");
    }
}
void updateLCD() {
    //LCD ΟΘΟΝΗ
    lcd.setCursor(0, 0); // lcd.setCursor(colum, row) (στήλη, σειρά)
    // αισθητήρας υγρασίας εδάφους
    lcd.print("MoisG:");  // αυτό που εμφανίζει στην οθόνη lcd. Moisture Ground
    lcd.print(persantageHumidity);
    lcd.print("% ");
    // αισθητήρας εσωτερικής υγρασίας
    lcd.setCursor(11, 0);
    lcd.print("Hum:"); //Humidity
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
    lcd.print("C");
    lcd.setCursor(7,2);
    lcd.print("L1:");
    lcd.setCursor(14,2);
    lcd.print("L2:");
    lcd.setCursor(0,3);
    lcd.print("AirFlow:");
    }
