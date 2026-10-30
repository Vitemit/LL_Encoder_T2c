[Русская версия](README_RU.md)

# LL_Encoder_T2c

A low-level library for working with EC11 encoders on AVR microcontrollers.

## Disclaimer

This library is provided "AS IS", without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement.

The author makes no warranties regarding the accuracy, reliability, safety, or suitability of this library for any particular application.

This library is intended for general use and is not certified for safety-critical systems. It must not be relied upon as the sole means of control or protection in systems where a failure could result in injury, loss of life, or significant property damage.

Users are responsible for evaluating, testing, and verifying the library for their intended application, as well as for implementing appropriate safety measures.

To the maximum extent permitted by applicable law, the author shall not be liable for any claims, damages, or other liability arising from the use of this library.

## Strategy

The goal of this library is to process an encoder with the lowest possible cost in MCU time and memory. The library was originally developed for EC11-type encoders and their Type 2 board-mounted versions. However, it can also be used with ordinary encoders, within the limits of the update() call frequency.
This library has no software noise filter (for the sake of optimization); this task must be handled by the hardware, by pulling the lines to ground or to the supply using capacitors and resistors of suitable values.

## Class arguments

LL_Encoder_T2c("Channel A or S1", "Channel B or S2", "Key", "Mode for channels A and B (S1, S2)", "Mode for Key") "Object name";

In "Channel A or S1", "Channel B or S2", and "Key", specify the pin numbers as they are labeled on the Arduino Nano board. For example, if you decide to connect a Type 2 encoder to inputs D4, D5, D6, with S1 on D4, S2 on D5, and Key on D6, then the arguments are: LL_Encoder_T2c(4, 5, 6, "Mode for channels A and B (S1, S2)", "Mode for Key") "Object name";.

In "Mode for channels A and B (S1, S2)", specify the GPIO operating mode for "Channel A or S1" and "Channel B or S2".

In "Mode for Key", specify the GPIO operating mode for "Key".

In "Object name", specify any name you want for the class object.

## GPIO modes

The library supports three main GPIO configuration modes for channels A, B, and Key: LL_INPUT, LL_INPUT_PULLUP, LL_EXTERNAL_PULLUP.
When choosing pin numbers, the following restrictions must be taken into account:
Channels A (S1) and B (S2) must be in the same GPIO register: either D2–D7 (PIND) or D8–D13 (PINB). This is done to optimize the processing of the encoder signals.
All three pins — A, B, and Key — must have different numbers.
Only pins D2–D13 are allowed for connecting channels A, B, and Key. Pins D0 and D1, used for RX/TX, are not supported.
If these restrictions are violated, the compiler will produce an error. For example, if channels A and B are in different ranges, the message `Invalid pin combination. Both pins must be either 2-7 or 8-13` will appear.



### LL_INPUT

Direct reading of the GPIO without the MCU's internal pull-up. This allows the encoder outputs to be connected directly to the MCU GPIO, but in this case there is a high probability of contact bounce and noise pickup on the line, which will lead to incorrect operation. This mode also supports a hardware pull-down through a resistor and a capacitor — in this case, the encoder outputs A, B, and Key must be connected through resistors in parallel with capacitors to ground, and also in parallel to the MCU GPIO, as shown in the example diagram below:

    Supply (VCC / e.g. +5V)
         │
       ┌─┴─┐
       │   │  A, B or Key encoder output
       └─┬─┘
         │
         ├───[ Microcontroller pin (input) GPIO ]
         │
         ├───┐
         │   │
        [R] [C]  In parallel: resistor R (10 kΩ) and capacitor C (0.1 µF)
         │   │
         └───┤
             │
            ─┴─  Ground (GND)

### LL_INPUT_PULLUP

In this mode, the microcontroller's internal pull-up resistor between the GPIO and VCC is enabled, so when the contact is open, a high logic level (HIGH) is set on the input.

For correct operation, the encoder must short the GPIO input to ground (GND) when it activates. That is, the encoder's common contact must be connected to GND, not to the supply.

To suppress contact bounce, it is recommended to use a hardware filter based on a resistor and a capacitor.

### LL_EXTERNAL_PULLUP

In this mode, the microcontroller's internal pull-up resistors are disabled. To form a high logic level, an external pull-up of the input to the supply (VCC) through a resistor is used.

This circuit is widely used in ready-made EC11 Type 2 encoder modules, including the common round Chinese modules. If the module board already has external pull-up resistors, you should usually choose the LL_EXTERNAL_PULLUP mode.

Example of connecting a single encoder channel with an external pull-up and an RC filter:

    Supply (VCC / e.g. +5V)
         │
        [R]  Resistor
         │
         ├──────────────[ Microcontroller pin (input) ]
         │                         │
         │                        [C] Capacitor
         │                         │
         │                         │
         │                       ──┴── GND
         │
       ┌─┴─┐
       │   │  A, B or Key — encoder contact
       └─┬─┘
         │
        ─┴─  Ground (GND)

## Main class methods

## begin() method

This method initializes and configures the GPIO according to the selected mode and pin addresses. This method should be called in the void setup() function.


### update() method

The main method for processing the encoder signal. It works on the principle of a Mealy state machine.

Encoder state graph (Mealy State Machine)
Idle state

    State 0

        Encoder_Input == Mask_S1 -> transition to State 1 (start of clockwise rotation)

        Encoder_Input == Mask_S2 -> transition to State 10 (start of counterclockwise rotation)

