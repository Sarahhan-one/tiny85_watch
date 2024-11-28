# tiny85 Watch 
The Tiny85 Watch is an ATtiny85-based wearable timepiece that uses 12 LEDs to display time in an analog-inspired style. 
This project builds upon [David's Tiny Time Watch](http://www.technoblogy.com/show?1JH5), adapting with additional features. For details on compiling the program, calibrating the internal oscillator, and more technical insights, please refer to David's original project.

![watch_3](https://github.com/user-attachments/assets/d81b954f-a138-4d0a-a2ed-3a7b6c50ebba)

**[Check out the video in action!](https://youtu.be/fZJ_hKkSAIo)**

## Overview
The Tiny85 Watch operates with a single button and offers a unique way of displaying time:

* **Time Display**: When the button is pressed, the watch displays the current time for four seconds.
  The hour is shown first, with the corresponding hour LED lighting up steadily.
  The minute is then displayed, with the minute LED blinking a number of times corresponding to to the minute.
  
  _For instance: At 4:48, the LED at position 4 will light up steadily, followed by 4 blinks of the LED at position 9._
  
  If the minute and hour markers overlap _(e.g., 6:30 or 7:35)_, only the minute LED will blink.
  
* **Time Adjustment**: Pressing and holding the button for more than four seconds allows the time to be adjusted.
  
The Tiny85 Watch is powered by a CR2032 battery cell and uses the ATtiny85's built-in oscillator, maintaining accuracy within a few minutes over 24 hours <sup> (based on David's project)</sup>.

The "2400 Joules" is a playful reference to vintage watches printing "21/etc Jewels" for indicating higher end watches. This inspiration comes from the [design of a specific wristwatch](https://www.reddit.com/r/electronics/comments/t00i9o/ive_made_an_attiny85based_wristwatch_more_in/?share_id=4Ajg3Bm2FpcpxPIzXsNSp&utm_content=1&utm_medium=ios_app&utm_name=ioscss&utm_source=share&utm_term=1).

## Schematic & PCB 
The Tiny85 Watch is built using all use of the ATtiny85’s five I/O lines:

* LED Control: 12 LEDs are driven via charlieplexing using four I/O lines.
* Button Input: The fifth I/O line detects button presses.

![sche](https://github.com/user-attachments/assets/32ff3efd-d1da-478b-bc95-e1742cf023dc)

The pcb directory includes EasyEDA project files that outline the PCB design.

* The watch is built on a 0.6mm-thick PCB for comfortable wear. 
* Surface-Mount Device (SMD) components were used, and the assembly was hand-soldered. 

## Program Enhancements
The Tiny85 Watch program builds upon David's original code, customizing the display method:

* Time Representation and Handling
  
Introduced minute-level granularity to provide a precise representation of time.
```c++
volatile int Minutes = 0;                 // Actual minutes (0-59)
volatile int Hours = 0;                   // From 0 to 11
```
```c++
Secs = (unsigned long)(Secs + 300);      // Increment by 5 minutes
Minutes = (((Secs + 59)/60) / 5 * 5) % 60;  // Adjust minutes (round to nearest 5)
Hours = (unsigned long)((Secs+3599)/3600)%12;
```
* Minute LED Blinking
  
The minute LED blinks a number of times corresponding to the remainder of minutes divided by 5. This provides a clear and intuitive way to read the exact minute.

```c++
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
```

* Handling Overlapping LEDs

Prioritizes the minute LED during overlap.

```c++
if (Hours == (Minutes/5)) { 
    if (MinuteLEDState && ((Minutes/5) == Pins[row][i])) bits |= 1<<i; // Show only minutes
} else { 
    if (Hours == Pins[row][i]) bits |= 1<<i;
    if (MinuteLEDState && ((Minutes/5) == Pins[row][i])) bits |= 1<<i;
}
```

* Display Multiplexing
  
Ensures minute LEDs are prioritized during overlaps. Implements blinking behavior for minute LEDs.

```c++
if (Hours == (Minutes/5)) { 
    if (MinuteLEDState && ((Minutes/5) == Pins[row][i])) bits |= 1<<i; // Show only minutes
} else { 
    if (Hours == Pins[row][i]) bits |= 1<<i;
    if (MinuteLEDState && ((Minutes/5) == Pins[row][i])) bits |= 1<<i;
}
```



 
