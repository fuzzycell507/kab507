#include <math.h>

int count = 1;
int peltier = 3; //The N-Channel MOSFET is on digital pin 3
int power = 0; //Power level fro 0 to 99%
int peltier_level = int(map(power, 0, 99, 0, 255)); //This is a value from 0 to 255 that actually controls the MOSFET

String mode = "tuning";

//-------- fuzzy block

float n = 5.0; // число термов
float w = 1.0; // весовой коэффициент
float NewWeight;
float OldWeight = 1.0;
float weight;


float DX = 20.0f; // ширина основания
float A_1;
float A_2;
float A_3;
float MF_1;
float MF_2;
float MF_3;
float MF_4;
float MF_5;

float M1 = 0.0f;
float M2 = 25.0f;
float M3 = 50.0f;
float M4 = 75.0f;
float M5 = 100.0f;

float temp1;
float temp2;
float temp3;
float temp4;
float temp5;

float S11;

float S21;
float S22;
float S23;
float S24;
float S25;

float S33;

float D; // отношение S33/S11

float y_defuzzy;

float temper;
float fuzzy;

//-------- 


// PID
 // Установка задающий сигнал
float setpoint;
float new_setpoint;
float Threshold = 0.1f;
float input1; // Для нечеткой системы 
float input2; // для обучения
float threshold_2 = 0.1; // для обучения
float ko = 0.7;
float k1 = 0.02f; // коэффициент обучения
float temp;
float record_temp;

float TMPmeasurement()
{
  float v1, v2, v3;
  float temperature1, temperature2, temperature3;
  float temperature;
  v1 = analogRead(A2) * 5.0f / 1024.0f;
  temperature1 = (1.1488f * v1 * v1) - (28.452f * v1) + 87.857f;
  delay(1);

  v2 = analogRead(A2) * 5.0f / 1024.0f;
  temperature2 = (1.1488f * v2 * v2) - (28.452f * v2) + 87.857f;
  delay(1);

  v3 = analogRead(A2) * 5.0f / 1024.0f;
  temperature3 = (1.1488f * v3 * v3) - (28.452f * v3) + 87.857f;

  temperature = (temperature1 + temperature2 + temperature3) / 3.0;
  
  return temperature;
}

float FuzzyMAR(float _input, float _w)
{
  MF_1 = MembershipFunction(10.0, 20.0, 30.0, _input);
  MF_2 = MembershipFunction(20.0, 30.0, 40.0, _input);
  MF_3 = MembershipFunction(30.0, 40.0, 50.0, _input);
  MF_4 = MembershipFunction(40.0, 50.0, 60.0, _input);
  MF_5 = MembershipFunction(50.0, 60.0, 70.0, _input);

//  MF_1 = MembershipFunction(10.0, 20.0, 30.0, setpoint);
//  MF_2 = MembershipFunction(20.0, 30.0, 40.0, setpoint);
//  MF_3 = MembershipFunction(30.0, 40.0, 50.0, setpoint);
//  MF_4 = MembershipFunction(40.0, 50.0, 60.0, setpoint);
//  MF_5 = MembershipFunction(50.0, 60.0, 70.0, setpoint);

  temp2 = MF_2;
  temp3 = MF_3;
  temp4 = MF_4;

  MF_2 = max(temp2, temp3)*1.4;
  MF_3 = temp3*0.05;
  MF_4 = max(temp4,temp3)*1.4;
  
  //---------- MAR

  S11 = n * DX * _w / 1.4;

  S21 = Square(MF_1);
  S22 = Square(MF_2);
  S23 = Square(MF_3);
  S24 = Square(MF_4);
  S25 = Square(MF_5);
  S33 = S21 + S22 + S23 + S24 + S25;
    
  D = S33 / S11;
  
  //if(setpoint < 40)
  if(_input < 40)
  {
    y_defuzzy = (D * (70.0 - 10.0)) + 10.0;
  }  
  else
  {
    y_defuzzy = 70 - ((D * (70.0 - 10.0)) + 10.0);
  }
  return y_defuzzy;
}

float FuzzyCoG(float _input, float _w)
{
  MF_1 = MembershipFunction(0.0, 10.0, 20.0, _input);
  MF_2 = MembershipFunction(10.0, 20.0, 30.0, _input);
  MF_3 = MembershipFunction(20.0, 30.0, 40.0, _input);
  MF_4 = MembershipFunction(30.0, 40.0, 50.0, _input);
  MF_5 = MembershipFunction(40.0, 50.0, 60.0, _input);

//  MF_1 = MembershipFunction(10.0, 20.0, 30.0, setpoint);
//  MF_2 = MembershipFunction(20.0, 30.0, 40.0, setpoint);
//  MF_3 = MembershipFunction(30.0, 40.0, 50.0, setpoint);
//  MF_4 = MembershipFunction(40.0, 50.0, 60.0, setpoint);
//  MF_5 = MembershipFunction(50.0, 60.0, 70.0, setpoint);

  temp1 = MF_1;
  temp2 = MF_2;
  temp3 = MF_3;
  temp4 = MF_4;
  temp5 = MF_5;
  
 
  //---------- MAR

  S11 = MF_1 + MF_2 + MF_3 + MF_4 + MF_5;

  S21 = MF_1 * M1;
  S22 = MF_2 * M2;
  S23 = MF_3 * M3;
  S24 = MF_4 * M4;
  S25 = MF_5 * M5;
  
  S33 = S21 + S22 + S23 + S24 + S25;
    
  y_defuzzy = S33 * _w / S11;
    
  return y_defuzzy;
}

