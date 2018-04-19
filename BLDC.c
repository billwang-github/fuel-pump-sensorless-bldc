
#include "HT66FM5440.h"
#include "mydef.h"

//static volatile	_bits bit_var_0		__attribute__	((at(0x80)));
//static volatile	__hall_type	__attribute__	((at(0x81)));
//static volatile	_bits bit_var_2		__attribute__	((at(0x82)));
extern uint16	uiHallPeriod, uiHallCnt;
extern uint8	ucCommStep, uiCommCycle;
extern uint8	ucDragTmr;
extern uint16	uiDragDly;
extern uint8	ucCount1ms;
extern uint16	uiDutyRamp;
extern uint8	state_motor;
extern _bits	status;
extern uint8	ucRxData;
extern uint8	Ad_Is, Ad_Vdc, Ad_Vsp;
extern uint16			uiDutyFinal  ; //period 800
extern boolean cc_start;

uint16 dly_comm , dly_comm_final;
uint8			i;

const uint8 	cw_pattern[] =
{
	1, 3, 2, 6, 4, 5
};


const uint8 	ccw_pattern[] =
{
	1, 5, 4, 6, 2, 3
};


const uint16	drag_time[] =
{
	20, 20, 19, 17, 14, 11, 9
};


uint8			ucDragStep = 0;

/*
static uint8 hall_step = 0;
static uint8 cnt_hall = 0;
static uint16 period_hall = 0;
static uint16 period_comm = 0;
static uint16 cnt_comm = 0;

static uint16 uiHallTmr = 0;
*/
void main()
{
	GCC_NOP();
	GCC_NOP();
	GCC_NOP();
	DI();

	// watch dog timer clock
	_ws0				= 1;
	_ws1				= 1;
	_ws2				= 1;

	//Init_System();		
	Init_IO();
	Init_Comp();	
	Init_TM0();
	Init_TM1();
	Init_TM3();

	Init_Vars();
	Init_UART();
	Init_OCP(ILIM_DAC);	
	OCP_EN;
	Init_ADC(ADC_10b);		
	Init_PWM();
	PWM_SET(0, uiDutyRamp);
	PWM_ON;
	Init_TimeBase();
	Init_Int();
	EI();

	while (1)
	{	
		if (bNmsFlag)
		{
			bNmsFlag			= 0;			
			/*
			if (bRxData)
			{
				Uart_Tx(ucRxData);
				Uart_Tx(' ');
				bRxData = 0;
			}*/
		}		
		FeedWatchDog();
	}
}


void Drag_Motor(void)
{
	//PWM_Duty(PWM_DRAG);	
	Commutation();
	TM3_Dly_Set(250);
	RST_TM3;

	if (ucDragStep < 255)
		ucDragStep++;

	if (ucDragStep < 7)
		uiDragDly = drag_time[ucDragStep];
}