Branch 1: Clockwise rotation

    State 1

        Encoder_Input == inver_Mask_Encoder -> transition to State 2

        Encoder_Input == Mask_Encoder -> reset to State 0

    State 2

        Encoder_Input == Mask_S2 -> transition to State 3

        Encoder_Input == Mask_S1 -> roll back to State 1

        Encoder_Input == Mask_Encoder -> reset to State 0

    State 3 (end of the clockwise cycle)

        Encoder_Input == Mask_Encoder && !Key_Input -> Count++, Pulse_Right = true, return to State 0

        Encoder_Input == Mask_Encoder && Key_Input -> Count_with_Key++, Pulse_Right_Key = true, return to State 0

        Encoder_Input == Mask_S1 -> roll back to State 1

        Encoder_Input == inver_Mask_Encoder -> roll back to State 2

Branch 2: Counterclockwise rotation

    State 10

        Encoder_Input == inver_Mask_Encoder -> transition to State 12

        Encoder_Input == Mask_Encoder -> reset to State 0

    State 12

        Encoder_Input == Mask_S1 -> transition to State 13

        Encoder_Input == Mask_S2 -> roll back to State 10

        Encoder_Input == Mask_Encoder -> reset to State 0

    State 13 (end of the counterclockwise cycle)

        Encoder_Input == Mask_Encoder && !Key_Input -> Count--, Pulse_Left = true, return to State 0

        Encoder_Input == Mask_Encoder && Key_Input -> Count_with_Key--, Pulse_Left_Key = true, return to State 0

        Encoder_Input == Mask_S2 -> reset to State 0

        Encoder_Input == inver_Mask_Encoder -> roll back to State 12

### GetCount() method

Returns the counter value accumulated while the "Key" pin is not at a high level, as a uint8_t from 0 to 255 with cyclic overflow.

### GetPulseRight() method

Returns the clockwise rotation state, without a high level on the "Key" pin, as a bool.
GetPulseRight() is reset to false on every update() call, and then may become true if an encoder cycle was completed during that call.

### GetPulseLeft() method

Returns the counterclockwise rotation state, without a high level on the "Key" pin, as a bool.
GetPulseLeft() is reset to false on every update() call, and then may become true if an encoder cycle was completed during that call.

### GetDirectionCode() method

Returns the rotation direction state, without a high level on the "Key" pin, as a uint8_t.
For clockwise rotation: 0b00000001.
For counterclockwise rotation: 0b00000010.

### GetKeyState() method

Returns the state of "Key" as a bool.

### GetCountKey() method

Returns the counter value as a uint8_t from 0 to 255 with cyclic overflow.
The counter of the GetCountKey() method changes if, at the moment an encoder cycle is completed, there is a high level on the "Key" pin.

### GetPulseRightKey() method

Returns the clockwise rotation state, together with a high level on the "Key" pin, as a bool.
GetPulseRightKey() is reset to false on every update() call, and then may become true if an encoder cycle was completed during that call.

### GetPulseLeftKey() method

Returns the counterclockwise rotation state, together with a high level on the "Key" pin, as a bool.
GetPulseLeftKey() is reset to false on every update() call, and then may become true if an encoder cycle was completed during that call.

### GetDirectionCodeKey() method

Returns the rotation direction state, together with a high level on the "Key" pin, as a uint8_t.
For clockwise rotation: 0b00000001.
For counterclockwise rotation: 0b00000010.

### ResetCount() method

Resets the counter of the GetCount() method to 0.

### ResetCountKey() method

Resets the counter of the GetCountKey() method to 0.

## Features and possible problems

### Incorrectly selected GPIO mode

If the GPIO mode is selected incorrectly, the Mealy state machine may count incorrectly, shifting the counting phase to the right or to the left depending on the rotation.
At the same time, it may seem that the counter works correctly. But when switching by one detent right-left-right-left (or vice versa), you will see that the count gets stuck at one value and does not increment or decrement by one.

### Compiler warnings (`if constexpr`)

When compiling for the Arduino Nano board with `arduino:avr`, you may see warnings of the following kind in the log:

`warning: 'if constexpr' only available with -std=c++1z or -std=gnu++1z`

The library code uses the `if constexpr` construct, standardized in C++17. When compiling for an Arduino Nano based on the ATmega328P with default settings, a warning may appear saying that `if constexpr` is only available in the -std=c++17 or -std=gnu++17 modes.
However, the AVR GCC version in use supports this construct as a language extension even when building with -std=gnu++11.
Verified configuration:
Board: Arduino Nano, ATmega328P.
Arduino AVR Boards: 1.8.8.
AVR GCC: 7.3.
Compilation flags: -std=gnu++11.
In this configuration, the library compiles successfully despite the warning. Compatibility with other compiler versions and build settings has not been separately verified.

### GPIO bit order specifics

When processing encoder signals, the library takes into account the bit order in the AVR microcontroller's GPIO registers. The order in which channels A (S1) and B (S2) are located in the register does not have to match the order in which they are designated in the encoder documentation.

For example, if S1 is connected to the least significant bit of the register, and S2 to the next one, then when the register is read, the signal combination will be represented in the bit order S2, S1, rather than S1, S2.

For correct operation, the library forms the masks `Mask_Encoder`, `inver_Mask_Encoder`, `Mask_S1`, and `Mask_S2` at compile time, taking the selected GPIO mode into account. In the `LL_INPUT_PULLUP` and `LL_EXTERNAL_PULLUP` modes, the processing logic differs from the `LL_INPUT` mode, because the active logic level of the signals changes.

These transformations are performed inside the library and require no additional actions from the user. As long as the pin connection restrictions are observed, the bit order does not affect the use of the class's public methods.
