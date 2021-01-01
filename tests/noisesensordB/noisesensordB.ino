const int MIC = 35; //the microphone amplifier output is connected to pin A0
int adc;
int dB, PdB; //the variable that will hold the value read from the microphone each time

void setup() {
Serial.begin(19200); //sets the baud rate at 9600 so we can check the values the microphone is obtaining on the Serial Monitor
  //pinMode(35, OUTPUT);
}

void loop(){

  PdB = dB; //Store the previous of dB here
  
adc= analogRead(MIC); //Read the ADC value from amplifer 
//Serial.println (adc);//Print ADC for initial calculation 
dB = (adc-66.286) / 20.143; //Convert ADC value to dB using Regression values

//if (PdB!=dB)
Serial.print("dB = ");
Serial.print (dB);
Serial.print("-> adc = ");
Serial.println (adc);
delay(500);
}