void Commutation(void)
{
		 
	TO0 				= ~TO0;
	INTEN_HALL			= 0;
	ucDragTmr			= 0;	

	
	if (state_motor >= RUN)
	{	/*	
		dly_comm_final = (uiHallPeriod >> 1);
		if (dly_comm_final > 30)
			dly_comm_final -= 20;				
		if (dly_comm > dly_comm_final)
			dly_comm--;
		else if (dly_comm < dly_comm_final)
			dly_comm++;	*/
		dly_comm = 30;
	}
	else
		dly_comm = 5;
	
#ifdef MSKMS_HW
	TM0_Dly_Set(dly_comm);
#else	
	delay_tm0(dly_comm);
#endif

#ifdef MSKMS_HW

	switch (ucCommStep)
	{
		case 0: //CT,AB, PHASE B DOWN = C3P DOWN
			_hdcd = 4;
			_integ0 = 0b01100000; //
			break;

		case 1: //CT,BB, PHASE A UP = C2P UP	
			_hdcd = 5;
			_integ0 = 0b01000100; //
			break;

		case 2: //AT,BB, PHASE C DOWN = C1P DOWN
			_hdcd = 1;
			_integ0 = 0b01000010; //
			break;

		case 3: //AT,CB, PHASE B UP = C3P UP
			_hdcd = 3;
			_integ0 = 0b01010000; //	
			break;

		case 4: //BT,CB, PHASE A DOWN = C2P DOWN
			_hdcd = 2;
			_integ0 = 0b01001000; //
			break;

		case 5: //BT,AB, PHASE C UP = C1P UP	
			_hdcd = 6;
			_integ0 = 0b01000001; //
			break;

		default:
			_hdcd = 0;
			_integ0 = 0b01000000;
			break;
	}

#else

	switch (ucCommStep)
	{
		case 0: //CT,AB	
			_pc = 0b00000010;
			_pcps0 = 0x00;
			_pcps1 = 0b00000010;
			_integ0 = 0b01100000; //PHASE B DOWN = C3P DOWN
			break;

		case 1: //CT,BB
			_pc = 0b00001000;
			_integ0 = 0b01000100; //PHASE A UP = C2P UP	
			break;

		case 2: //AT,BB
			_pcps1 = 0b00000000;
			_pcps0 = 0b00000010;
			_integ0 = 0b01000010; //PHASE C DOWN = C1P DOWN
			break;

		case 3: //AT,CB
			_pc = 0b00100000;
			_integ0 = 0b01010000; //PHASE B UP = C3P UP	
			break;

		case 4: //BT,CB
			_pcps0 = 0b00100000;
			_integ0 = 0b01001000; //PHASE A DOWN = C2P DOWN
			break;

		case 5: //BT,AB
			_pc = 0b00000010;
			_integ0 = 0b01000001; //PHASE C UP = C1P UP	
			break;

		default:
			_pc = 0b00000000;
			_integ0 = 0b01000000;
			break;
	}

#endif

	if (ucCommStep++ >= 5)
	{		
		ucCommStep			= 0;				
		if (uiCommCycle < 250)
			uiCommCycle++;	
		Duty_Rampup();			
	}

	FeedWatchDog();
}


void Init_System(void)
{
	_smod				= 0b00000001;

	for (_mp0 = 0x80; _mp0 != 0x00; _mp0 ++)
		_iar0 = 0x00; //clear RAM  

	_mp1h				= 1;

	for (_mp1l = 0x80; _mp1l != 0x00; _mp1l ++)
		_iar1 = 0x00; //clear RAM  

	_mp2h				= 2;

	for (_mp2l = 0x80; _mp2l != 0x00; _mp2l ++)
		_iar1 = 0x00; //clear RAM  

	_mp1h				= 0;
	_mp2h				= 0;
	FeedWatchDog();

}

