
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

DEFINE_ISR(ISR_TM0, 0x28);
ISR_TM0(void) // For commutation period calculation, 16b
{
	_int_pri10f 		= 0;
	_tm0af				= 0;						// clear TM0 interrup flag	  
	_pt0on				= 0;
	FeedWatchDog();
}

DEFINE_ISR(ISR_TM1, 0x2c);
ISR_TM1(void) // For hall period calculation, 16b
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


DEFINE_ISR(ISR_HALL, 0x04);
ISR_HALL(void)
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
ISR_OCP(void)
{
	_int_pri2f			= 0;
	FeedWatchDog();

	//	bOCPFlag = 1;
}

// 20kHz, 50us

DEFINE_ISR(ISR_PWM0_2, 0x0C);
ISR_PWM0_2(void) // 
{
	//	uint16 time_delay;

	/*
	bHallU_new = (_mcd & 0x02) == 0x02 ? 1: 0;
	bHallV_new = (_mcd & 0x04) == 0x04 ? 1: 0;
	bHallW_new = (_mcd & 0x01) == 0x01 ? 1: 0;	
		
	// fall edge detector , delayed some time after commutation
	if ((uiCommTmr >= 5))// && (uiCommCycle > 0))
	{
		switch (ucCommStep)
		{
			case 0:
				if ((bHallW_old == 0) && (bHallW_new == 1))
					bHallW_zcr = 1;					
				break;
			case 1:
				if ((bHallV_old == 1) && (bHallV_new == 0))
					bHallV_zcf = 1;
				break;
			case 2:
				if ((bHallU_old == 0) && (bHallU_new == 1))
					bHallU_zcr = 1;
				break;								
			case 3:
				if ((bHallW_old == 1) && (bHallW_new == 0))
					bHallW_zcf = 1;
				break;			
			case 4:
				if ((bHallV_old == 0) && (bHallV_new == 1))
					bHallV_zcr = 1;
				break;					
			case 5:
				if ((bHallU_old == 1) && (bHallU_new == 0))
					bHallU_zcf = 1;
				break;	
			default:
				break;			
		}
	}	

	if (hall_bits.byte != 0) // if zc, save current hall time to hall period
	{
		TO1 = 1;
		uiHallPeriod = uiHallTmr;
		uiHallTmr = 0;
		bHallU_zc = 1;
		hall_bits.byte = 0;		
		if (uiHallCnt < 1000)
			uiHallCnt++;		

				
	}

	// save old comparator value
	bHallU_old = bHallU_new;
	bHallV_old = bHallV_new;
	bHallW_old = bHallW_new;

	if (uiHallTmr == (uiHallPeriod >> 1))			
		TO1 = 0;
						
	if (uiCommTmr < 65534) //  reset at Commutation()
		uiCommTmr++;	

	if (ucDragTmr < 65534) //
		ucDragTmr++;	
			
	if (uiHallTmr < 65534) // 1000ms
		uiHallTmr++;		
		
	if (uiHallDlyTmr < 65534) // 
		uiHallDlyTmr++;
			
	if (uiDutyRampTmr < 65534) // 500ms, reset at ZC
		uiDutyRampTmr++;
		


	 

	// clear interrupt flags



	if (ucDragTmr < 65534) //
		ucDragTmr++;
	*/
	FeedWatchDog();
	_pwmpf				= 0;
	_pwmd0f 			= 0;
	_pwmd1f 			= 0;
	_pwmd2f 			= 0;
	_int_pri3f			= 0;
}


