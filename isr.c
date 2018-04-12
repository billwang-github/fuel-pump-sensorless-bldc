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

void 	Drag_Motor(void);
void 	Commutation(void);
void 	TM3_Dly_Set(uint16 dly);
void 	delay_tm0(uint16 dly);
void 	PWM_Duty(uint16 duty);

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