void Init_IO(void)
{
	/* set PC as output */
	_pc 				= 0x00;
	_pcc				= 0x00;
	_pc1				= 1;
	_pc3				= 1;
	_pc5				= 1;

#ifdef MSKMS_HW

	// PC as Gate output
	_pc0s1				= 1;
	_pc0s0				= 0;
	_pc1s1				= 1;
	_pc1s0				= 0;
	_pc2s1				= 1;
	_pc2s0				= 0;
	_pc3s1				= 1;
	_pc3s0				= 0;
	_pc4s1				= 1;
	_pc4s0				= 0;
	_pc5s1				= 1;
	_pc5s0				= 0;

#else

	_pcps0				= 0x00;
	_pcps1				= 0x00;
#endif

	// set pb0,pb1,pb2 as hall output
	_pb 				= 0x00;
	_pbc0				= 0;
	_pbc1				= 0;
	_pbc2				= 0;
	_pbc6				= 1;						//RX
	_pbc7				= 0;						//TX
	_pb0s1				= 0;						//HAO
	_pb0s0				= 1;
	_pb1s1				= 0;						//HBO
	_pb1s0				= 1;
	_pb2s1				= 0;						//HCO
	_pb2s0				= 1;
	_pb7s1				= 0;						//TX
	_pb7s0				= 1;
	_pb6s1				= 0;						//RX
	_pb6s0				= 1;

	// set pb0, pb1,pb2 as debug output
	//_pb = 0;
	//_pbc = 0;

	/* set OCDS port as output */
	_pac0				= 0;
	_pac2				= 0;
	_pa 				= 0x00;

	// PA1(AP) as ADC for current sense
	_pa1s1				= 0;
	_pa1s0				= 1;

	// PA6(AN7) as ADC for voltage sense
	_pa6s1				= 1;
	_pa6s0				= 0;

	// PA7(AN6) for test
	_pa7s1				= 0;
	_pa7s0				= 1;

	// PA3,PA4,PA5 as comparator input
	_pa3s1				= 0;						// C1P
	_pa3s0				= 1;
	_pa4s1				= 1;						// C2P
	_pa4s0				= 0;
	_pa5s1				= 1;						// C3p
	_pa5s0				= 0;

	// PB3 as CPN, C1N, C2N and C3N will be shorted
	_pbc3				= 0;
	_pb3s1				= 1;
	_pb3s0				= 0;

	// PD2, PD3 as test output
	_pd 				= 0;
	_pdc2				= 0;
	_pdc3				= 0;

}


void Init_Int(void)
{
	// enable hall sensor interrupt
	_halaf				= 0;
	_halbf				= 0;
	_halcf				= 0;
	_halae				= 1;
	_halbe				= 1;
	_halce				= 1;
	INTEN_HALL			= 0;

	// enable CMP0
	_int_pri2f			= 0;
	_int_pri2e			= 1;
	
	// ADC
	_aeocf = 0;
	_aeoce = 0;
	_isaeocf = 0;
	_isaeoce = 1;
	_int_pri8f = 0;
	_int_pri8e = 1;

	// TM0
	_tm0af				= 0;
	_tm0ae				= 1;
	_int_pri10f 		= 0;
	_int_pri10e 		= 1;

	// TM1
	_tm1af				= 0;
	_tm1ae				= 1;
	_int_pri11f 		= 0;
	_int_pri11e 		= 1;

	// TM3
	_tm3af				= 0;
	_tm3ae				= 1;
	_int_pri14f 		= 0;
	_int_pri14e 		= 1;


	// Time Base
	_tbf				= 0;
	_tbe				= 1;
	_int_pri15f 		= 0;
	INTEN_TMB			= 1;

	// PWM
	_pwmpf				= 0;
	_pwmd0f 			= 0;
	_pwmd1f 			= 0;
	_pwmd2f 			= 0;
	_int_pri3f			= 0;

	_pwmpe				= 1;
	_pwmd0e 			= 0;
	_pwmd1e 			= 0;
	_pwmd2e 			= 0;
	INTEN_PWM			= 1;

	// UART
	_uartf 				= 0;				
	_int_pri15f			= 0;

	_uarte				= 1;	
	_int_pri15e			= 1;
	
	DI();
}


