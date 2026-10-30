enum EncoderMode : uint8_t {
    LL_INPUT           = 0,
    LL_INPUT_PULLUP    = 2,
    LL_EXTERNAL_PULLUP = 3
};

#define LL_Encoder_T2c(A_ch, B_ch, Key, mode, mode_Key) _LL_Encoder_T2c<A_ch, B_ch, Key, mode, mode_Key>
template <uint8_t A_ch, uint8_t B_ch, uint8_t Key, EncoderMode mode, EncoderMode mode_Key>
class _LL_Encoder_T2c {
  
  private:

    static_assert(((A_ch != B_ch) && (A_ch !=Key) && (B_ch !=Key) && ((Key >= 2) && (Key <= 13))), "Pins must be different");
    static_assert(((A_ch >= 2 && A_ch <= 7  && B_ch >= 2 && B_ch <= 7) || (A_ch >= 8 && A_ch <= 13 && B_ch >= 8 && B_ch <= 13)), "Invalid pin combination. Both pins must be either 2-7 or 8-13");
    static constexpr uint8_t PINx_ADDR = (A_ch >= 2 && A_ch <= 7 && B_ch >= 2 && B_ch <= 7) ? 0x09 : 0x03; // Select the PINx register address for Port D or Port B
    static constexpr uint8_t raw_Mask_S1 = (A_ch >= 2 && A_ch <= 7) ? (0b00000001 << A_ch) : (0b00000001 << (A_ch - 8)); // Define the mask for reading the S1 (A) input state
    static constexpr uint8_t raw_Mask_S2 = (B_ch >= 2 && B_ch <= 7) ? (0b00000001 << B_ch) : (0b00000001 << (B_ch - 8)); // Define the mask for reading the S2 (B) input state
    static constexpr uint8_t Mask_Key = (Key >= 2 && Key <= 7) ? (0b00000001 << Key) : (0b00000001 << (Key - 8)); // Key input mask
    static constexpr uint8_t raw_Mask_Encoder = ((A_ch >= 2 && A_ch <= 7) ? (0b00000001 << A_ch) : (0b00000001 << (A_ch - 8))) | ((B_ch >= 2 && B_ch <= 7) ? (0b00000001 << B_ch) : (0b00000001 << (B_ch - 8))); // Combined S1 and S2 mask
    static constexpr uint8_t Mask_S1 = (mode == LL_INPUT) ? raw_Mask_S2 : raw_Mask_S1; // Calculate Mask_S1 for the Mealy state machine according to the selected mode
    static constexpr uint8_t Mask_S2 = (mode == LL_INPUT) ? raw_Mask_S1 : raw_Mask_S2; // Calculate Mask_S2 for the Mealy state machine according to the selected mode
    static constexpr uint8_t Mask_Encoder = (mode == LL_INPUT) ? 0b00000000 : raw_Mask_Encoder; // Calculate Mask_Encoder for the Mealy state machine according to the selected mode
    static constexpr uint8_t inver_Mask_Encoder = (mode == LL_INPUT) ? raw_Mask_Encoder : 0b00000000; // Inverted encoder mask
    
    // Variables
    uint8_t Encoder_Input = 0; // Current state of the S1 and S2 (A, B) input pins
    uint8_t Key_Input = false;  // Current state of the Key input
    uint8_t Last_Encoder_Input = 0; // Previous state of the S1 and S2 (A, B) input pins
    uint8_t State = 0; // Mealy state machine state
    uint8_t Count = 0; // Current counter value when Key is inactive
    uint8_t Count_with_Key = 0; // Current counter value when Key is active
    bool Pulse_Right = false; // Clockwise rotation event flag when Key is inactive
    bool Pulse_Left = false; // Counterclockwise rotation event flag when Key is inactive
    bool Pulse_Right_Key = false; // Clockwise rotation event flag when Key is active
    bool Pulse_Left_Key = false; // Counterclockwise rotation event flag when Key is active





