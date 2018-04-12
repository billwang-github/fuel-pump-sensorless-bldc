#include "HT66FM5440.h"
#include "mydef.h"

//static volatile	_bits bit_var_0		__attribute__	((at(0x80)));
//static volatile	__hall_type	__attribute__	((at(0x81)));
//static volatile	_bits bit_var_2		__attribute__	((at(0x82)));

extern uint16 uiHallPeriod, uiHallCnt , uiCommCycle ;
extern uint8 state_motor, ucCommStep;


void 	Var_Init(void);
void 	delay_10u(uint16 x);
void 	delay1(uint8 n);
void 	delay_ms(uint16 x);

void 	Init_IO(void);
void 	Init_System(void);
void 	Init_PWM(void);
void 	Init_Comp(void);
void 	Init_CAPTM(void);
void 	Init_TM0(void);
void 	Init_TM1(void);
void 	Init_TM3(void);
void 	Init_Int(void);
void 	Init_ADC(void);
void 	Init_TimeBase(void);

void 	ADC_Ch_Sel(uint8 ch);
void 	ADC_Trig(void);
void 	ADC_AC_CH(uint8 order, uint8 ch);


void 	Motor_Start(void);
void 	PWM_Duty(uint16 duty);
void 	PWM_SET(uint8 mode, uint16 duty);


void 	delay_tm1(uint16 dly);
void 	Hall_Int_Set(uint8 step);

void 	Drag_Motor(void);
void 	Commutation(void);
void 	TM3_Dly_Set(uint16 dly);


_bits status = {0,0,0,0,0,0,0,0};
extern uint8 ucDragTmr;
extern uint16 uiDragDly;
extern uint8 ucCount1ms;

uint16 duty_max = 800;  //period 800
const uint8 cw_pattern[] = {1,3,2,6,4,5};
const uint8 ccw_pattern[] = {1,5,4,6,2,3};	
//const uint8 drag_time[]= {20,19, 17, 14, 11, 9};
const uint16 drag_time[]= {20,20,19, 17, 14, 11, 9};
//const uint16 drag_time[]= {25,25,22, 22, 19, 17, 14};




static uint8 hall_step = 0;
static uint8 cnt_hall = 0;

static uint16 period_hall = 0;
static uint16 period_comm = 0;

static uint16 cnt_comm = 0;
static uint8 ucDragStep = 0;

static uint16 uiHallTmr = 0;
//static uint16 uiHallDlyTmr = 0;

//static uint16 uiHallPeriod_Old = 0;

//static uint16 uiHallDly = 0;


extern uint16 uiDutyRamp;
//static uint16 uiDutyRampTmr = 0;

boolean bHallU_new = 0;
boolean bHallU_old = 0;
boolean bHallV_new = 0;
boolean bHallV_old = 0;
boolean bHallW_new = 0;
boolean bHallW_old = 0;

//static union __hall_type hall_bits;
static boolean bHallU_zc = 0;
/*
boolean bHallU_zcr = 0;
boolean bHallV_zcr = 0;
boolean bHallW_zcr = 0;
boolean bHallU_zcf = 0;
boolean bHallV_zcf = 0;
boolean bHallW_zcf = 0;
*/

void Var_Init(void)
{	
	hall_step = 0;
	cnt_hall = 0;
	period_hall = 0;
	period_comm = 0;
	cnt_comm = 0;
	state_motor = STOP;	
//	bOCPFlag = 0;
//	bStallFlag = 0;
}


