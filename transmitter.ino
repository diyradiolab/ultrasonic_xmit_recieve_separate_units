#include <avr/io.h>
#include <util\atomic.h>

//transmitter: 
//TCCR2A |= (1 <<COM2A0); // turn on bits in compare match to toggle.  this line is commented out to prevent timer2 output
//The transmitter switches are on/off and reset


//typedef unsigned char  u8;
//typedef signed short  s16;
#define KEY_PIN PIND
#define KEY_PORT PORTD
#define KEY_DDR DDRD
#define KEY0    0
#define KEY1    1
#define KEY2    2
#define KEY3    3
#define KEY4    4
#define KEY5    5
#define KEY6    6
#define KEY7    7
#define XTAL    16e6  


uint8_t key_state;        // debounced and inverted key state:
// bit = 1: key pressed
uint8_t key_press;        // key press detect

ISR( TIMER0_COMPA_vect )    // every 10ms
{
  static uint8_t ct0 = 0xFF, ct1 = 0xFF;  // 8 * 2bit counters
  uint8_t i;

  i = ~KEY_PIN;       // read keys (low active)
  i ^= key_state;     // key changed ?
  ct0 = ~( ct0 & i );     // reset or count ct0
  ct1 = ct0 ^ (ct1 & i);    // reset or count ct1
  i &= ct0 & ct1;     // count until roll over ?
  key_state ^= i;     // then toggle debounced state
  key_press |= key_state & i;   // 0->1: key press detect
}


u8 get_key_press( u8 key_mask )
{
  ATOMIC_BLOCK(ATOMIC_FORCEON){   // read and clear atomic !
    key_mask &= key_press;    // read key(s)
    key_press ^= key_mask;    // clear key(s)
  }
  return key_mask;
}

  void set_timers(int timer1, int timer2){

    //timer 1 register configuration for CTC mode, prescaler, and compare match
    TCCR1B |= (1 << CS10); // set prescaler to 0
    TCCR1B |= (1 << WGM12); //put timer 0 in ctc mode a mode where the top is defined in register OCR0A
    if(timer1 == -1){
      TCCR1A &= (0 <<COM1A0); // turn off bits in compare match to toggle.
    }
    else{
      TCCR1A |= (1 <<COM1A0); // turn on bits in compare match to toggle.
    }
    OCR1A = timer1; //set value to trigger compare match. this determines the frequency on pin PORTB1
    
    //timer 2 register configuration for CTC mode, prescaler, and compare match
    TCCR2B |= (1 << CS21); // set prescaler to 8
    TCCR2A |= (1 << WGM21); //put timer 0 in ctc mode a mode where the top is defined in register OCR0A
    //TCCR2A |= (1 <<COM2A0); // turn on bits in compare match to toggle. 
    OCR2A = timer2; //set value to trigger compare match. this determines the frequency on pin PORTB1
  }

void set_serial(){
    Serial.begin(9600);
    Serial.println("HI");
}

int main() {

  set_serial();
  
  TCCR0A = 1<<WGM01;      // T0 Mode 2: CTC
  TCCR0B = 1<<CS02^1<<CS00;   // divide by 1024
  OCR0A = XTAL / 1024.0 * 10e-3 -1; // 10ms
  TIMSK0 = 1<<OCIE0A;     // enable T0 interrupt

  KEY_DDR = 0;        // input
  KEY_PORT = 0xFF;      // pullups on

  key_state = ~KEY_PIN;     // no action on keypress during reset
  sei();
  
  DDRB |= (1 << DDB1); // Port B1 as output. arduino pin 9
  DDRB |= (1 << DDB3); // port B3 as output. arduino pin 11.

  //set mode which determines switch flow  
  int mode = 0;

  while (1) {

      //f_set1
    
      if( get_key_press( 1<<KEY0) ){

        switch(mode){
          case 0:
            set_timers(200,25);
            mode = 1;
            break;   
            
       
          case 1:
            set_timers(205,25);
            mode = 2;
            break;  
        

          case 2:  //turns off oscillator      
            set_timers(-1,25);
            mode = 0;
            break;

         }
      }

    /*
    //f_set2
    if( get_key_press( 1<<KEY1 )){
        set_timers(204,25);
    */
    
      
  }
 }
 