  public:

// The begin() method initializes the pins.
// Compile-time if constexpr conditions select the port and INPUT / INPUT_PULLUP mode.
  void begin() {
  if constexpr (PINx_ADDR == 0x03) { // Port B, pins 8–13
    if constexpr ((mode == EncoderMode::LL_INPUT) || (mode == EncoderMode::LL_EXTERNAL_PULLUP)){ // INPUT mode
      DDRB &= ~raw_Mask_Encoder; // DDRx bit = 0 -> input
      PORTB &= ~raw_Mask_Encoder; // Disable internal pull-up resistors
    }
    else if constexpr (mode == EncoderMode::LL_INPUT_PULLUP) { 
      DDRB &= ~raw_Mask_Encoder;
      PORTB |= raw_Mask_Encoder;
    }
  }
  else { // Port D 0x09, pins 2–7 
    if constexpr ((mode == EncoderMode::LL_INPUT) || (mode == EncoderMode::LL_EXTERNAL_PULLUP)){
      DDRD &= ~raw_Mask_Encoder;
      PORTD &= ~raw_Mask_Encoder;
    }
    else if constexpr (mode == EncoderMode::LL_INPUT_PULLUP) {
      DDRD &= ~raw_Mask_Encoder;
      PORTD |= raw_Mask_Encoder;
    }
  }

// Key
  if constexpr (Key >= 2 && Key <= 7){ // Port D, pins 2–7
    if constexpr ((mode_Key == EncoderMode::LL_INPUT) || (mode_Key == EncoderMode::LL_EXTERNAL_PULLUP)){
      DDRD &= ~Mask_Key;
      PORTD &= ~Mask_Key;
    }
    else if constexpr (mode_Key == EncoderMode::LL_INPUT_PULLUP) {  
      DDRD &= ~Mask_Key;
      PORTD |= Mask_Key;
    }
  }
  else 
  if constexpr ((mode_Key == EncoderMode::LL_INPUT) || (mode_Key == EncoderMode::LL_EXTERNAL_PULLUP)){ // Port B, pins 8–13
      DDRB &= ~Mask_Key;
      PORTB &= ~Mask_Key;
    }
    else if constexpr (mode_Key == EncoderMode::LL_INPUT_PULLUP) {
      DDRB &= ~Mask_Key;
      PORTB |= Mask_Key;
    }
    
}


// The update() method polls the inputs and processes the Mealy state machine.

