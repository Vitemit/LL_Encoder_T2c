/*
 * Simplified usage example for the LL_Encoder_T2c library.
 * This example demonstrates basic functionality, pin configuration with external pull-ups,
 * polling via update(), and testing primary methods (counts, pulses, directions, and key states) via Serial.
 */

#include <LL_Encoder_T2c.h>

#define A 2
#define B 3
#define KEY 4

uint8_t Get_Count = 0;
uint8_t Get_Count_Key = 0;
uint8_t Last_Value_Count = 0;
uint8_t Last_Value_Count_Key = 0;
uint8_t Get_Direction_Code = 0;
uint8_t Get_Direction_Code_Key = 0;
bool Get_Key_State = false;
bool Last_Get_Key_State = false;
bool Pulse_Right = false;
bool Pulse_Left = false;
bool Pulse_Right_Key = false;
bool Pulse_Left_Key = false;
int Last_Count_Value = 0;
int Last_Count_Value_Key = 0;
int Sum_Count = 0;
int SumCount_Key = 0;
char command;

LL_Encoder_T2c(A, B, KEY, LL_EXTERNAL_PULLUP, LL_EXTERNAL_PULLUP) encoder;

void setup() {
  Serial.begin(115200);
  encoder.begin(); // Initialize and configure GPIO pins (pin modes, pull-up resistors, and port registers) 


}

void loop() {
  encoder.update(); // Polls encoder inputs and processes the Mealy state machine


// Method: GetCount()

  Get_Count = encoder.GetCount();
  if (Get_Count != Last_Value_Count){
    Serial.print("Count: ");
    Serial.println(Get_Count);
  }
  Last_Value_Count = Get_Count;

// Method: GetPulseRight()

  Pulse_Right = encoder.GetPulseRight();
  if (Pulse_Right == true){
    Serial.println("Pulse_Right");
  }

// Method: GetPulseLeft()

  Pulse_Left = encoder.GetPulseLeft();
  if (Pulse_Left == true){
    Serial.println("Pulse_Left");
  }

// Method: GetDirectionCode()

  Get_Direction_Code = encoder.GetDirectionCode();
  if (Get_Direction_Code != 0){
    Serial.print("Direction Code: ");
    Serial.println(Get_Direction_Code, BIN);
  }

// Method: ResetCount() and ResetCountKey()

  if (Serial.available() >0){
    command = Serial.read();
    if (command == 'r'){        // Send 'r' via Serial to reset the encoder count using ResetCount().                                     
      encoder.ResetCount();
      Get_Count = 0;
    } 
    else if (command == 'k'){  // Send 'k' via Serial to reset the encoder key count using ResetCountKey().
      encoder.ResetCountKey();
      Get_Count_Key = 0;
    }
  }

// Method: GetKeyState()

  Get_Key_State = encoder.GetKeyState();
  if (Get_Key_State && !Last_Get_Key_State){
    Serial.println("Key press");
  }
  Last_Get_Key_State = Get_Key_State;

// Method: GetCountKey()

  Get_Count_Key = encoder.GetCountKey();
  if (Get_Count_Key != Last_Value_Count_Key){
    Serial.print("Count Key: ");
    Serial.println(Get_Count_Key);
  }
  Last_Value_Count_Key = Get_Count_Key;

// Method: GetPulseRightKey()

    Pulse_Right_Key = encoder.GetPulseRightKey();
  if (Pulse_Right_Key == true){
    Serial.println("Pulse_Right_Key");
  }

// Method: GetPulseLeftKey()

  Pulse_Left_Key = encoder.GetPulseLeftKey();
  if (Pulse_Left_Key == true){
    Serial.println("Pulse_Left_Key");
  }

// Method: GetDirectionCodeKey()

  Get_Direction_Code_Key = encoder.GetDirectionCodeKey();
  if (Get_Direction_Code_Key != 0){
    Serial.print("Direction Code Key: ");
    Serial.println(Get_Direction_Code_Key, BIN);
  }

// Count

  if (Get_Count != Last_Count_Value){
    if (Pulse_Right == true){
      if((Get_Count < Last_Count_Value)){
      Sum_Count = Sum_Count + ((Get_Count - Last_Count_Value)+256);
    }
    else {
      Sum_Count = Sum_Count + (Get_Count - Last_Count_Value);
    }
    }
    if (Pulse_Left == true){
      if((Get_Count > Last_Count_Value)){
      Sum_Count = Sum_Count - ((Last_Count_Value - Get_Count)+256);
    }
    else {
      Sum_Count = Sum_Count - (Last_Count_Value - Get_Count);
    }
    }
    Serial.print("Sum Count: ");
    Serial.println(Sum_Count);
  }
  Last_Count_Value = Get_Count;

// Count with Key

  if (Get_Count_Key != Last_Count_Value_Key){
    if (Pulse_Right_Key == true){
      if((Get_Count_Key < Last_Count_Value_Key)){
      SumCount_Key = SumCount_Key + ((Get_Count_Key - Last_Count_Value_Key)+256);
    }
    else {
      SumCount_Key = SumCount_Key + (Get_Count_Key - Last_Count_Value_Key);
    }
    }
    if (Pulse_Left_Key == true){
      if((Get_Count_Key > Last_Count_Value_Key)){
      SumCount_Key = SumCount_Key - ((Last_Count_Value_Key - Get_Count_Key)+256);
    }
    else {
      SumCount_Key = SumCount_Key - (Last_Count_Value_Key - Get_Count_Key);
    }
    }
    Serial.print("Sum Count Key: ");
    Serial.println(SumCount_Key);
  }
  Last_Count_Value_Key = Get_Count_Key;

}