void Init_PWM(void)
{
	// PWMC
	_pwmms1 			= 0;						//0x:Edge-aligned mode				
	_pwmms0 			= 0;						//10: Center-aligned mode 1

	//11: Center-aligned mode 2
	_pwmon				= 0;						// pwm on	
	_pcks0				= 0;						// fpwm
	_pcks1				= 0;
	_pwmld				= 1;						// update prdr

	// PWMCS
	_pwmsuf 			= 1;						// dutr synchronous update request
	_pwmsu				= 1;						//	dutr synchronous update enable
	_pwmsv				= 1;						// duty written to dutr0~2		

	// DUTR0, PRDR
	_dutr0h 			= 0x00; 					// duty 0.1
	_dutr0l 			= 0x00;
	_prdrh				= 0x01; 					// 20kHz, 400	
	_prdrl				= 0x90;

	// MCF
#ifdef MSKMS_HW
	_mskms				= 0;						// 0:hardware 1:software

#else

	_mskms				= 1;
#endif

	_mpwms				= 1;						// non-complementary
	_mpwe				= 1;						// output enable
	_fmos				= 0;						// protect output selection
	_pwms				= 1;						// top pwm output

	// PWMME
	_pwmme				= 0x00;

	//PWMMD	
	_pwmmd				= 0x00;

	//DTS
	_dtcks1 			= 1;						// fsys/8
	_dtcks0 			= 1;

	// ===== hall sensor decoder ====
	//	INTEG0
	_hsel				= 1;						// cmp
	_intcs1 			= 0;						// diable edge triger interrupt
	_intcs0 			= 0;
	_intbs1 			= 0;
	_intbs0 			= 0;
	_intas1 			= 0;
	_intas0 			= 0;
	_integ0 			= 0B01000000;

	//	HDCR
	_ctm_sel1			= 0;						// TM select
	_ctm_sel0			= 0;


#ifdef MSKMS_HW
	_hdly_msel			= 1;						// hall delay path enabled

#else

	_hdly_msel			= 0;						// hall delay path disabled
#endif

	_hals				= 1;						// 120 degree
	_hdms				= 0;						// software decoder mode
	_brke				= 0;						// brake mode
	_frs				= 0;						//forward

#ifdef MSKMS_HW
	_hdcen				= 1;						//enable

#else

	_hdcen				= 0;						//disable
#endif

	// HDCD
	_sha				= 0;
	_shb				= 0;
	_shc				= 0;

	// HDCTn
	// [SC,SB,SA] => [HAT,HAB,HBT,HBB,HCT,HCB]
	_hdct0				= 0B00100100;				//SC SB SA(001) 		1
	_hdct1				= 0B00100001;				//SC SB SA(011) 		3		
	_hdct2				= 0B00001001;				//SC SB SA(010) 		2
	_hdct3				= 0B00011000;				//SC SB SA(110) 		6
	_hdct4				= 0B00010010;				//SC SB SA(100) 		4
	_hdct5				= 0B00000110;				//SC SB SA(101) 		5	

	_hdct6				= 0B00100100;				//SC SB SA(001) 		1
	_hdct7				= 0B00100001;				//SC SB SA(011) 		3		
	_hdct8				= 0B00001001;				//SC SB SA(010) 		2
	_hdct9				= 0B00011000;				//SC SB SA(110) 		6
	_hdct10 			= 0B00010010;				//SC SB SA(100) 		4
	_hdct11 			= 0B00000110;				//SC SB SA(101) 		5	

	// HCHK_NUM
	_hchk_num			= NUM_DRAG; 				// noise filter check time

	// HFN_MSEL
	_hnf_en 			= 1;						// enable noise filter
	_hfr_sel2			= 0;						// filter clock source
	_hfr_sel1			= 1;
	_hfr_sel0			= 1;

	//====	Motor protection	====
	// MPTC1
	_pswd				= 0;						//software protection
	_pswe				= 0;
	_pswps				= 0;
	
	_ahlhe				= 0;
	_ahlps				= 0;	
	_ishe				= 1;
	_isps				= 0;
	
	_capche				= 0;
	_capcps				= 1;
	_capohe				= 0;
	_capops				= 1;
	
	_ocpse				= 1;

	// MPTC2
	_isps				= 1;
	
	// OCPS
	_ocps = 0;
	// I/O Init
	//	
	PWM_SET(0, 200);
	_pwmon				= 1;

}




void Init_OCP(uint8 idata) // idata = i * R * 1020 
{
	_opoms				= 0x43; 					// rising edge trigger, AV=20
	_opcm				= idata;					
	
	//OPA0CAL
	_a0rs = 1;
	_a0ofm	= 0;
	
	//CMPC	
	_c0hyen 			= 1;
	_c0en 				= 1;
			
	//interrupt
	_int_pri2f = 0;
	_int_pri2e = 1;
}

