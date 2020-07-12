#include <LiquidCrystal_I2C.h>  
#include <DHT.h>                

#define DHTPIN 10       
#define DHTTYPE DHT11   

LiquidCrystal_I2C lcd(0x27, 16, 2);  
DHT dht(DHTPIN, DHTTYPE);            

byte thermometer_icon[8] = {
  0b00100,
  0b00110,
  0b00100,
  0b00110,
  0b00100,
  0b01110,
  0b01110,
  0b00000
};

byte drop_icon[8] = {
  0b00100,
  0b00100,
  0b01110,
  0b01110,
  0b10111,
  0b10111,
  0b01110,
  0b00000
};

byte degree_icon[8] = {
  0b00010,
  0b00101,
  0b00010,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte fan_icon[8] = {
  0b00000,
  0b11001,
  0b01011,
  0b00100,
  0b11010,
  0b10011,
  0b00000,
  0b00000
};

byte arrow_icon[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};

int buzzer_pin = 13;                     
int fan_pin = 9;                         
int turn_on_button_pin = 2;         
int manual_control_button_pin = 3;    
                                       
float min_temperature = 14;           
float max_temperature = 34;          
float manual_fan_speed = 100;              
float press_length_millisSeconds = 0;  
boolean automatic_control = true;            

float temperature;                 
int air_humidity;                    
int fan_speed;             
int fan_speedLCD;          

void setup(){
  pinMode(buzzer_pin, OUTPUT);             
  pinMode(fan_pin, OUTPUT);          
  pinMode(turn_on_button_pin, INPUT);  
  pinMode(manual_control_button_pin, INPUT);  

  lcd.createChar(0, thermometer_icon); 
  lcd.createChar(1, drop_icon);
  lcd.createChar(2, degree_icon);
  lcd.createChar(3, fan_icon);
  lcd.createChar(4, arrow_icon);
  
  lcd.begin();                  
  lcd.print("Turn on        "); 
  lcd.write((byte)4);           
 
  dht.begin(); 
}

void loop() {
  temperature = dht.readTemperature();  
  air_humidity = dht.readHumidity();      
  
  if(!isnan(temperature) && !isnan(air_humidity)){    
    if(digitalRead(turn_on_button_pin) == HIGH){   
      fanSpeedMapping();
      displayInfo();
      switchToManualControl();
      standardsViolation();
      turn_on_button_pin = HIGH;                   
    }
  }
}

/*
Procedure standardsViolation() is used to handle case when
kiedy temperature temperature reading from sensor is too high or too low. 
In this case, the buzzer is activated, which inforoms the user of
the event. It beeps for 2 seconds, then goes to sleep for 5 seconds.
It will do this until the temperature reading returns to normal.
*/

  void standardsViolation(){
    if(temperature >= max_temperature){
      digitalWrite(buzzer_pin, HIGH);
      delay(2000);
      digitalWrite(buzzer_pin, LOW);
      delay(5000);
    }else if (temperature <= min_temperature){
      digitalWrite(buzzer_pin, HIGH);
      delay(2000);
      digitalWrite(buzzer_pin, LOW);
      delay(5000);
    }
}

/*
Procedure fanSpeedMapping() is used to mapping:
  - the temperature reading range for a suitable motor operating range (fan_speed)
  - temperature reading range per appropriate percent of motor operating range for a properly adjusted motor operating range. 
    It is displayed on the LCD screen (fan_speedLCD)
Mapping will not be performed when:
  - temperature will be outside the acceptable range
  - manual control mode is enabled
*/
void fanSpeedMapping(){
    if(temperature >= min_temperature && temperature <= max_temperature){
      if(automatic_control == true){
        fan_speed = map(temperature, min_temperature, max_temperature, 100, 250);   
        fan_speedLCD = map(temperature, min_temperature, max_temperature, 0, 100);
        analogWrite(fan_pin, fan_speed);
      }else{
        analogWrite(fan_pin, manual_fan_speed);
      }
    }
}

/*
Procedure displayInfo() is used to displays all the 
information you need such as:
  - temperature in degrees Celsius
  - the percentage of humidity of air
  - the percentage of fan speed
  - the current mode of the fan
with the symbols defined at the beginning of the program.
*/
void displayInfo(){
    lcd.home(); 
    lcd.write((byte)0); 
    lcd.print(":");
    lcd.print(temperature); 
    lcd.write((byte)2); 
    lcd.print("C");
    lcd.print(" ");
    lcd.write((byte)1); 
    lcd.print(":");
    lcd.print(air_humidity); 
    lcd.print("% ");
    lcd.setCursor(0, 1);
    lcd.write((byte)3);
    lcd.print(":");
    
    if(automatic_control == true){
      lcd.print(fan_speedLCD);
      lcd.print("%");
      lcd.print("   Autom. ");
    }else{
      lcd.print(map(manual_fan_speed, 100, 250, 0, 100)); 
      lcd.print("%");
      lcd.print("   Manual ");
    }
}

/*
Procedure switchToManualControl(int przycisk) is used to to switch between fan modes. 
Length of pressing the blue button is remembered. Depending on the value read, 
the conditional statement is executed:
  - first reading of the length of the button press within the limits <100,2000)
    will cause the motor to go into manual mode and its speed is 10%.
    Each time user presses the blue button, this value is increased by 10%.
  - if the button is held for 2 seconds or more, it switches to automatic mode
    and resets the manual speed to its initial state.
*/
void switchToManualControl(){
  while(digitalRead(manual_control_button_pin) == HIGH){
    delay(100);
    press_length_millisSeconds = press_length_millisSeconds + 100;
  }
  
  if(press_length_millisSeconds >= 100 && press_length_millisSeconds < 2000){ 
    automatic_control = false;
    singlePressManualControlButton();
  }else if(press_length_millisSeconds >= 2000){
    automatic_control = true;
    manual_fan_speed = 100;
  }
  press_length_millisSeconds = 0;
}

/*
Procedure singlePressManualControlButton() is used for manual engine control.
When in manual motor control mode, pressing the blue button once 
will increase the motor operation by 10%.
When maximum speed is reached, again
pressing the button will return you to the initial value, 10%
*/
void singlePressManualControlButton(){   
    if(manual_fan_speed >= 100 && manual_fan_speed <=240){
      manual_fan_speed += 15;
    }else if (manual_fan_speed > 249){
      manual_fan_speed = 115;
    }
}
