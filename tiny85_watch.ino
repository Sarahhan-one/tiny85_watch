#include <avr/sleep.h>
#include <avr/power.h>

const int now = 1547;                     // To set the time; eg 15:47
const int Tickspersec = 250;              // Ticks per second

volatile byte Ticks = 0;
volatile unsigned long Secs = (unsigned long)((now/100) * 60 + (now%100)) * 60;
volatile int Timeout;
volatile byte Cycle = 0;
volatile boolean DisplayOn = false;
volatile int Step;                        // For setting time

volatile int Hours = 0;                   // From 0 to 11
volatile int Minutes = 0;                 // Actual minutes (0-59)

volatile boolean TimeAdvancing = false;   // flag to track time adjustment mode
volatile byte BlinkCount = 0;             // Counter for LED blinks
volatile boolean MinuteLEDState = false;  // Current state of minute LED
volatile byte BlinkPhase = 0;            // To control blink timing

// Pin assignments
int Pins[5][5] = {{-1, 11, -1,  1,  6 },
                  {10, -1, -1,  9,  8 },
                  {-1, -1, -1, -1, -1 },
                  { 2,  3, -1, -1,  4 },
                  { 0,  7, -1,  5, -1 } };

// Display multiplexer
void DisplayNextRow() {
  Cycle++;
  byte row = Cycle & 0x03;
  if (row > 1) row++;    // Skip PB2
  byte bits = 0;
  for (int i=0; i<5; i++) {
    if (Hours == (Minutes/5)) { //when hours and minutes LEDs overlap
      if (MinuteLEDState && ((Minutes/5) == Pins[row][i])) bits |= 1<<i; //show only the minutes
    } else { 
      if (Hours == Pins[row][i]) bits |= 1<<i;
      if (MinuteLEDState && ((Minutes/5) == Pins[row][i])) bits |= 1<<i;
    }

  }
  DDRB = 1<<row | bits;
  PORTB = bits | 0x04;         // Keep PB2 high
}

// Timer/Counter0 interrupt - multiplexes display and counts ticks
ISR(TIM0_COMPA_vect) {
  Ticks++;
  if (Ticks == Tickspersec) {
    Ticks = 0;
    Secs++;
  }
  if (!DisplayOn) return;
  
  // Handle minute LED blinking for normal display
  if (DisplayOn && !TimeAdvancing && BlinkCount < (Minutes % 5)+1) {
    BlinkPhase++;
    if (BlinkPhase == Tickspersec/4) {  // ON phase
      MinuteLEDState = true;
    } else if (BlinkPhase == Tickspersec/2) {  // OFF phase
      MinuteLEDState = false;
      BlinkPhase = 0;
      BlinkCount++;
    }
  }
  
  // Handle display during time adjustment
  if (TimeAdvancing) {
    MinuteLEDState = true;  // Keep the minute LED steady during adjustment
  }
  
  DisplayNextRow();
  Timeout--;
  if (Timeout != 0) return;
  
  if (PINB & 1<<PINB2) {
    // If button is now up, turn off display and time adjustment
    DDRB = 0;                     // Blank display - all inputs
    PORTB = 0xFF;                 // All pullups on
    DisplayOn = false;
    TimeAdvancing = false;        // Reset time adjustment mode
  } else {
    // If button is still down, set time
    TimeAdvancing = true;         // Enter time adjustment mode
    Timeout = Tickspersec/2;      // Half second delay
    Secs = (unsigned long)(Secs + 300);
    // 5 minutes during adjustment
    Minutes = (((Secs + 59)/60) / 5 * 5) % 60;
    Hours = (unsigned long)((Secs+3599)/3600)%12;
  }
}

// Push button interrupt
ISR(INT0_vect) {
  // Turn on display
  Timeout = Tickspersec*4;        // Display for 4 secs
  DisplayOn = true;

  TimeAdvancing = false;          // Reset time adjustment mode
  Minutes = ((Secs + 59)/60) % 60;
  Hours = (unsigned long)((Secs+3599)/3600)%12;
  
  BlinkCount = 0;                 // Reset blink counter
  MinuteLEDState = false;         // Start with LED off
  BlinkPhase = 0;                 // Reset blink phase
}

// Setup and loop 
void setup() {
  // Wait for 5 secs before reducing clock speed
  delay(5000);

  // Slow down clock by factor of 256 to save power
  CLKPR = 1<<CLKPCE;
  CLKPR = 4<<CLKPS0;

  // Set up button
  PORTB = 0xFF;                 // All pullups on
  MCUCR = MCUCR | 2<<ISC00;     // Interrupt on falling edge
  GIMSK = 1<<INT0;              // Enable INT0 interrupt

  // Set up Timer/Counter0 to multiplex the display
  TCCR0A = 2<<WGM00;            // CTC mode; count up to OCR0A
  TCCR0B = 0<<WGM02 | 2<<CS00;  // Divide by 8 = 62500Hz
  OCR0A = 240;                  // Divide by 250 -> 250Hz
  TIMSK = TIMSK | 1<<OCIE0A;    // Enable compare match interrupt

  // Disable what we don't need to save power
  ADCSRA &= ~(1<<ADEN);         // Disable ADC
  PRR = 1<<PRUSI | 1<<PRADC | 1<<PRTIM1;  // Turn off clocks
  set_sleep_mode(SLEEP_MODE_IDLE);
}

void loop() {
  sleep_enable();
  sleep_cpu();                  // Go back to idle after each interrupt
}