void Init_ADC(uint8 res)
{
	// ==	ADCR0	==
	_adrfs				= 0;						//12-bit data format (ADCRL_SEL=0):
													//0: High Byte=D[11:4]; Low Byte=D[3:0]
													//1: High Byte=D[11:8]; Low Byte=D[7:0]
													//10-bit data format (ADCRL_SEL=1):
													//0: High Byte=D[9:2]; Low Byte=D[1:0]
													//1: High Byte=D[9:8]; Low Byte=D[7:0]
	_adoff 				= 1; 						// ADC on
	Adc_ch_sel0(ADC_VDC);

	/* ADCR1	*/
	_dlstr = 1;										// auto scan 
	_pwis	= 0;	// period auto scan	
	_adchve =	1;									//00: Low boundary value < Converted data < High boundary value
	_adclve	=	0;									//01: Converted data <= Low boundary value
													//10: Converted data >= High boundary value
													//11: Converted data <= Low boundary value or Converted data >= High boundary value
	_adck2				= 0;						// fsys/4 = 4MHz, 250ns
	_adck1				= 1;
	_adck0				= 0;

	//ADRC2
	_adcrl_sel			= res;					// resolution
	_adch_sel1			= 1;						// 3 channel to scan
	_adch_sel0			= 0;


	Adc_ch_sel1(0, ADC_IS);
	Adc_ch_sel1(1, ADC_VDC);
	Adc_ch_sel1(2, ADC_VSP);

	// ADDL, delay time, dt=1us/16
	_addl = 48;
	// ADBYPS
	_ugb_on 			= 1;						// buffer on

	//
	_adoff				= 0;						// adc on
	_isaeocf = 0;
	_isaeoce = 1;
}


// for ADSTR triggered A/D conversion
void Adc_ch_sel0(uint8 ch)
{
	_acs3				= (ch >> 3) & 0x01;
	_acs2				= (ch >> 2) & 0x01;
	_acs1				= (ch >> 1) & 0x01;
	_acs0				= ch & 0x01;
}

/* set ADC auto scan channel */
// ADISG1, auto scan channel 1,0 
// ADISG2, auto scan channel 3,2
void Adc_ch_sel1(uint8 order, uint8 ch)
{
	uint8			temp;

	if (order == 0)
	{
		temp				= _adisg1;
		_adisg1 			= (temp & 0xF0) | ch;
	}
	else if (order == 1)
	{
		temp				= _adisg1;
		_adisg1 			= (temp & 0x0F) | (ch << 4);
	}
	else if (order == 2)
	{
		temp				= _adisg2;
		_adisg2 			= (temp & 0xF0) | ch;
	}
	else if (order == 3)
	{
		temp				= _adisg2;
		_adisg2 			= (temp & 0x0F) | (ch << 4);
	}
}


void ADC_Trig(void)
{
	_adstr				= 0;
	_adstr				= 1;
	_adstr				= 0;
}

void Adc_Read_Auto()
{
	Ad_Is = _isrh0;
	Ad_Vdc = _isrh1;
	Ad_Vsp = _isrh2;
	_iseocb = 0;	
}


void Init_CAPTM(void)
{
	_captpau			= 0;						// run CAPTM
	_captck2			= 1;						// fh/128
	_captck1			= 1;
	_captck0			= 1;
	_capton 			= 0;						// CAPTM off

}


void Init_TM0(void) // for hall delay circuit
{
	_ptm0c0 			= 0b00100000;				//1us, off

#ifdef MSKMS_HW
	_ptm0c1 			= 0b00000001;				// Compare match output, comparator A

#else

	_ptm0c1 			= 0b11000001;				// timer /counter mode, A match clear
#endif

	_ptm0al 			= 0xF0; 					// low byte first, 100us
	_ptm0ah 			= 0xFF;

	_tm0af				= 0;
	_tm0ae				= 1;
	_pt0on				= 1;
}


