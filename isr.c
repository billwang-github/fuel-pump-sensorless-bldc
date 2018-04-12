#include "HT66FM5440.h"
#include "mydef.h"

uint16 uiHallPeriod = 0, uiHallCnt = 0;

uint8 ucDragTmr = 0;
uint16 uiDragDly = 0;

uint16 uiCommCycle = 0;
uint8 ucCommStep = 0;

uint8 state_motor = STOP;
uint8 ucCount1ms = 0;
uint16 uiDutyRamp = 0;

extern _bits status;

void __attribute ((interrupt(0x28))) ISR_TM0(void) // For commutation period calculation, 16b
{
	_int_pri10f = 0;
	_tm0af = 0;	// clear TM1 interrup flag    
	_pt0on = 0;					
    WDT_RESET;
}

void __attribute ((interrupt(0x2c))) ISR_TM1(void) // For hall period calculation, 16b
{
	_tm1af = 0;	// clear TM1 interrup flag    
    _pt1on = 0; // disable TM1 
    WDT_RESET;
}

void __attribute ((interrupt(0x38))) ISR_TM3(void)
{
	
	//TO1 = 1;
	CLRF_TM3;	// clear TM3 interrup flag    
	 _pt3on = 0; // stop TM3
    CLRF_HALL; // clear hall int flag
	if ((uiCommCycle == 0) && (ucCommStep == 1))
		INTEN_HALL = 0; // enable hall interrupt  
	else
    	INTEN_HALL = 1; // enable hall interrupt  
    _integ0 |= 0x40; // CMP output  
    WDT_RESET;
    
}

// 16MHz/16384 = 1.024ms
void __attribute ((interrupt(0x3c))) ISR_TimeBase(void) 
{

	CLRF_TMB;
	
	if (state_motor == DRAG)
	{
		if (ucDragTmr++ >= uiDragDly)
		{			
			Drag_Motor();										
		}	
	}
	
	ucCount1ms ++;
    if(ucCount1ms >= 1)	
	{
	    ucCount1ms = 0;
	    bNmsFlag = 1;
	}
		
    WDT_RESET;	
}

void __attribute ((interrupt(0x04))) ISR_HALL(void)
{
	uint8  ucHallTimeTempL, ucHallTimeTempH;

	CLRF_HALL;
	_pt1on = 0;
    ucHallTimeTempH = _ptm1dh;    
   	ucHallTimeTempL = _ptm1dl;
    uiHallPeriod = (ucHallTimeTempH << 8) | ucHallTimeTempL;
    RST_TM1;

	//TO1 = 0;	
	if (uiHallCnt < 65534)
		uiHallCnt++;    
    
    Commutation();
    if (state_motor >= RAMP)
    {
    	TM3_Dly_Set(uiHallPeriod >> 3);
    }
    else
    {
    	TM3_Dly_Set(250);
    	if (uiCommCycle == 2)
    	{
    		state_motor = RAMP;
    	}    	
    }
    RST_TM3; 
    
    WDT_RESET;
    
}

void Commutation(void)
{
	TO0 = ~TO0;
	ucDragTmr = 0;
	if (state_motor >= RUN)
	{
		delay_tm0((uiHallPeriod >> 1)-15);
		//delay1(100);
	}
	else
		delay_tm0(100);	
		//delay1(100);

			
	INTEN_HALL= 0;		
	
	switch (ucCommStep)
	{
	 case 0:	//CT,AB	
		_pc     = 0b00000010;	
		_pcps0  = 0x00;
		_pcps1  = 0b00000010;	
	    _integ0 = 0b01100000;	//PHASE B DOWN = C3P DOWN
	 break;
	
	 case 1:	//CT,BB
		_pc     = 0b00001000;
	    _integ0 = 0b01000100;	//PHASE A UP = C2P UP	
	 break;
	 
	 case 2:	//AT,BB
		_pcps1  = 0b00000000;
		_pcps0  = 0b00000010;
	    _integ0 = 0b01000010;	//PHASE C DOWN = C1P DOWN
	 break;
	 
	 case 3:	//AT,CB
		_pc     = 0b00100000;
	    _integ0 = 0b01010000;	//PHASE B UP = C3P UP	
	 break;
	 
	 case 4:	//BT,CB
		_pcps0  = 0b00100000;
	    _integ0 = 0b01001000;	//PHASE A DOWN = C2P DOWN
	 break;
	 
	 case 5:	//BT,AB
		_pc     = 0b00000010;
	    _integ0 = 0b01000001;	//PHASE C UP = C1P UP	
	 break;
	 
	 default:
	 	_pc     = 0b00000000;
	 	_integ0 = 0b01000000;		
	 break;
	}
		
	if(ucCommStep++ >= 5)
	{
		ucCommStep = 0;
		if(uiCommCycle < 250)
		    uiCommCycle ++;
		    
			if (state_motor == RAMP)
			{
				//_hchk_num = 0x10;
				//if (uiCommCycle > 20)
				{
					TO1 = ~TO1;
					uiDutyRamp += 1;
					if (uiDutyRamp >= 800)
					{
						state_motor = RUN;
						uiDutyRamp = 800;						
					}
					PWM_Duty(uiDutyRamp);					
					//PWM_Duty(800);
				}
			}
			else if (state_motor == RUN)
			{
				_hchk_num = 0x01;
			}		    
	}
		
	WDT_RESET;
}

void __attribute ((interrupt(0x08))) ISR_OCP(void)
{
	_int_pri2f = 0;
	WDT_RESET;
//	bOCPFlag = 1;
}


// 20kHz, 50us
void __attribute ((interrupt(0x0C))) ISR_PWM0_2(void) // 
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
	WDT_RESET;
	_pwmpf = 0;
	_pwmd0f = 0;
	_pwmd1f = 0;
	_pwmd2f = 0;
	_int_pri3f = 0;		
}

