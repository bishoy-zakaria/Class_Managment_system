#include <mega16.h>
#include <alcd.h>
#include <delay.h>
/************* Macros ***************/
#define SET_BIT(REG,BIT)  (REG |= (1<<BIT))
#define CLR_BIT(REG,BIT)  (REG &=~ (1<<BIT))
#define ADC_Channel 0
/************* Prototypes *************/
void Sytem_Init(void);    // initialize all system pripherals and pins
void Real_Time_Clock(void);//this function for displaying the real time in lcd and determine AM and PM
void Air_Cond(int Temp); // this function for controlling the air conditionning system
void Lighting_Control(void); // this function for controlling the lighting system and displaying the student number on LCD
unsigned int ADC_Read(unsigned char ch);
int Sensor_Val();
/************* Global variables *********/
unsigned char secs = 0, mins = 0, hours = 8, mid; //Global variables for the real time clock accessed by T2OVF interrupt and Real_Time_Clock function and initializing the clock
// assuming that the system starts @8 am
unsigned char count = 0;
char T = 100; //this variable for temperature and its initial value for safty from very high start up current
/************* Main function *************/
void main(void)
{
	Sytem_Init();

	while (1)
		{
		lcd_clear(); //clear LCD in each cycle
		Air_Cond(T);
		Real_Time_Clock();
		Lighting_Control();
		delay_ms(500); //delay for displaying data in LCD
		}
}

/******************* Functions Implimintation ************************/

void Sytem_Init(void)
{
	/******************** DIO initialization *********************/
	CLR_BIT(DDRB, 2);      // IR EXIT SENSOR SET AS INPUT
	CLR_BIT(DDRD, 3);     // IR ENTRY SENSOR SET AS INPUT
	SET_BIT(PORTB, 2);    // PULL UP RESISTANCE IS CONNECTED TO PIN B2
	SET_BIT(PORTD, 3);    // PULL UP RESISTANCE IS CONNECTED TO PIN D3
	SET_BIT(DDRB, 0); // first lighting
	SET_BIT(DDRB, 1); // second lighting
	SET_BIT(DDRB, 3); // third lighting

	SET_BIT(DDRD, 4); //ALARM RED LED

	SET_BIT(DDRB, 4); //set pins 4,5,6 and 7 for primary heater,suporter heater,primary cooler and suporter cooler
	SET_BIT(DDRB, 5);
	SET_BIT(DDRB, 6);
	SET_BIT(DDRB, 7);

	lcd_init(16); // lcd initializing pins

	/************* timer2 initialization ********************/ //connecting crystal oscillator with value 32.768 khz to TOSC1 $ TOSC2
	TCCR2 = 0b00000101; // normal mode $ prescaler 128
	SET_BIT(ASSR, 3); //(AS2 Bit in ASSR register) this enables to use timer2 as real timer counter(RTC)
	SET_BIT(TIMSK, 6); // enable overflow interrupt

	/************* External interrupt 1&2 initialization **************/
	SET_BIT(SREG, 7);     // SET GLOBAL INTERRUPT ENABLE
	SET_BIT(GICR, 5);     // SET INTERRUPT TWO
	SET_BIT(GICR, 7);     // SET INTERRUPT ONE
	CLR_BIT(MCUCR, 2);
	SET_BIT(MCUCR, 3);    // INTERRUPT ONE WORK IN FALLING EDGE
	SET_BIT(MCUCSR, 6);  // INTERRUPT TWO WORK IN RISING EDGE

	/********************* ADC initialization **********************/
	ADMUX = 0b00100000;  //AREF Reference voltage, left adjust the result to read only 8 bit
	ADCSRA = 0b10000011; //enable ADC, Set ADC prescaler equal to 8
}

void Real_Time_Clock(void)
{
	if (hours < 12)//AM time
		{
		lcd_gotoxy(3, 0);
		lcd_printf("%02d:%02d:%02d AM", hours, mins, secs);
		}
	else if (hours > 12)//PM time
		{
		mid = hours - 12;
		lcd_gotoxy(3, 0);
		lcd_printf("%02d:%02d:%02d PM", mid, mins, secs);
		}
}