  void inline update() {

// Read the S1 and S2 input states
  if constexpr (PINx_ADDR == 0x03) {
      Encoder_Input = (PINB & raw_Mask_Encoder);  
  }
  else { //0x09
      Encoder_Input = (PIND & raw_Mask_Encoder); 
  }
  // Key input
  if constexpr (Key >= 2 && Key <= 7){
    if constexpr (mode_Key == EncoderMode::LL_INPUT){
      Key_Input = (PIND & Mask_Key); // Determine which input signal level is interpreted as logical true
    }
    else if constexpr ((mode_Key == EncoderMode::LL_INPUT_PULLUP) || (mode_Key == EncoderMode::LL_EXTERNAL_PULLUP)) {
      Key_Input = (~PIND & Mask_Key); 
    }
  }
  else 
  if constexpr (mode_Key == EncoderMode::LL_INPUT){
      Key_Input = (PINB & Mask_Key); 
    }
    else if constexpr ((mode_Key == EncoderMode::LL_INPUT_PULLUP) || (mode_Key == EncoderMode::LL_EXTERNAL_PULLUP)) {
      Key_Input = (~PINB & Mask_Key); 
    }
  

    /*
    For a two-channel Type 2 EC11 encoder using channels S1 and S2 (A and B),
    the following signal sequences occur during clockwise rotation in
    LL_EXTERNAL_PULLUP and LL_INPUT_PULLUP modes:
    S1  S2                            S2  S1  
    1   1                              1   1 Mask_Encoder = raw_Mask_Encoder (Phase 0)
    1   0   bit order in the register  0   1 Mask_S1 = raw_Mask_S1           (Phase 1)
    0   0   ----------------------->   0   0 inver_Mask_Encoder = 0b00000000 (Phase 2)
    0   1                              1   0 Mask_S2 = raw_Mask_S2           (Phase 3)
    1   1                              1   1 Mask_Encoder = raw_Mask_Encoder (Phase 0)
    During counterclockwise rotation:
    S1  S2                            S2  S1  
    1   1                              1   1 Mask_Encoder = raw_Mask_Encoder (Phase 0)
    0   1   bit order in the register  1   0 Mask_S2 = raw_Mask_S2           (Phase 1)
    0   0   ----------------------->   0   0 inver_Mask_Encoder = 0b00000000 (Phase 2)
    1   0                              0   1 Mask_S1 = raw_Mask_S1           (Phase 3)
    1   1                              1   1 Mask_Encoder = raw_Mask_Encoder (Phase 0)
    
    For LL_INPUT mode, the sequence during clockwise rotation is:

    S1  S2                            S2  S1  
    0   0                              0   0 Mask_Encoder = 0b00000000             (Phase 0)
    0   1   bit order in the register  1   0 Mask_S2 = raw_Mask_S1                 (Phase 1)
    1   1   ----------------------->   1   1 inver_Mask_Encoder = raw_Mask_Encoder (Phase 2)
    1   0                              0   1 Mask_S1 = raw_Mask_S2                 (Phase 3)
    0   0                              0   0 Mask_Encoder = 0b00000000             (Phase 0)
    During counterclockwise rotation:
    S1  S2                            S2  S1  
    0   0                              0   0 Mask_Encoder = 0b00000000             (Phase 0)
    1   0   bit order in the register  0   1 Mask_S1 = raw_Mask_S2                 (Phase 1)
    1   1   ----------------------->   1   1 inver_Mask_Encoder = raw_Mask_Encoder (Phase 2)
    0   1                              1   0 Mask_S2 = raw_Mask_S1                 (Phase 3)
    0   0                              0   0 Mask_Encoder = 0b00000000             (Phase 0)

    According to the tables above, Mask_Encoder, inver_Mask_Encoder,
    Mask_S1, and Mask_S2 can change at compile time depending on the
    selected GPIO operating mode. 
    */

    Pulse_Right = false;
    Pulse_Left = false;
    Pulse_Right_Key = false;
    Pulse_Left_Key = false;

    // Mealy state machine 
    if ((Encoder_Input)!=Last_Encoder_Input){
  switch (State) {
  case 0:
    if (Encoder_Input == Mask_S1) State = 1; // Phase 1 - Start of clockwise rotation: S2 = 1, S1 = 0 in LL_INPUT mode, or S2 = 0, S1 = 1 in other modes
    else if (Encoder_Input == Mask_S2) State = 10; // Phase 1 - Start of counterclockwise rotation: S2 = 0, S1 = 1 in LL_INPUT mode, or S2 = 1, S1 = 0 in other modes
    break;
  case 1:
    if (Encoder_Input == inver_Mask_Encoder) State = 2; // Phase 2 - S2 = 1, S1 = 1 in LL_INPUT mode, or S2 = 0, S1 = 0 in other modes; current state is State = 1
    else if (Encoder_Input == Mask_Encoder) State = 0; // Return to State 0 when the GPIO state matches Phase 0: S2 = 0, S1 = 0 in LL_INPUT mode, or S2 = 1, S1 = 1 in other modes; current state is State = 1
    break; 
  case 2:
    if (Encoder_Input == Mask_S2) State = 3; // Phase 3 - S2 = 0, S1 = 1 in LL_INPUT mode, or S2 = 1, S1 = 0 in other modes
    else if (Encoder_Input == Mask_S1) State = 1; // Return to State 1 when the GPIO state matches S1 = 0 in LL_INPUT mode, or S1 = 1 in other modes; current state is State = 2
    else if (Encoder_Input == Mask_Encoder) State = 0; // Return to State 0 when the GPIO state matches S2 = 0, S1 = 0 in LL_INPUT mode, or S2 = 1, S1 = 1 in other modes; current state is State = 2
    break;  
  case 3:
    if (Encoder_Input == Mask_Encoder && !Key_Input) { // Increment the counter when State = 3 and Key_Input is false; a complete cycle ends with the transition from Phase 3 to Phase 0: S2 = 0, S1 = 0 in LL_INPUT mode, or S2 = 1, S1 = 1 in other modes   
        Count++;
        Pulse_Right = true;
        State = 0;
    }
    else if (Encoder_Input == Mask_Encoder && Key_Input){ // Increment the counter when State = 3 and Key_Input is true; a complete cycle ends with the transition from Phase 3 to Phase 0: S2 = 0, S1 = 0 in LL_INPUT mode, or S2 = 1, S1 = 1 in other modes
        Count_with_Key ++;
        Pulse_Right_Key = true;
        State = 0;
    }
    else if (Encoder_Input == Mask_S1){State = 1;} // Return to State 1 when the GPIO state matches S1 = 1 in LL_INPUT mode, or S1 = 0 in other modes; current state is State = 3
    else if (Encoder_Input == inver_Mask_Encoder){State = 2;} // Return to State 2 when the GPIO state matches S2 = 1, S1 = 1 in LL_INPUT mode, or S2 = 0, S1 = 0 in other modes; current state is State = 3
    break;



  case 10:
    if (Encoder_Input == inver_Mask_Encoder) State = 12; // Phase 2 - S2 = 1, S1 = 1 in LL_INPUT mode, or S2 = 0, S1 = 0 in other modes; current state is State = 10
    else if (Encoder_Input == Mask_Encoder) State = 0; // Return to State 0 when the GPIO state matches Phase 0: S2 = 0, S1 = 0 in LL_INPUT mode, or S2 = 1, S1 = 1 in other modes; current state is State = 10
    break;
  case 12: 
    if (Encoder_Input == Mask_S1) State = 13; // Phase 3 - S2 = 1, S1 = 0 in LL_INPUT mode, or S2 = 0, S1 = 1 in other modes
    else if (Encoder_Input == Mask_S2) State = 10; // Return to State 10 when the GPIO state matches S2 = 1 in LL_INPUT mode, or S2 = 0 in other modes; current state is State = 12
    else if (Encoder_Input == Mask_Encoder) State = 0; // Return to State 0 when the GPIO state matches S2 = 0, S1 = 0 in LL_INPUT mode, or S2 = 1, S1 = 1 in other modes; current state is State = 12
    break;
  case 13:
    if (Encoder_Input == Mask_Encoder && !Key_Input) { // Decrement the counter when State = 13 and Key_Input is false; a complete cycle ends with the transition from Phase 3 to Phase 0: S2 = 0, S1 = 0 in LL_INPUT mode, or S2 = 1, S1 = 1 in other modes
        Count--;
        Pulse_Left = true;
        State = 0;
      }
      else if (Encoder_Input == Mask_Encoder && Key_Input){ // Decrement the counter when State = 13 and Key_Input is true; a complete cycle ends with the transition from Phase 3 to Phase 0: S2 = 0, S1 = 0 in LL_INPUT mode, or S2 = 1, S1 = 1 in other modes
        Count_with_Key --;
        Pulse_Left_Key = true;
        State = 0;
      }
    else if (Encoder_Input == Mask_S2){ State = 0;} // Return to State 0 when the GPIO state matches S2 = 1 in LL_INPUT mode, or S2 = 0 in other modes; current state is State = 13
    else if (Encoder_Input == inver_Mask_Encoder){ State = 12;} // Return to State 12 when the GPIO state matches S2 = 1, S1 = 1 in LL_INPUT mode, or S2 = 0, S1 = 0 in other modes; current state is State = 13
    break;

    }
    Last_Encoder_Input = Encoder_Input;
  }
  }
  uint8_t GetCount(){ // Return the counter value
    return Count;
  }
  bool GetPulseRight(){ // Return the clockwise rotation event flag
    return Pulse_Right;
  }
  bool GetPulseLeft(){ // Return the counterclockwise rotation event flag
    return Pulse_Left;
  }
  uint8_t GetDirectionCode(){ // Return the direction code: bit 1 = counterclockwise, bit 0 = clockwise
    return (Pulse_Left << 1)|Pulse_Right;
  }
  bool GetKeyState(){ // Return the current state of the Key input
    return Key_Input;
  }
  uint8_t GetCountKey(){ // Return the counter value when Key is active
    return Count_with_Key;
  }
  bool GetPulseRightKey(){ // Return the clockwise rotation event flag when Key is active
    return Pulse_Right_Key;
  }
  bool GetPulseLeftKey(){ // Return the counterclockwise rotation event flag when Key is active
    return Pulse_Left_Key;
  }
  uint8_t GetDirectionCodeKey(){ // Return the direction code when Key is active: bit 1 = counterclockwise, bit 0 = clockwise
    return (Pulse_Left_Key << 1)|Pulse_Right_Key;
  }
  void ResetCount(){ // Reset the counter when Key is inactive
    Count = 0;
  }
  void ResetCountKey(){ // Reset the counter when Key is active
    Count_with_Key = 0;
  }
};