void main()
{
	GCC_NOP();
	GCC_NOP();
	GCC_NOP();	
	_emi = 0;
	// watch dog timer clock
	_ws0 = 1;
	_ws1 = 1;
	_ws2 = 1;
	
	//Init_System();	
	Var_Init();	
	Init_IO();
	Init_Comp();
	//Init_CAPTM();
	Init_ADC();	
	Init_TM0();
	Init_TM1();
	Init_TM3();	
	//Init_Motor_Ctrl();
	
	Init_Int();
	_emi = 1;
		
	Init_PWM();
	ucCommStep = 0;
	uiHallCnt = 0;
	_pwmon = 1;	

	state_motor = DRAG;
	ucCommStep = 0;	
	uiCommCycle = 0;
	uiDutyRamp = 700;	
	PWM_SET(0, uiDutyRamp);
	ucDragStep = 0;
	uiHallCnt = 0;
	uiHallTmr = 0;
	bHallU_zc = 0;	
	Init_TimeBase();
	while (1)
	{
	
		if (bNmsFlag)
		{
			bNmsFlag = 0;
			

												
		}	
		WDT_RESET;
	}

}


void Init_System(void)
{
	_smod = 0b00000001;                   
	for(_mp0 = 0x80;_mp0 != 0x00;_mp0 ++)_iar0 = 0x00;		//clear RAM  
	_mp1h = 1;
	for(_mp1l = 0x80;_mp1l != 0x00;_mp1l++)_iar1 = 0x00;	//clear RAM  
	_mp2h = 2;
	for(_mp2l = 0x80;_mp2l != 0x00;_mp2l++)_iar1 = 0x00;	//clear RAM  
	_mp1h = 0;
	_mp2h = 0;
	asm("CLR	WDT");

}

void Init_IO(void)
{
	/* set PC as output */
	_pc   = 0x00;	
	_pcc  = 0x00;		
	_pc1 = 1;
	_pc3 = 1;
	_pc5 = 1;
	
	// PC as Gate output
	/*	
	_pc0s1 = ;
	_pc0s0 = 0;		
	_pc1s1 = 1;
	_pc1s0 = 0;
	_pc2s1 = 1;
	_pc2s0 = 0;
	_pc3s1 = 1;
	_pc3s0 = 0;
	_pc4s1 = 1;
	_pc4s0 = 0;
	_pc5s1 = 1;
	_pc5s0 = 0;	*/	
	_pcps0 = 0x00;
	_pcps1 = 0x00;

	
	// set pb0,pb1,pb2 as hall output
	_pb = 0x00;
	_pbc0 = 0;
	_pbc1 = 0;
	_pbc2 = 0;
	_pb0s1 = 0; //HAO
	_pb0s0 = 1;
	_pb1s1 = 0; //HBO
	_pb1s0 = 1;
	_pb2s1 = 0; //HCO
	_pb2s0 = 1;		
	
	// set pb0, pb1,pb2 as debug output
	//_pb = 0;
	//_pbc = 0;
					
	/* set OCDS port as output */
	_pac0 = 0;
	_pac2 = 0;	
	_pa  = 0x00;

	// PA1(AP) as ADC for current sense
	_pa1s1 = 0;
	_pa1s0 = 1;
	
	// PA6(AN7) as ADC for voltage sense
	_pa6s1 = 1;
	_pa6s0 = 0;

	// PA7(AN6) for test
	_pa7s1 = 0;
	_pa7s0 = 1;
	
	// PA3,PA4,PA5 as comparator input
	_pa3s1 = 0; // C1P
	_pa3s0 = 1;
	_pa4s1 = 1; // C2P
	_pa4s0 = 0;
	_pa5s1 = 1;	// C3p
	_pa5s0 = 0;	
	
	// PB3 as CPN, C1N, C2N and C3N will be shorted
	_pbc3 = 0;
	_pb3s1 = 1;
	_pb3s0 = 0;
	
	// PD2, PD3 as test output
	_pd =0;
	_pdc2 = 0;
	_pdc3 = 0;
	
}