void Air_Cond(int Temp)
{
	if (Temp <= 10)
		{
		CLR_BIT(PORTB, 4);
		CLR_BIT(PORTB, 5);
		SET_BIT(PORTB, 6);
		SET_BIT(PORTB, 7);
		}
	else if ((Temp > 10) && (Temp <= 20))
		{
		CLR_BIT(PORTB, 4);
		CLR_BIT(PORTB, 5);
		SET_BIT(PORTB, 6);
		CLR_BIT(PORTB, 7);
		}

	else if ((Temp > 20) && (Temp <= 30))
		{
		CLR_BIT(PORTB, 4);
		CLR_BIT(PORTB, 5);
		CLR_BIT(PORTB, 6);
		CLR_BIT(PORTB, 7);
		}

	else if ((Temp > 30) && (Temp <= 40))
		{
		SET_BIT(PORTB, 4);
		CLR_BIT(PORTB, 5);
		CLR_BIT(PORTB, 6);
		CLR_BIT(PORTB, 7);
		}

	else if ((Temp > 40) && (Temp <= 50))
		{
		SET_BIT(PORTB, 4);
		SET_BIT(PORTB, 5);
		CLR_BIT(PORTB, 6);
		CLR_BIT(PORTB, 7);
		}
	lcd_gotoxy(0, 1);
	lcd_printf("TEMP:%uC", Temp,);

}

void Lighting_Control(void)
{
	if (count == 0)
		{
		CLR_BIT(PORTB, 0);
		CLR_BIT(PORTB, 1);
		CLR_BIT(PORTB, 3);
		lcd_gotoxy(10, 1);
		lcd_printf("NUM:%u", count);
		}

	else if (count <= 20)
		{
		lcd_gotoxy(10, 1);
		lcd_printf("NUM:%u", count);

		if (count <= 10)
			{
			SET_BIT(PORTB, 0);
			CLR_BIT(PORTB, 1);
			CLR_BIT(PORTB, 3);
			}
		else if ((count > 10) && (count <= 20))
			{
			SET_BIT(PORTB, 0);
			SET_BIT(PORTB, 1);
			CLR_BIT(PORTB, 3);
			}
		}
	else if (count > 20)
		{
		SET_BIT(PORTB, 3);
		if (count < 25)
			{
			lcd_gotoxy(10, 1);
			lcd_printf("NUM:%u", count);
			CLR_BIT(PORTD, 4);
			}
		else if(count >= 25)
			{
			lcd_gotoxy(10, 1);
			lcd_printf("FULL");
			SET_BIT(PORTD, 4);
			}
		}
}

unsigned int ADC_Read(unsigned char ch)
{
	ADMUX  = (ADMUX & 0b11100000) | ch;  //Select channel -> First: clear the previously set channel, then set the new channel
	ADCSRA.6 = 1;         //Start conversion
	while(ADCSRA.6 == 1); // Wait till the conversion ends
	return ADCH;

}

int Sensor_Val()
{
	int temp = 0;
	temp = (ADC_Read(ADC_Channel) * 50) / 255; // temp ranges from 0 to 50 ^c
	return temp;
}

/**************** Interrupt service routiens ********************/

interrupt[3] void ext_int1(void)     // INTERRUPT ONE FOR IR SENSOR FOR ENTRY DOOR
{
	count++;
}

interrupt[19] void ext_int2(void)      // INTERRUPT TWO FOR IR SENSOR FOR EXIT DOOR
{
	if(count != 0)//for not get negative value
		count--;
}

interrupt [5] void T2_OV (void)
{

	if ((secs % 5) == 0 ) // every 5 sec check the temperature value
		T = Sensor_Val();

	secs++;

	if (secs == 60)
		{
		secs = 0;
		mins++;
		}
	if (mins == 60)
		{
		mins = 0;
		hours++;

		}

}