void Init_TM1(void)
{
	_ptm1c0 			= 0b00100000;				//fh/16, 1u
	_ptm1c1 			= 0b11000001;				// timer /counter mode, A match clear

	_ptm1al 			= 0xf0; 					// low byte first
	_ptm1ah 			= 0xff;

	_tm1af				= 0;
	_tm1ae				= 1;
	_pt1on				= 1;
}



void Init_TM3(void)
{
	_ptm3c0 			= 0b00100000;				// fh/16, 1us
	_ptm3c1 			= 0b11000001;				// timer /counter mode, A match clear
	_ptm3al 			= 0xF0; 					// low byte first, 100us
	_ptm3ah 			= 0xFF;

	_tm3af				= 0;
	_tm3ae				= 1;
	_pt1on				= 1;
}



void Init_Comp(void)
{
	_c3hyen 			= 1;
	_c2hyen 			= 1;
	_c1hyen 			= 1;
	_c0hyen 			= 1;
	_c3en				= 1;
	_c2en				= 1;
	_c1en				= 1;
	_c0en				= 0;
}


void Init_TimeBase(void) // 16MHz/16384 = 1.024ms
{
	_tbc				= 0xc0;
	CLRF_TMB;
	_tbe				= 1;
	INTEN_TMB			= 1;
	FeedWatchDog();
}


void TM0_Dly_Set(uint16 dly)
{
	_ptm0al 			= dly & 0xFF;
	_ptm0ah 			= (dly >> 8) & 0xFF;
}


void TM3_Dly_Set(uint16 dly)
{
	_ptm3al 			= dly & 0xFF;
	_ptm3ah 			= (dly >> 8) & 0xFF;
}



void PWM_Duty(uint16 duty)
{
	//duty = PWM_MAX -duty;
	_dutr0h 			= (uint8) (duty >> 8);
	_dutr0l 			= (uint8) (duty & 0xFF);
	_pwmsuf 			= 1;						// this bit must enable to update pwm duty
}


void PWM_SET(uint8 mode, uint16 duty)
{

	if (mode == 0)
	{
		_pwmms1 			= 0;
		_prdrh				= 0x03; 				// 20kHz, 800	
		_prdrl				= 0x20;
	}
	else if (mode == 1)
	{
		_pwmms1 			= 1;
		_pwmms0 			= 0;
		_prdrh				= 0x01; 				// 20kHz, 400	
		_prdrl				= 0x90;
	}
	else 
	{
		_pwmms1 			= 1;
		_pwmms0 			= 1;
		_prdrh				= 0x01; 				// 20kHz, 400	
		_prdrl				= 0x90;
	}

	if (mode < 2)
	{
		_dutr0h 			= (uint8) (duty >> 8);
		_dutr0l 			= (uint8) (duty & 0xFF);
	}
	else 
	{
		duty				= PWM_MAX - duty;
		_dutr0h 			= (uint8) (duty >> 8);
		_dutr0l 			= (uint8) (duty & 0xFF);
	}

	_pwmsuf 			= 1;						// this bit must enable to update pwm duty	
}


//1:1.37us
//10: 5.3us
//100:45us
void delay1(uint8 n)
{
	unsigned char	j;

	for (j = 0; j < n; j++)
		FeedWatchDog();
}


// 1: 10us
// 10:100us
void delay_10u(uint16 x)
{
	unsigned int	i;
	unsigned char	j;

	for (i = 0; i < x; i++)
		for (j = 0; j < 14; j++)
			FeedWatchDog();

	FeedWatchDog();
}


void delay_ms(uint16 x)
{
	unsigned int	i;
	unsigned char	j;
	unsigned char	k;

	for (i = 0; i < x; i++)
		for (j = 0; j < 16; j++)
			for (k = 0; k < 250; k++)
				FeedWatchDog();
}