//===============================================
//
//===============================================
void Init_ADC(void)
{
	/* ADCR0 	*/	
	_adrfs = 0;	 // High Byte=D[9:2]; Low Byte=D[1:0]
	ADC_Ch_Sel(AN6);

	/* ADCR1	*/
	_adck2 = 1; // fsys/16 = 1MHz
	_adck1 = 0;
	_adck0 = 0;	 	
		
	//ADRC2
	_adcrl_sel = ADC_10b;	// 10 bit
	_adch_sel1 = 1; 		// 3 channel to scan
	_adch_sel0 = 0;
	
	// ADISG1, auto scan channel 1,0 
	// ADISG2, auto scan channel 3,2
	ADC_AC_CH(0, OPA0);
	ADC_AC_CH(1, AN7);
	ADC_AC_CH(2, AN6);
	
	// ADDL, delay time
	
	// ADBYPS
	_ugb_on = 1; // buffer on
	
	//
	_adstr = 1;
	_dlstr = 1;
	_adoff = 0;  // adc on
	delay1(100);	
}

void ADC_Ch_Sel(uint8 ch)
{
	_acs3 = (ch >> 3) & 0x01;
	_acs2 = (ch >> 2) & 0x01;
	_acs1 = (ch >> 1) & 0x01;
	_acs0 = ch & 0x01;	
}

void ADC_Trig(void)
{
	_adstr = 0;
	_adstr = 1;
	_adstr = 0;
}

/* set ADC auto scan channel */
void ADC_AC_CH(uint8 order, uint8 ch) 
{
	uint8 temp ;

	if (order == 0)
	{
		temp = _adisg1;	
		_adisg1 = (temp & 0xF0)	| ch;
	}
	else if (order == 1)
	{
		temp = _adisg1;	
		_adisg1 = (temp & 0x0F)	| (ch << 4);
	}
	else if (order == 2)
	{
		temp = _adisg2;	
		_adisg2 = (temp & 0xF0)	| ch;
	}
	else if (order == 3)
	{
		temp = _adisg2;	
		_adisg2 = (temp & 0x0F)	| (ch << 4);
	}
}



void Init_CAPTM(void)
{	
	_captpau = 0; 	// run CAPTM
	_captck2 = 1;	// fh/128
	_captck1 = 1;
	_captck0 = 1;
	_capton = 0;	// CAPTM off

}	

void Init_TM1(void)
{
    _ptm1c0=0b00100000;  //fh/16, 1u
	_ptm1c1=0b11000001;	 // timer /counter mode, A match clear
	
	_ptm1al=0xf0; // low byte first
	_ptm1ah=0xff;
	
	_tm1af = 0;
	_tm1ae = 1;
	_pt1on = 1;
}

void Init_TM0(void)
{
    _ptm0c0=0b00100000;  //1us
	_ptm0c1=0b11000001;	 // timer /counter mode, A match clear
	
	_ptm0al=0x00; 		// low byte first
	_ptm0ah=0xFF;
	
	_tm0af = 0;
	_tm0ae = 1;
	_pt0on = 1;
}

void Init_TM3(void)
{
    _ptm3c0 = 0b00100000;   // fh/16, 1us
	_ptm3c1 = 0b11000001;	// timer /counter mode, A match clear
	
	TM3_Dly_Set(250);
	_tm3af = 0;
    _tm3ae = 1;	

}

void TM3_Dly_Set(uint16 dly)
{
	_ptm3al = dly & 0xFF;
	_ptm3ah = (dly >> 8) & 0xFF;
}

void Init_Int(void)
{
	// enable hall sensor interrupt
	_halaf = 0;
	_halbf = 0;
	_halcf = 0;
	_halae = 1;
	_halbe = 1;					
	_halce = 1;
	INTEN_HALL = 0;	
	
	// enable CMP0
	_int_pri2f = 0;
	_int_pri2e = 1;
	
	// TM0
	_tm0af = 0;
	_tm0ae = 1;
	_int_pri10f = 0;
	_int_pri10e = 1;
	
	// TM1
	_tm1af = 0;
	_tm1ae = 1;	
	_int_pri11f = 0;
	_int_pri11e = 1;	
	
	// TM3
	_tm3af = 0;
	_tm3ae = 1;	
	_int_pri14f = 0;
	_int_pri14e = 1;


	// Time Base
	_tbf = 0;
	_tbe = 1;
	_int_pri15f = 0;
	INTEN_TMB = 1;
	
	// PWM
	_pwmpf = 0;
	_pwmd0f = 0;
	_pwmd1f = 0;
	_pwmd2f = 0;
	_int_pri3f = 0;
	
	_pwmpe = 0;
	_pwmd0e = 0;
	_pwmd1e = 0;
	_pwmd2e = 0;
	INTEN_PWM= 0;
			
	_emi = 0;
}