float MembershipFunction(float Left, float Center, float Right, float Input)
{
  float mu;
  if ( (Input >= Left) && (Input <= Center) )  
    mu = (Input - Left) / (DX / 2);  
  else if ( (Input > Center) && (Input <= Right) )  
    mu = (Right - Input) / (DX / 2);  
  else
    mu = 0.0f;
  return mu;
}

float Square(float mu)
{    
  float square = mu * DX / 2.0;
  return square;
}



float computePID(float _temp2, float _setpoint, float _power) {
    
   float w;
    
    w = (_setpoint - _temp2) * ko;
    _power = _power + w;
   
    return _power;
}

void SetPeltierLevel(int _power)
{
  if(_power > 100) _power = 100;
  if(_power < 0) _power = 0;  
  peltier_level = int(map(_power, 0, 100, 0, 255));  
  analogWrite(peltier, peltier_level); //Write this new value out to the port
}

void setup(){
  Serial.begin(9600);
  pinMode(peltier, OUTPUT);
  setpoint = 25.0;
  new_setpoint = setpoint;
  record_temp = 0;
}

void loop(){
  
  if (Serial.available() > 0)
  {
    new_setpoint = Serial.parseInt();
    if(new_setpoint != setpoint)
    {
      count = 1;
      setpoint = new_setpoint;
    }        
  }
  temper = 23.0f;
  //temper = TMPmeasurement();
  if (count == 1)
  {    
    //fuzzy = FuzzyMAR(temper, 1.0f);
    fuzzy = FuzzyCoG(temper, 1.0f);
    power = int(fuzzy);  

    Serial.print(fuzzy);
    Serial.print(",  ");
    Serial.println(power);
    
    count = 1;
    //mode = "learning";  
  }
  if (count == 2)
  {
    NewWeight = (temper - setpoint) * k1;
    weight = OldWeight + NewWeight;
    //if(weight < -1) break;
    //if(weight > 2) break;
    power = FuzzyMAR(temper, weight);
    SetPeltierLevel(power);
    OldWeight = weight;    
    Serial.print(weight);
    Serial.print(",  ");
    Serial.println(power);    
  }
  

  /*
  Serial.print(setpoint);
  Serial.print(", ");
  Serial.print(temper);
  Serial.print(" , ");
  Serial.print(fuzzy);
  Serial.print(" , ");
  Serial.println(peltier_level);
  */


/*if (Serial.available() > 0) {
    setpoint = Serial.parseInt();
    //Serial.println(setpoint);    
  }

if (mode == "tuning");
{
 
    
  power = int(y_defuzzy);
  
  
  if(power > 70) power = 70;
  if(power < 0) power = 0;
  
  peltier_level = int(map(power, 0, 70, 0, 255));  
  analogWrite(peltier, peltier_level); //Write this new value out to the port
  mode = "learning";
}  

if (mode == "learning")
{
  if ((input2 <= setpoint + threshold_2) && (input2 >= setpoint - threshold_2))                    
    {
      peltier_level = int(map(power, 0, 60, 0, 255));
      analogWrite(peltier, peltier_level); //Write this new value out to the port
      float p1 = setpoint +threshold_2;
      float p2 = setpoint -threshold_2;
    }
  else
    {
      temp = 0;
      //float power_f = computePID(input2, setpoint, temp);
      float power_f1;
      power = int(computePID(input2, setpoint, temp));
      if(power > 100) power = 100;
      if(power < 0) power = 0;
      peltier_level = int(map(power, 0, 100, 0, 255));     
      analogWrite(peltier, peltier_level); //Write this new value out to the port
      if ( count == 1)      temp = y_defuzzy;
      else temp = record_temp;
      while(input2 <= setpoint-threshold_2 || input2 >= setpoint+threshold_2)
      {
        
                
        if ( count == 1)
        {
          power_f1 = computePID(input2, setpoint, temp);
          peltier_level = int(map(power_f1, 0, 100, 0, 255));     
          analogWrite(peltier, peltier_level); //Write this new value out to the port
          float y_defuzzy_new = power_f1;
          temp = y_defuzzy_new;
        }
        else
        {
          temp = record_temp;
        }
          TMPmid_1 = TMPmeasurement ();
          delay(1);
          TMPmid_2 = TMPmeasurement ();
          delay(1);
          TMPmid_3 = TMPmeasurement ();
          delay(1);

          input2 = (TMPmid_1 + TMPmid_2 + TMPmid_3) / 3.0;
          Serial.print(setpoint);
          Serial.print(", ");
          Serial.print(input2);
          Serial.print(" , ");
          Serial.println(power);

      
        if ((input2 <= setpoint + threshold_2) && (input2 >= setpoint - threshold_2)) 
        {
          break;
        }
        
      }
      
     
      peltier_level = int(map(power, 0, 100, 0, 255));      
      analogWrite(peltier, peltier_level); //Write this new value out to the port
      if (Serial.available() > 0) {
        setpoint = Serial.parseInt();
        count =1;
        //Serial.println(setpoint);    
      }
      else{
       count =2;
       record_temp = temp;
      } 
     
   }    
}




analogWrite(peltier, peltier_level); //Write this new value out to the port

Serial.print(setpoint);
Serial.print(", ");
Serial.print(input2);
Serial.print(" , ");
Serial.println(power);

//delay(500);
*/
}
