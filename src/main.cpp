#include <Arduino.h>
#include <MIDI.h>

#define LED_BUILTIN 2

#define GLOBAL_KEY_COUNT 48
#define GLOBAL_KEY_LOOP(x) for(int x = 0; x < GLOBAL_KEY_COUNT; ++ x)
#define GLOBAL_KEY_MIDI_OFFSET 8 // The integer offset between the keyboard keys and te midi notes

#define KEYBOARD_MIDI_OUTPUT_PIN 18
#define KEYBOARD_MIDI_OUTPUT_CHANNEL 1

#define KEYBOARD_SHIFT_REGISTER_DATA_PIN 2
#define KEYBOARD_SHIFT_REGISTER_CLOCK_PIN 3
#define KEYBOARD_SHIFT_REGISTER_LATCH_PIN 4

#define SYNTH_SHIFT_REGISTER_DATA_PIN 5
#define SYNTH_SHIFT_REGISTER_CLOCK_PIN 6
#define SYNTH_SHIFT_REGISTER_LATCH_PIN 7

#define SYNTH_MIDI_INPUT_PIN 19
#define SYNTH_MIDI_INPUT_CHANNEL 1

bool keyboardShiftRegisterPreviousData[GLOBAL_KEY_COUNT];
bool keyboardShiftRegisterInputData[GLOBAL_KEY_COUNT];

bool synthExpectedKeyState[GLOBAL_KEY_COUNT];

midi::SerialMIDI<HardwareSerial> serialMIDI(Serial2);
midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDI((midi::SerialMIDI<HardwareSerial>&)serialMIDI);

/**
 * Dumps the data from input bit shift registers
 * into a temporary buffer
 */
void syncKeyboardShiftRegisterData() {
  // Makes sure the clock pin is low to prevent any missing rising edge issue


  GLOBAL_KEY_LOOP(keyAddress) {
    digitalWrite(KEYBOARD_SHIFT_REGISTER_CLOCK_PIN, HIGH);
    keyboardShiftRegisterInputData[keyAddress] = digitalRead(KEYBOARD_SHIFT_REGISTER_DATA_PIN);
    digitalWrite(KEYBOARD_SHIFT_REGISTER_CLOCK_PIN, LOW);
  }
}

/**
 * Handles a physical heyboard change
 * 
 * Triggers a MIDI Note output
 */
void handleKeyboardChange(int keyAddress, bool value) {
  digitalWrite(LED_BUILTIN, HIGH);

  int midiNote = keyAddress + GLOBAL_KEY_MIDI_OFFSET;

  if (value) MIDI.sendNoteOn(midiNote, 127, KEYBOARD_MIDI_OUTPUT_CHANNEL);
  else MIDI.sendNoteOff(midiNote, 0, KEYBOARD_MIDI_OUTPUT_CHANNEL);

  digitalWrite(LED_BUILTIN, LOW);

  value
    ? Serial.print("KEYBOARD NOTE CHANGE: ON ")
    : Serial.print("KEYBOARD NOTE CHANGE: OFF ");

  Serial.println(keyAddress);
}

/**
 * Handles the input data from the keyboard itself
 * 
 *  - Fetches the physical state of te keyboard
 *  - Handles any change since last iteration
 */
void loopKeyboard() {
  syncKeyboardShiftRegisterData();

  GLOBAL_KEY_LOOP(keyAddress) {
    if (
      keyboardShiftRegisterInputData[keyAddress]
      == keyboardShiftRegisterPreviousData[keyAddress]
    ) continue;

    handleKeyboardChange(
      keyAddress,
      keyboardShiftRegisterInputData[keyAddress]
    );
  }

  // The we copy te input data as previous data for next iteration
  for(int i = 0; i < GLOBAL_KEY_COUNT; i++)
  {
    keyboardShiftRegisterPreviousData[i] = keyboardShiftRegisterInputData[i];
  }
}

/**
 * Dumps the expected key state into the output
 * bit shift register
 */
void syncMidiToSynth() {
  // Hold the changes by disabling latch
  digitalWrite(SYNTH_SHIFT_REGISTER_LATCH_PIN, LOW);

  GLOBAL_KEY_LOOP(keyAddress) {
    digitalWrite(SYNTH_SHIFT_REGISTER_DATA_PIN, synthExpectedKeyState[keyAddress]);
    digitalWrite(SYNTH_SHIFT_REGISTER_CLOCK_PIN, HIGH);
    digitalWrite(SYNTH_SHIFT_REGISTER_CLOCK_PIN, LOW);
  }

  // Trigger all the changes by activating the bit shift register latch
  digitalWrite(SYNTH_SHIFT_REGISTER_LATCH_PIN, HIGH);
}

/**
 * Handles a note change from MIDI input
 */
void handleMidiNoteChange(byte midiNote, bool midiValue) {
  digitalWrite(LED_BUILTIN, HIGH);

  int keyAddress = midiNote - GLOBAL_KEY_MIDI_OFFSET;
  synthExpectedKeyState[keyAddress] = midiValue;

  syncMidiToSynth();

  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("MIDI NOTE CHANGE");
}

/**
 * Handles potential MIDI input
 * 
 * 
 */
void loopMidi() {
  if (!MIDI.read()) return;

  if (MIDI.getChannel() != SYNTH_MIDI_INPUT_CHANNEL) return;

  if (MIDI.getType() == midi::NoteOn) return handleMidiNoteChange(MIDI.getData1(), true);
  if (MIDI.getType() == midi::NoteOff) return handleMidiNoteChange(MIDI.getData1(), false);
}

/**
 * Debug method dumping a "state" into serial output
 */
void printKeyState(String prefix, bool state[]) {
  Serial.print(prefix);
  Serial.print(" [");
  GLOBAL_KEY_LOOP(keyAddress) {
    state[keyAddress]
      ? Serial.print('1')
      : Serial.print('0');
  }
  Serial.println(']');
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(KEYBOARD_SHIFT_REGISTER_CLOCK_PIN, OUTPUT);
  pinMode(KEYBOARD_SHIFT_REGISTER_DATA_PIN, INPUT);
  pinMode(KEYBOARD_SHIFT_REGISTER_LATCH_PIN, OUTPUT);
  digitalWrite(KEYBOARD_SHIFT_REGISTER_CLOCK_PIN, LOW);

  pinMode(SYNTH_SHIFT_REGISTER_CLOCK_PIN, OUTPUT);
  pinMode(SYNTH_SHIFT_REGISTER_DATA_PIN, OUTPUT);
  pinMode(SYNTH_SHIFT_REGISTER_LATCH_PIN, OUTPUT);


  GLOBAL_KEY_LOOP(keyAddress) {
    keyboardShiftRegisterPreviousData[keyAddress] = false;
    keyboardShiftRegisterInputData[keyAddress] = false;

    synthExpectedKeyState[keyAddress] = false;
  }

  MIDI.begin();

  Serial.println("All done !");
}

void loop() {
  // Handles keyboard input
  loopKeyboard();

  // Handles MIDI input
  loopMidi();
}