//==================================================
void Init_Comp(void)
{
	_c3hyen = 1;
	_c2hyen = 1;
	_c1hyen = 1;
	_c0hyen = 1;
	_c3en = 1;
	_c2en = 1;
	_c1en = 1;
	_c0en = 1;
}
void Init_OCP(void)
{
	_opoms = 0x43; // rising edge trigger, AV=20
	_a0rs = 1; // offset cal on
	_a0ofm = 1;		
	_opcm = 103; // 2V, 2.5A
	
	_c0hyen = 1;
	_c0en = 1;

}

void Init_PWM(void)
{
// PWMC
	_pwmms1 = 1; 	//0x:Edge-aligned mode				
	_pwmms0 = 1;	//10: Center-aligned mode 1
					//11: Center-aligned mode 2
	_pwmon 	= 0; // pwm on	
	_pcks0 	= 0; // fpwm
	_pcks1  = 0;
	_pwmld 	= 1; // update prdr
// PWMCS
	_pwmsuf = 1; // dutr synchronous update request
	_pwmsu 	= 1; //	dutr synchronous update enable
	_pwmsv	= 1; // duty written to dutr0~2		
// DUTR0, PRDR
	_dutr0h = 0x00;   	// duty 0.1
	_dutr0l = 0x00; 
	_prdrh 	= 0x01;		// 20kHz, 400   
	_prdrl 	= 0x90;
// MCF
	_mskms 	= 1; // 0:hardware 1:software
	_mpwms 	= 1; // non-complementary
	_mpwe 	= 1; // output enable
	_fmos 	= 0; // protect output selection
	_pwms 	= 1;	// top pwm output
// PWMME
	_pwmme = 0x00;
//PWMMD	
	_pwmmd = 0x00;
//DTS
	_dtcks1 = 1; // fsys/8
	_dtcks0 = 1;
	
// ===== hall sensor decoder ====
//	INTEG0
	_hsel = 1;	// cmp
	_intcs1 = 0; // diable edge triger interrupt
	_intcs0 = 0;
	_intbs1 = 0;
	_intbs0 = 0;		
	_intas1 = 0;
	_intas0 = 0;							
//	HDCR
	_ctm_sel1 = 1;	// TM select
	_ctm_sel0 = 1;
	_hdly_msel = 0; // no hall delay
	_hals = 1; // 120 degree
	_hdms = 0; // software decoder mode
	_brke = 0; // brake mode
	_frs = 0; //forward
	_hdcen = 1; //enable
// HDCD
	_sha = 0;
	_shb = 0;
	_shc = 0;
// HDCTn
	_hdct0 = 0B00100100;	//SC SB SA(001)      	1
	_hdct1 = 0B00100001;	//SC SB SA(011) 		3		
	_hdct2 = 0B00001001;	//SC SB SA(010) 		2
	_hdct3 = 0B00011000;	//SC SB SA(110) 		6
	_hdct4 = 0B00010010;	//SC SB SA(100) 		4
	_hdct5 = 0B00000110;	//SC SB SA(101) 		5	
	
	_hdct6 = 0B00100100;	//SC SB SA(001)      	1
	_hdct7 = 0B00100001;	//SC SB SA(011) 		3		
	_hdct8 = 0B00001001;	//SC SB SA(010) 		2
	_hdct9 = 0B00011000;	//SC SB SA(110) 		6
	_hdct10 = 0B00010010;	//SC SB SA(100) 		4
	_hdct11 = 0B00000110;	//SC SB SA(101) 		5	
// HCHK_NUM
	_hchk_num = 0x16; // noise filter check time
// HFN_MSEL
	_hnf_en = 1; 	// enable noise filter
	_hfr_sel2 = 0;	// filter clock source
	_hfr_sel1 = 1;	
	_hfr_sel0 = 1;	
//====	Motor protection 	====
// MPTC1
	_pswd = 0; //software protection
	_pswe = 0;
	_ishe = 1;
	_isps = 0;
	_ocpse = 1;;
	
// MPTC2
	_isps = 1;	

// I/O Init
	_pc = 0x00;
	_pcc = 0x00;
	_pcps0 = 0x00;
	_pcps1 = 0x00;
//	
	PWM_SET(0, 200);
	_pwmon = 1;
	
}