void delay_tm0(uint16 dly)
{
	uint16			cnt;

	_pt0on				= 0;
	_pt0on				= 1;

	while (1)
	{
		cnt 				= _ptm0dh << 8 | _ptm0dl;

		if (cnt >= dly)
		{
			break;
		}

		FeedWatchDog();
	}
}


void delay_tm1(uint16 dly)
{
	uint16			cnt;

	_pt1on				= 0;
	_pt1on				= 1;

	while (1)
	{
		cnt 				= _ptm1dh << 8 | _ptm1dl;

		if (cnt >= dly)
		{
			break;
		}

		FeedWatchDog();
	}
}

void Init_UART(void)
{
	//baud rate	BRG(calculated)	BRG 	actural	baud rate	error(%)
	//==================================================================
	//9600		103.1666667 	103 	9615.384615 		0.16025641
	//14400		68.44444444 	68		1	4492.75362 		0.644122383
	//19200		51.08333333 	51		19230.76923 		0.16025641
	//38400		25.04166667 	25		38461.53846 		0.16025641
	//57600		16.36111111 	16		58823.52941 		2.124183007
	//76800		12.02083333 	12		76923.07692 		0.16025641
	//115200	7.680555556 	8		111111.1111 		-3.549382716
	//128000	6.8125			7		125000				-2.34375
	//256000	2.90625			3		250000				-2.34375


	_uarten = 1;	// enable uart pins	
	_adden = 0;		// address detect disabled
	_wake = 0;
	_rie = 1;		// RX interrupt enabled
	_tiie = 0;		// TX interrupt disabled
	_teie = 0;		// Transmitter empty interrupt is disabled	
	
	_bno = 0;		// 8b
	_pren = 0;		// parity none
	_stops = 0; 	// 1 stop bit
	_txbrk = 0; 	// no break

	_brgh = 1;		// high speed
	_brg = 1;		// 256000
	
	_txen = 1;
	_rxen = 1;	
}


boolean Uart_Tx(uint8 txdata)
{
	uint8			errcnt = 0;

	while (1)
	{
		if (errcnt++ == 255)
			return 1;

		if (_txif)
			break;

		FeedWatchDog();
	}

	_txr_rxr			= txdata;
	return 0;
}


void Duty_Rampup(void)
{
	if ((state_motor >= RAMP) )//&& (uiCommCycle >=3))
	{
		TO1 				= ~TO1;
		//if (uiCommCycle > 20)
		//if (Ad_Is <= ILIM_CC_RAMP)
		{

			if (uiDutyRamp < uiDutyFinal) 
			{
				uiDutyRamp			+= PWM_INC;
				if (uiDutyRamp >= uiDutyFinal)
					uiDutyRamp = uiDutyFinal;
				PWM_Duty(uiDutyRamp);
			}
			else if (uiDutyRamp > uiDutyFinal)
			{			
				uiDutyRamp			-= PWM_INC;
				if (uiDutyRamp <= PWM_DRAG_START)
					uiDutyRamp = PWM_DRAG_START;
				PWM_Duty(uiDutyRamp);					
			}			
		}
		/*else
		{	
					
			uiDutyRamp			-= PWM_INC;
			if (uiDutyRamp <= PWM_DRAG_START)
				uiDutyRamp = PWM_DRAG_START;
			PWM_Duty(uiDutyRamp);	
						
		}*/
			
		if ((uiDutyRamp >= (uiDutyFinal - 5)) && (uiDutyRamp <= (uiDutyFinal + 5)))
		{
			state_motor = RUN;
			_hchk_num = NUM_RUN;
		}		
	}	
	

}


void Init_Vars(void)
{
	ucCommStep			= 0;
	uiHallCnt			= 0;
	state_motor 		= DRAG;
	ucCommStep			= 0;
	uiCommCycle 		= 0;
	ucDragStep			= 0;
	uiHallCnt			= 0;
	uiDutyRamp			= PWM_DRAG_START;
	uiDutyFinal 			= 800;
	cc_start = 0;
}
