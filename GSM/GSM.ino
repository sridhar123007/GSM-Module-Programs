#include<SoftwareSerial.h>
SoftwareSerial my(6,7);

void setup() {
  // put your setup code here, to run once:
my.begin(9600);
Serial.begin(9600);
Serial.println("GSM Start");
}

void loop() {
  // put your main code here, to run repeatedly:
if(Serial.available()>0){
  switch(Serial.read()){
   case 's':
        sending();
        break;
   case 'r':
        receiving();
        break;
  }

if(my.available()>0){
  Serial.write(my.read());
  }
  
}
}
void sending(){
  Serial.println("..................");
  my.println("AT+CMGF=1");
  delay(1000);
  my.println("AT+CMGS=\"+916281138630\"\r");
  delay(1000);
  my.println("I am ");
  delay(1000);
  my.println((char)26);
  delay(1000);
  Serial.println("......Done............");
}
void receiving(){
  my.println("AT+CNMI=0,2,2,0,0,0");
}