void PWM_Duty(uint16 duty)
{
	//duty = PWM_MAX -duty;
	_dutr0h = (uint8)(duty >> 8);   
	_dutr0l = (uint8)(duty & 0xFF); 
	_pwmsuf = 1;  // this bit must enable to update pwm duty
}	

void PWM_SET(uint8 mode, uint16 duty)
{
	
	if (mode == 0)
	{
		_pwmms1 = 0;	
		_prdrh 	= 0x03;		// 20kHz, 800   
		_prdrl 	= 0x20;		
		duty_max  = 800;
	}	
	else if (mode == 1)
	{
		_pwmms1 = 1;	
		_pwmms0 = 0;
		_prdrh 	= 0x01;		// 20kHz, 400   
		_prdrl 	= 0x90;			
		duty_max  = 400;
	}	
	else 
	{
		_pwmms1 = 1;	
		_pwmms0 = 1;
		_prdrh 	= 0x01;		// 20kHz, 400   
		_prdrl 	= 0x90;			
		duty_max  = 400;
	}
		
	if (mode < 2)
	{
		_dutr0h = (uint8)(duty >> 8);   
		_dutr0l = (uint8)(duty & 0xFF); 		
	}
	else
	{
		duty = duty_max -duty;
		_dutr0h = (uint8)(duty >> 8);   
		_dutr0l = (uint8)(duty & 0xFF); 		
	}
			
	_pwmsuf = 1;  // this bit must enable to update pwm duty	
}
//1:1.37us
//10: 5.3us
//100:45us
void delay1(uint8 n) 
{
    unsigned char j;
    for(j=0;j<n;j++)
    WDT_RESET;		
}

// 1: 10us
// 10:100us
void delay_10u(uint16 x) 
{
    unsigned int i;
    unsigned char j;  
    for(i=0;i<x;i++)
    for(j=0;j<14;j++)
   	WDT_RESET;
	WDT_RESET;
}


void delay_ms(uint16 x) 
{
    unsigned int i;
    unsigned char j;  
    unsigned char k;  
    for(i=0;i<x;i++)
    for(j=0;j<16;j++)
    for(k=0;k<250;k++)
   	WDT_RESET;
}

void delay_tm0(uint16 dly)
{
	uint16 cnt;
	
	_pt0on = 0;
	_pt0on = 1;
	while (1)
	{
		cnt = _ptm0dh << 8 | _ptm0dl ;
		if (cnt >= dly)
		{ 
			break;
		}
		WDT_RESET;
	}
}

void delay_tm1(uint16 dly)
{
	uint16 cnt;
	
	_pt1on = 0;
	_pt1on = 1;
	while (1)
	{
		cnt = _ptm1dh << 8 | _ptm1dl ;
		if (cnt >= dly)
		{ 
			break;
		}
		WDT_RESET;
	}
}


void Init_TimeBase(void) // 16MHz/16384 = 1.024ms
{
    _tbc = 0xc0;
	CLRF_TMB;    
    _tbe = 1;	
    INTEN_TMB = 1;
    WDT_RESET;
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
		uiDragDly = drag_time[ucDragStep] ;				
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