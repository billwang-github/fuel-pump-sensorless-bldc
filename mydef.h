#ifndef _MYDEF_H_
#define _MYDEF_H_

#include "HT66FM5440.h"
//#define MSKMS_HW

#define PWM_MAX 				800
#define PWM_DRAG_START			500
#define PWM_DRAG_END			800
#define PWM_INC 				5

#define NUM_DRAG				0x16
#define NUM_RAMP				0x16
#define NUM_RUN					0x01

typedef unsigned char boolean; /* Boolean value type. */
typedef unsigned long int uint32; /* Unsigned 32 bit value */
typedef unsigned short uint16; /* Unsigned 16 bit value */
typedef unsigned char uint8; /* Unsigned 8 bit value */
typedef signed long int int32; /* Signed 32 bit value */
typedef signed short int16; /* Signed 16 bit value */
typedef signed char int8; /* Signed 8 bit value */

typedef struct {
	unsigned char	b0				: 1;
	unsigned char	b1				: 1;
	unsigned char	b2				: 1;
	unsigned char	b3				: 1;
	unsigned char	b4				: 1;
	unsigned char	b5				: 1;
	unsigned char	b6				: 1;
	unsigned char	b7				: 1;
} _bits;


union __hall_type { 								// define union
	_bits			bits;							// using of structure
	uint8			byte;
};


#define bNmsFlag				status.b0

/*
#define bHallU_zcr	hall_bits.bits.b0
#define bHallU_zcf	hall_bits.bits.b1
#define bHallV_zcr	hall_bits.bits.b2
#define bHallV_zcf	hall_bits.bits.b3
#define bHallW_zcr	hall_bits.bits.b4
#define bHallW_zcf	hall_bits.bits.b5
#define bHallU_zc	hall_bits.bits.b6
*/
// Watch dog timer reset
#define WDT_RESET				asm("CLR	WDT")

// ADC resolution
#define ADC_10b 				1
#define ADC_12b 				0

// analog I/O
#define AN0 					0
#define AN1 					1
#define AN2 					2
#define AN3 					3
#define OPA2					4
#define OPA1					5
#define OPA0					6
#define AN6 					7
#define AN7 					8
#define ADC_ILIMIT				0
#define ADC_IS					3
#define ADC_VSP 				6
#define ADC_VDC 				7

// motor status
#define STOP					0
#define DRAG					1
#define RAMP					2
#define RUN 					3

// gate output
#define UT						_pc0
#define UB						_pc1
#define VT						_pc2
#define VB						_pc3
#define WT						_pc4
#define WB						_pc5

// interrupt enable
#define INTEN_TMB				_int_pri15e
#define INTEN_HALL				_int_pri1e
#define INTEN_TM3				_int_pri14e
#define INTEN_PWM				_int_pri3e
#define EMI_ON					_emi				= 1

// interrupt flag clear
#define CLRF_TMB				_int_pri15f = 0; _tbf = 0
#define CLRF_HALL				_int_pri1f = 0; _halaf = 0; _halbf = 0; _halcf = 0
#define CLRF_TM3				_int_pri14f = 0; _tm3af = 0

// timer reset
#define RST_TM0 				_pt0on = 0; _pt0on = 1
#define RST_TM1 				_pt1on = 0; _pt1on = 1
#define RST_TM3 				_pt3on = 0; _pt3on = 1

#define START_TM0(x)			_ptm0al = x; _ptm0ah = (x >> 8); _tm0af = 0; _pt0on = 0; _pt0on = 1

#define FLAG_TM0				_tm0af


#define PWM_ON					_pwmon				= 1


// Test pin 
#define TO0 					_pd2
#define TO1 					_pd3

#define TB1 					_pb1
#define TB2 					_pb2
#define TB0 					_pb0

// Function declaration
void Init_IO(void);
void Init_System(void);
void Init_PWM(void);
void Init_Comp(void);
void Init_CAPTM(void);
void Init_TM0(void);
void Init_TM1(void);
void Init_TM3(void);
void Init_Int(void);
void Init_ADC(void);
void Init_TimeBase(void);
void Init_Vars(void);

void delay_10u(uint16 x);
void delay1(uint8 n);
void delay_ms(uint16 x);
void delay_tm0(uint16 dly);
void delay_tm1(uint16 dly);

void ADC_Ch_Sel(uint8 ch);
void ADC_Trig(void);
void ADC_AC_CH(uint8 order, uint8 ch);

void PWM_Duty(uint16 duty);
void PWM_SET(uint8 mode, uint16 duty);

void TM0_Dly_Set(uint16 dly);
void TM3_Dly_Set(uint16 dly);

void Hall_Int_Set(uint8 step);
void Drag_Motor(void);
void Commutation(void);

#endif
