
#include "HT66FM5440.h"
#include "mydef.h"

_bits			status =
{
	0, 0, 0, 0, 0, 0, 0, 0
};

uint16			uiHallPeriod = 0, uiHallCnt = 0;

uint8			ucDragTmr = 0;
uint16			uiDragDly = 0;

uint16			uiCommCycle = 0;
uint8			ucCommStep = 0;

uint8			state_motor = STOP;
uint8			ucCount1ms = 0;
uint16			uiDutyRamp = 0;

uint8			ucRxData = 0;

uint8			Ad_Is, Ad_Vdc, Ad_Vsp;

uint16			uiDutyFinal ;

boolean 		cc_start = 0;

DEFINE_ISR(ISR_HALL, 0x04);
void ISR_HALL(void)
{
	uint8			ucHallTimeTempL, ucHallTimeTempH;

	CLRF_HALL;
	_pt1on				= 0;
	ucHallTimeTempH 	= _ptm1dh;
	ucHallTimeTempL 	= _ptm1dl;
	uiHallPeriod		= (ucHallTimeTempH << 8) | ucHallTimeTempL;
	RST_TM1;

	//TO1 = 0;	
	if (uiHallCnt < 65534)
		uiHallCnt++;

	Commutation();

	if (state_motor >= RAMP)
	{
		TM3_Dly_Set(uiHallPeriod >> 2);
	}
	else 
	{
		if (uiCommCycle == 2)
		{
			state_motor 		= RAMP;
			_hchk_num			= NUM_RAMP;
		}
	}

	RST_TM3;

	FeedWatchDog();

}

DEFINE_ISR(ISR_OCP, 0x08);
void ISR_OCP(void)
{
	_int_pri2f			= 0;
	FeedWatchDog();

	//	bOCPFlag = 1;
}

// 20kHz, 50us

DEFINE_ISR(ISR_PWM0_2, 0x0C);
void ISR_PWM0_2(void) // 
{

	FeedWatchDog();
	_pwmpf				= 0;
	_pwmd0f 			= 0;
	_pwmd1f 			= 0;
	_pwmd2f 			= 0;
	_int_pri3f			= 0;
}


DEFINE_ISR(ISR_TM0, 0x28);
void ISR_TM0(void) // For commutation period calculation, 16b
{
	_int_pri10f 		= 0;
	_tm0af				= 0;						// clear TM0 interrup flag	  
	_pt0on				= 0;
	FeedWatchDog();
}

DEFINE_ISR(ISR_TM1, 0x2c);
void ISR_TM1(void) // For hall period calculation, 16b
{
	_int_pri11f 		= 0;
	_tm1af				= 0;						// clear TM1 interrup flag	  
	_pt1on				= 0;						// disable TM1 
	FeedWatchDog();
}

DEFINE_ISR(ISR_TM3, 0x38);
void ISR_TM3(void) // 10b
{
	_int_pri14f 		= 0;
	_tm3af				= 0;
	_pt3on				= 0;						// stop TM3

	CLRF_TM3;										// clear TM3 interrup flag	  
	CLRF_HALL;										// clear hall int flag

	if ((uiCommCycle == 0) && (ucCommStep == 1))
		INTEN_HALL = 0; // disable hall interrupt  
	else 
		INTEN_HALL = 1; // enable hall interrupt  

	_integ0 			|= 0x40;					// CMP output  
	FeedWatchDog();

}


// I2C, UART, LVD, Timebase
DEFINE_ISR(ISR_Int15, 0x3c);
void ISR_Int15(void)
{
	if (_tbf) // 16MHz/16384 = 1.024ms
	{
		if (state_motor == DRAG)
		{
			if (ucDragTmr++ >= uiDragDly)
			{
				Drag_Motor();
			}
		}

		ucCount1ms++;

		if (ucCount1ms >= 1)
		{
			ucCount1ms			= 0;
			bNmsFlag			= 1;			
		}
		_tbf				= 0;
	}

	if (_uartf)
	{
		if (_rxif)
		{
			_rxif				= 0;
			ucRxData 			= _txr_rxr;
			_uartf				= 0;
			bRxData 			= 1;
		}
	}

	_int_pri15f = 0 ;
	//_iicf				= 0;
	//_uartf				= 0;
	//_lvf				= 0;
	//_tbf				= 0;
	FeedWatchDog();
}


DEFINE_ISR(ISR_ADC, 0x20);
void ISR_ADC(void)
{
	static boolean num;

	Adc_Read_Auto();
	Uart_Tx(Ad_Is);	
	
	// soft start the current to CC mode
	if ((state_motor <= DRAG) && (uiCommCycle == 0) && (ucCommStep <= 2) && (ucDragTmr >= 2))
		cc_start = 1;
		
	if (cc_start)
	{
		//TO1 = ~TO1;
		if (Ad_Is < ILIM_CC_DRAG)
		{	
			uiDutyRamp +=1;
			if (uiDutyRamp >= uiDutyFinal)
			uiDutyRamp = uiDutyFinal;
				
		}
		else if (Ad_Is > ILIM_CC_DRAG)
		{
			if (uiDutyRamp >= PWM_DRAG_START)
				uiDutyRamp -=1;	
		}
		PWM_Duty(uiDutyRamp);
		
	}
		
	_iseocb = 0;
	_eocb	= 0;		
	_isaeocf = 0;
	_aeocf = 0;
	_int_pri7f = 0;
	FeedWatchDog();
}


