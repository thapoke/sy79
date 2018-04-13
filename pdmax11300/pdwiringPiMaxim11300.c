/// pdwiringPi.c : Pd external to communicate with hardware on a Raspberry Pi using wiringPi
/// Copyright (c) 2014, Garth Zeglin.  All rights reserved.  Provided under the
/// terms of the BSD 3-clause license.

/****************************************************************/
// Links to related reference documentation:

//   Pd externals:      http://pdstatic.iem.at/externals-HOWTO/node9.html
//   wiringPi:          http://wiringpi.com

/****************************************************************/ 
// import standard libc API
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>


#define PIXI_READ                  0x01
#define PIXI_WRITE                 0x00
// Register Table (each register is 16 bit wide)
#define PIXI_DEVICE_ID                  0x00
#define PIXI_INTERRUPT                  0x01
#define PIXI_ADC_DATA_STATUS_0_15       0x02
#define PIXI_ADC_DATA_STATUS_16_19      0x03
#define PIXI_OVERCURRENT_STATUS_0_15    0x04
#define PIXI_OVERCURRENT_STATUS_16_19   0x05
#define PIXI_GPI_STATUS_0_15            0x06
#define PIXI_GPI_STATUS_16_19           0x07
#define PIXI_INT_TEMP_DATA              0x08
#define PIXI_EXT1_TEMP_DATA             0x09
#define PIXI_EXT2_TEMP_DATA             0x0A
#define PIXI_GPI_DATA_0_15              0x0B
#define PIXI_GPI_DATA_16_19             0x0C
#define PIXI_GPO_DATA_0_15              0x0D
#define PIXI_GPO_DATA_16_19             0x0E
#define PIXI_DEVICE_CTRL                0x10
#define PIXI_INTERRUPT_MASK             0x11
#define PIXI_GPI_IRQ_MODE_0_7           0x12
#define PIXI_GPI_IRQ_MODE_8_15          0x13
#define PIXI_GPI_IRQ_MODE_16_19         0x14
#define PIXI_DAC_PRESET_DATA1           0x16
#define PIXI_DAC_PRESET_DATA2           0x17
#define PIXI_TEMP_MON_CONFIG            0x18
#define PIXI_TEMP_INT_HIGH_THRESHOLD    0x19
#define PIXI_TEMP_INT_LOW_THRESHOLD     0x1A
#define PIXI_TEMP_EXT1_HIGH_THRESHOLD   0x1B
#define PIXI_TEMP_EXT1_LOW_THRESHOLD    0x1C
#define PIXI_TEMP_EXT2_HIGH_THRESHOLD   0x1D
#define PIXI_TEMP_EXT2_LOW_THRESHOLD    0x1E

#define PIXI_PORT_CONFIG                0x20
#define PIXI_PORT0_CONFIG               0x20
#define PIXI_PORT1_CONFIG               0x21
#define PIXI_PORT2_CONFIG               0x22
#define PIXI_PORT3_CONFIG               0x23
#define PIXI_PORT4_CONFIG               0x24
#define PIXI_PORT5_CONFIG               0x25
#define PIXI_PORT6_CONFIG               0x26
#define PIXI_PORT7_CONFIG               0x27
#define PIXI_PORT8_CONFIG               0x28
#define PIXI_PORT9_CONFIG               0x29
#define PIXI_PORT10_CONFIG              0x2A
#define PIXI_PORT11_CONFIG              0x2B
#define PIXI_PORT12_CONFIG              0x2C
#define PIXI_PORT13_CONFIG              0x2D
#define PIXI_PORT14_CONFIG              0x2E
#define PIXI_PORT15_CONFIG              0x2F
#define PIXI_PORT16_CONFIG              0x30
#define PIXI_PORT17_CONFIG              0x31
#define PIXI_PORT18_CONFIG              0x32
#define PIXI_PORT19_CONFIG              0x33

#define PIXI_ADC_DATA                   0x40
#define PIXI_PORT0_ADC_DATA             0x40
#define PIXI_PORT1_ADC_DATA             0x41
#define PIXI_PORT2_ADC_DATA             0x42
#define PIXI_PORT3_ADC_DATA             0x43
#define PIXI_PORT4_ADC_DATA             0x44
#define PIXI_PORT5_ADC_DATA             0x45
#define PIXI_PORT6_ADC_DATA             0x46
#define PIXI_PORT7_ADC_DATA             0x47
#define PIXI_PORT8_ADC_DATA             0x48
#define PIXI_PORT9_ADC_DATA             0x49
#define PIXI_PORT10_ADC_DATA            0x4A
#define PIXI_PORT11_ADC_DATA            0x4B
#define PIXI_PORT12_ADC_DATA            0x4C
#define PIXI_PORT13_ADC_DATA            0x4D
#define PIXI_PORT14_ADC_DATA            0x4E
#define PIXI_PORT15_ADC_DATA            0x4F
#define PIXI_PORT16_ADC_DATA            0x50
#define PIXI_PORT17_ADC_DATA            0x51
#define PIXI_PORT18_ADC_DATA            0x52
#define PIXI_PORT19_ADC_DATA            0x53

#define PIXI_DAC_DATA                   0x60
#define PIXI_PORT0_DAC_DATA             0x60
#define PIXI_PORT1_DAC_DATA             0x61
#define PIXI_PORT2_DAC_DATA             0x62
#define PIXI_PORT3_DAC_DATA             0x63
#define PIXI_PORT4_DAC_DATA             0x64
#define PIXI_PORT5_DAC_DATA             0x65
#define PIXI_PORT6_DAC_DATA             0x66
#define PIXI_PORT7_DAC_DATA             0x67
#define PIXI_PORT8_DAC_DATA             0x68
#define PIXI_PORT9_DAC_DATA             0x69
#define PIXI_PORT10_DAC_DATA            0x6A
#define PIXI_PORT11_DAC_DATA            0x6B
#define PIXI_PORT12_DAC_DATA            0x6C
#define PIXI_PORT13_DAC_DATA            0x6D
#define PIXI_PORT14_DAC_DATA            0x6E
#define PIXI_PORT15_DAC_DATA            0x6F
#define PIXI_PORT16_DAC_DATA            0x70
#define PIXI_PORT17_DAC_DATA            0x71
#define PIXI_PORT18_DAC_DATA            0x72
#define PIXI_PORT19_DAC_DATA            0x73

// Detailed register content map
// reg 0x00 Device ID
#define DEVID           0xFFFF
// reg00x10 Device control
#define ADCCTL          0x0003
#define DACCTL    0x000C
#define ADCCONV 0x0030
#define DACREF    0x0040
#define THSHDN    0x0080
#define TMPCTL    0x0700
#define TMPCTLINT 0x0100
#define TMPCTLEXT1  0x0200
#define TMPCTLEXT2  0x0400
#define TMPPER    0x0800
#define RS_CANCEL 0x1000
#define LPEN    0x2000
#define BRST    0x4000
#define RESET   0x8000
//ADCCTL values
#define ADC_MODE_IDLE   0x0
#define ADC_MODE_SWEEP  0x1
#define ADC_MODE_CONV   0x2
#define ADC_MODE_CONT   0x3


// reg 0x18 Temperature monitor config
#define TMPINTMONCFG    0x0003
#define TMPEXT1MONCFG   0x000C
#define TMPEXT2MONCFG   0x0030
// reg 0x19-1E Temperature monitor threshold high and low
#define TMPINTHI        0x0FFF
#define TMPINTLO        0x0FFF
#define TMPEXT1HI       0x0FFF
#define TMPEXT1LO       0x0FFF
#define TMPEXT2HI       0x0FFF
#define TMPEXT2LO       0x0FFF

// reg 0x20-33 Port Configuration
#define FUNCPRM         0x0FFF
#define FUNCID          0xF000
// Port Configuration register bits
#define FUNCPRM_ASSOCIATED_PORT    0x001F
#define FUNCPRM_NR_OF_SAMPLES      0x00E0
#define FUNCPRM_RANGE              0x0700
#define FUNCPRM_AVR_INV            0x0800
#define FUNCID_MODE0_HIGHZ         0x0000



// reg 0x40-53  ADC data
#define ADCDAT          0x0FFF
// reg 0x60-73  DAC data
#define DACDAT          0x0FFF

// Channel placeholder
#define CHANNEL_0       0x00
#define CHANNEL_1       0x01
#define CHANNEL_2       0x02
#define CHANNEL_3       0x03
#define CHANNEL_4       0x04
#define CHANNEL_5       0x05
#define CHANNEL_6       0x06
#define CHANNEL_7       0x07
#define CHANNEL_8       0x08
#define CHANNEL_9       0x09
#define CHANNEL_10      0x0a
#define CHANNEL_11      0x0b
#define CHANNEL_12      0x0c
#define CHANNEL_13      0x0d
#define CHANNEL_14      0x0e
#define CHANNEL_15      0x0f
#define CHANNEL_16      0x10
#define CHANNEL_17      0x11
#define CHANNEL_18      0x12
#define CHANNEL_19      0x13

// Channel mode placeholder
#define CH_MODE_0               0x00
#define CH_MODE_HIZ             0x00
#define CH_MODE_1               0x01
#define CH_MODE_GPI             0x01
#define CH_MODE_2               0x02
#define CH_MODE_DIDIR_LT_TERM   0x02
#define CH_MODE_3               0x03
#define CH_MODE_GPO_REG         0x03
#define CH_MODE_4               0x04
#define CH_MODE_GPO_UNI         0x04
#define CH_MODE_5               0x05
#define CH_MODE_DAC             0x05
#define CH_MODE_6               0x06
#define CH_MODE_DAC_ADC_MON     0x06
#define CH_MODE_7               0x07
#define CH_MODE_ADC_P           0x07
#define CH_MODE_8               0x08
#define CH_MODE_ADC_DIFF_P      0x08
#define CH_MODE_9               0x09
#define CH_MODE_ADC_DIFF_N      0x09
#define CH_MODE_10              0x0a
#define CH_MODE_DAC_ADC_DIFF_N  0x0a
#define CH_MODE_11              0x0b
#define CH_MODE_TERM_GPI_SW     0x0b
#define CH_MODE_12              0x0c
#define CH_MODE_TERM_REG_SW     0x0c

// Channel range
#define CH_NO_RANGE              0x0000
#define CH_0_TO_10P              0x0001
#define CH_5N_TO_5P              0x0002
#define CH_10N_TO_0              0x0003
#define CH_0_TO_2P5_5N_TO_5P     0x0004
#define CH_RES                   0x0005
#define CH_0_TO_2P5_0_TO_10P     0x0006
#define CH_RES2                  0x0007


#define TEMP_CHANNEL_INT  0x0
#define TEMP_CHANNEL_EXT0 0x1
#define TEMP_CHANNEL_EXT1 0x2
uint8_t *txbuf;
uint8_t *rxbuf;

uint16_t info = 0;
uint16_t readx = 0;


#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif



// Import the API for Pd externals.  This is provided by the Pd installation,
// and this may require manually configuring the Pd include path in the
// Makefile.
#include "m_pd.h"

// Import the wiringPi API.
#include <wiringPi.h>
#include <wiringPiSPI.h>



/****************************************************************/ 
/// There is only one wiringPi interface, so the underlying
/// hardware state is stored globally to be referenced by any
/// wiringPi object.
static int sys_mode = 0;        ///< flag to indicate initialization with wiringPiSetupSys

/****************************************************************/ 
/// Data structure to hold the state of a single Pd 'wiringPi' object.
typedef struct pdwiringPi
{
  t_object x_ob;           ///< standard object header
  t_outlet *x_outlet;      ///< the outlet is the port on which return values are transmitted
  t_inlet *x_in2;  
  t_inlet *x_in3;

  int pin;                 ///< a non-negative pin number if initalized to control a single pin
  int mode;                ///< pin mode value if initialized to control a single pin

  int spi_channel;         ///< SPI channel number, or -1 if not in SPI mode
  int spi_speed;           ///< SPI speed in Hz, or -1 if not in SPI mode
  int spi_fd;              ///< SPI file descriptor, or -1 if not in SPI mode
  int value;              ///< spi value
  int spichan;              ///< spi chan



char* text ;

}



 t_pdwiringPi;

static t_class *pdwiringPi_class;

/****************************************************************/
// Utility functions.
static int atom_matches( t_atom *atom, char *symbol )
{
  return ( (atom->a_type == A_SYMBOL) 
	   && !strcmp( atom->a_w.w_symbol->s_name, symbol ) );
}

static inline int symbol_matches( t_symbol *sym, char *symbol )
{
  return !strcmp( sym->s_name, symbol );
}






/****************************************************************/
// Map a symbol representing a pin mode to the numeric constant.
static int atom_to_pin_mode( t_atom *s_mode )
{	
  int value = INPUT;
  if      ( atom_matches( s_mode, "output" ))     value = OUTPUT;
  else if ( atom_matches( s_mode, "pwm_output" )) value = PWM_OUTPUT;
  else if ( atom_matches( s_mode, "gpio_clock" )) value = GPIO_CLOCK;
  else if ( atom_matches( s_mode, "input" ))      value = INPUT;
  else post ("wiringPi: unrecognized pin mode symbol, assuming INPUT.");
  return value;
}




/****************************************************************/
// A bang is always interpreted as a type of read for pin-specific instances.
static void pdwiringPi_bang( t_pdwiringPi *x )
{
  if ( x->pin >= 0 ) {
    int value = digitalRead( x->pin );
    outlet_float( x->x_outlet, value );
    return;

  } else {
    post("wiringPi: bang not allowed for non-pin-specific instances.");
  }
}




/****************************************************************/
// A float is always interpreted as a type of write for pin-specific instances.
static void pdwiringPi_float( t_pdwiringPi *x, float value )
{
  // if a pin-specific instance
  if ( x->pin >= 0 ) {

    // pwm output mode
    if (x->mode == PWM_OUTPUT) {
      // "The Raspberry Pi has one on-board PWM pin, pin 1 (BMC_GPIO 18, Phys 12)
      // and the range is 0-1024."
      pwmWrite( x->pin, (int) value );
      if (sys_mode) post("wiringPi: Warning, pwmWrite has no effect in Sys mode.");
    }

    // else assume it is a digital output
    else {

      digitalWrite( x->pin, (int) value );
    }
  } else {
    post("wiringPi: float not allowed for non-pin-specific instances.");
  }
}




/****************************************************************/
// Abstract the difference between the Sys and Gpio initialization modes.
static void set_pin_mode( int pin, int mode )
{
  if (sys_mode) {
    if (mode == INPUT || mode == OUTPUT) {
      char *command;
      asprintf( &command, "gpio export %d %s", pin, (mode==INPUT) ? "in" : "out" );
      post("wiringPi in Sys mode: running '%s'", command );
      system(command);
      free(command);
    } else {
      post("wiringPi: error, cannot set mode %d on pin %d in Sys mode.", mode, pin );
    }
  } else {
    pinMode( pin, mode );
  }
}

















/****************************************************************/
void WriteRegister(uint8_t spichannel , uint8_t address, uint16_t value)
{
	txbuf[0] = ( (address) << 1) | PIXI_WRITE; //write
	txbuf[1] = (value) >> 8; //value H
	txbuf[2] = (value) & 0xFF; //valueL
		//post("wiringPi: writereg spichan %d address %d value %d buf[0] %d buf[1] %d buf[2] %d",spichannel, address, value, txbuf[0],txbuf[1],txbuf[2]);
	wiringPiSPIDataRW (spichannel, txbuf, 3);
}



/****************************************************************/
uint16_t ReadRegister(uint8_t spichannel  , uint8_t address, bool debug )
{
	uint16_t resultat = 0; 
	txbuf[0] = ( (address) << 1) | PIXI_READ; //write
	wiringPiSPIDataRW (spichannel, txbuf, 1);  
	resultat = txbuf[0] << 8 | txbuf[1];
		//post("wiringPi: readreg spichan %d address %d value %d",spichannel, address, resultat);
	return resultat;
}



/****************************************************************/
uint16_t ReadAnalog( uint8_t spichannel , uint8_t channel)
{
	uint16_t resultat = 0;
	txbuf[0] = ( (PIXI_ADC_DATA + channel) << 1) | PIXI_READ; //write
	wiringPiSPIDataRW(spichannel, txbuf, 1);

	resultat = txbuf[0] << 8 | txbuf[1];
		//post("wiringPi: readAnalog spichan %d chan %d value %d",spichannel, channel, resultat);

	return resultat;
}





















/****************************************************************/
uint16_t  configChannel0( uint8_t channel, uint8_t channel_mode,  uint16_t dac_dat,  uint16_t range, uint8_t adc_ctl,  uint8_t adc_smp )
{
  uint16_t result = 0;
  uint16_t info = 0;
post("wiringPi: configchannel" , channel,channel_mode);
  if ( ( channel <= 19 ) && ( channel_mode <= 12 ) )
  {

    if (channel_mode == CH_MODE_1  ||
        channel_mode == CH_MODE_3  ||
        channel_mode == CH_MODE_4  ||
        channel_mode == CH_MODE_5  ||
        channel_mode == CH_MODE_6  ||
        channel_mode == CH_MODE_10 )
    {
      // config DACREF (internal reference),DACCTL (sequential update)
      info = ReadRegister ( 0,  PIXI_DEVICE_CTRL, true );
      WriteRegister ( 0,PIXI_DEVICE_CTRL, info | DACREF | !DACCTL );
      //delay(1);
      info = ReadRegister ( 0,  PIXI_DEVICE_CTRL, true );
      // Enter DACDAT
      WriteRegister ( 0, PIXI_DAC_DATA + channel, dac_dat);
      // Mode1: config FUNCID, FUNCPRM (non-inverted default)
      if (channel_mode == CH_MODE_1)
      {
        WriteRegister (0, PIXI_PORT_CONFIG + channel, ( ( (CH_MODE_1 << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE ) ) );

      };
      //delay(1);
      // Mode3: config GPO_DAT, leave channel at logic level 0
      if (channel_mode == CH_MODE_3)
      {
        if ( channel <= 15 )
        {
          WriteRegister (0, PIXI_GPO_DATA_0_15, 0x00);
        }
        else if (channel >= 16 )
        {
          WriteRegister (0, PIXI_GPO_DATA_16_19, ( 0x00 )  );
        };
      }
      // Mode3,4,5,6,10: config FUNCID, FUNCPRM (non-inverted default)
      if (channel_mode == CH_MODE_3  ||
          channel_mode == CH_MODE_5  ||
          channel_mode == CH_MODE_6  ||
          channel_mode == CH_MODE_10)
      {
        WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE ) ) );

      }
      else if (channel_mode == CH_MODE_4  )
      {
        WriteRegister (0, PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE )
                        // assoc port & FUNCPRM_ASSOCIATED_PORT
                                                    ) );

      };
      //delay(1);
      // Mode1: config GPIMD (leave at default uint8_t never asserted
      if (channel_mode == CH_MODE_1)
      {
        //        WriteRegister ( 0,  PIXI_GPI_IRQ_MODE_0_7, 0 );

      };
      //delay(1);

    }

    else if (channel_mode == CH_MODE_7  ||
             channel_mode == CH_MODE_8  ||
             channel_mode == CH_MODE_9  ) {

      // Mode9: config FUNCID, FUNCPRM
      if (channel_mode == CH_MODE_9)
      {
        WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE )
                                                    ) );
      }
      //delay(1);
      if (channel_mode == CH_MODE_7  ||
          channel_mode == CH_MODE_8)
      {
        WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE | ((adc_smp<<5)&FUNCPRM_NR_OF_SAMPLES)
                                                   ) ) );
      }
      //delay(1);

      // config ADCCTL
      info = ReadRegister ( 0,  PIXI_DEVICE_CTRL, false );
      WriteRegister ( 0, PIXI_DEVICE_CTRL, info | ( adc_ctl & ADCCTL ) );
      //delay(1);

    }
    else if (channel_mode == CH_MODE_2  ||
             channel_mode == CH_MODE_11 ||
             channel_mode == CH_MODE_12 ) {

      WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                      ( (range << 8 ) & FUNCPRM_RANGE )
                                                  ) );



    };

  }

  return (result);
};




uint16_t  configChannel( uint8_t channel, uint8_t channel_mode,  uint16_t dac_dat,  uint16_t range, uint8_t adc_ctl )
{
  uint16_t result = 0;
  uint16_t info = 0;

  if ( ( channel <= 19 ) && ( channel_mode <= 12 ) )
  {

    if (channel_mode == CH_MODE_1  ||
        channel_mode == CH_MODE_3  ||
        channel_mode == CH_MODE_4  ||
        channel_mode == CH_MODE_5  ||
        channel_mode == CH_MODE_6  ||
        channel_mode == CH_MODE_10 )
    {
      // config DACREF (internal reference),DACCTL (sequential update)
      info = ReadRegister ( 0,  PIXI_DEVICE_CTRL, true );
      WriteRegister ( 0, PIXI_DEVICE_CTRL, info | DACREF | !DACCTL );
      //delay(1);
      info = ReadRegister ( 0,  PIXI_DEVICE_CTRL, true );
      // Enter DACDAT
      WriteRegister ( 0,  PIXI_DAC_DATA + channel, dac_dat);
      // Mode1: config FUNCID, FUNCPRM (non-inverted default)
      if (channel_mode == CH_MODE_1)
      {
        WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (CH_MODE_1 << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE ) ) );

      };
      //delay(1);
      // Mode3: config GPO_DAT, leave channel at logic level 0
      if (channel_mode == CH_MODE_3)
      {
        if ( channel <= 15 )
        {
          WriteRegister ( 0,  PIXI_GPO_DATA_0_15, 0x00);
        }
        else if (channel >= 16 )
        {
          WriteRegister ( 0,  PIXI_GPO_DATA_16_19, ( 0x00 )  );
        };
      }
      // Mode3,4,5,6,10: config FUNCID, FUNCPRM (non-inverted default)
      if (channel_mode == CH_MODE_3  ||
          channel_mode == CH_MODE_5  ||
          channel_mode == CH_MODE_6  ||
          channel_mode == CH_MODE_10)
      {
        WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE ) ) );

      }
      else if (channel_mode == CH_MODE_4  )
      {
        WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE )
                        // assoc port & FUNCPRM_ASSOCIATED_PORT
                                                    ) );

      };
      //delay(1);
      // Mode1: config GPIMD (leave at default uint8_t never asserted
      if (channel_mode == CH_MODE_1)
      {
        //        WriteRegister ( 0,  PIXI_GPI_IRQ_MODE_0_7, 0 );

      };
      //delay(1);

    }

    else if (channel_mode == CH_MODE_7  ||
             channel_mode == CH_MODE_8  ||
             channel_mode == CH_MODE_9  ) {

      // Mode9: config FUNCID, FUNCPRM
      if (channel_mode == CH_MODE_9)
      {
        WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE )
                                                    ) );
      }
      //delay(1);
      if (channel_mode == CH_MODE_7  ||
          channel_mode == CH_MODE_8)
      {
        WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                        ( (range << 8 ) & FUNCPRM_RANGE )
                                                    ) );
      }
      //delay(1);

      // config ADCCTL
      info = ReadRegister ( 0,  PIXI_DEVICE_CTRL, false );
      WriteRegister ( 0, PIXI_DEVICE_CTRL, info | ( adc_ctl & ADCCTL ) );
      //delay(1);

    }
    else if (channel_mode == CH_MODE_2  ||
             channel_mode == CH_MODE_11 ||
             channel_mode == CH_MODE_12 ) {

      WriteRegister ( 0,  PIXI_PORT_CONFIG + channel, ( ( (channel_mode << 12 ) & FUNCID ) |
                      ( (range << 8 ) & FUNCPRM_RANGE )
                                                  ) );

    };

  }
  return (result);
};
 







/****************************************************************/
uint8_t Maxconfig(void)
{
  uint16_t result = 0;
  uint16_t info = 0;
post("wiringPi: maxconfig");
  result = ReadRegister(0, PIXI_DEVICE_ID, true );

  if (result == 0x0424) {
    // enable default burst, thermal shutdown, leave conversion rate at 200k
    WriteRegister ( 0, PIXI_DEVICE_CTRL, !BRST | THSHDN ); // ADCCONV = 00 default.
    // enable internal temp sensor
    // disable series resistor cancelation
    info = ReadRegister ( 0,  PIXI_DEVICE_CTRL, false );
    WriteRegister ( 0, PIXI_DEVICE_CTRL, info | !RS_CANCEL );
    // keep TMPINTMONCFG at default 4 samples

    // Set int temp hi threshold
    WriteRegister ( 0, PIXI_TEMP_INT_HIGH_THRESHOLD, 0x0230 );    // 70 deg C in .125 steps
    // Keep int temp lo threshold at 0 deg C, negative values need function to write a two's complement number.
    // enable internal and both external temp sensors
    info = ReadRegister ( 0,  PIXI_DEVICE_CTRL, false );
    WriteRegister ( 0, PIXI_DEVICE_CTRL, info | TMPCTLINT | TMPCTLEXT1 | TMPCTLEXT2 );

  }


  return (result);

}













/****************************************************************/
void WriteAnalog(uint8_t spichannel, uint8_t channel, uint16_t value)
{

	txbuf[0] = ( (PIXI_DAC_DATA + channel)<<1)|PIXI_WRITE; 
			//post("wiringPi: awrite chan %d ", PIXI_DAC_DATA + channel<<1);
			//post("wiringPi: awrite buf1 %d ", txbuf[0]);
	txbuf[1] = value >> 8 ; //value H
			//post("wiringPi: awrite buf2 %d ", txbuf[1]);
	txbuf[2] = value & 0xFF; //valueL
			//post("wiringPi: awrite buf2 %d ", txbuf[2]);
	wiringPiSPIDataRW(spichannel, txbuf, 3);
	//post("wiringPi: analogWrite spichan %d channel %d value %d buf %d" ,spichannel , channel, value, txbuf);

}












/****************************************************************/
void setup() {
  static uint8_t _txbuf[8] __attribute__ ((section (".sram2")));
  static uint8_t _rxbuf[8] __attribute__ ((section (".sram2")));
  txbuf = _txbuf;
  rxbuf = _rxbuf;
  Maxconfig();
  configChannel( CHANNEL_0, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_1, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_2, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_3, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_4, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_5, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_6, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_7, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_8, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_9, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_10, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_11, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_12, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_13, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_14, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  configChannel( CHANNEL_15, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
 configChannel( CHANNEL_16, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
 configChannel( CHANNEL_17, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
 configChannel( CHANNEL_18, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
 configChannel( CHANNEL_19, CH_MODE_DAC, 0, CH_0_TO_10P, 0 );
  //configChannel( CHANNEL_7, CH_MODE_ADC_P, 0, CH_0_TO_10P, ADC_MODE_CONT,2 );
post("wiringPi: setupcomplete");


}


















/****************************************************************/
/// Process a list representing a function call or more elaborate I/O command.

static void pdwiringPi_eval( t_pdwiringPi *x, t_symbol *selector, int argcount, t_atom *argvec )
{
  // post ("pdwiringPi_eval called with %d args, selector %s\n", argcount, selector->s_name );

  if (x->pin >= 0) {
    post("wiringPi: general input not allowed for pin-specific instances.");
    return;
  }

  // test for a variety of function call forms
  if ( symbol_matches( selector, "digitalRead" ) && argcount == 1) {
    outlet_float( x->x_outlet, (float) digitalRead( atom_getint( &argvec[0] )));
    return;

  } else if ( symbol_matches( selector, "digitalWrite" ) && argcount == 2) {
    digitalWrite( atom_getint(&argvec[0]), atom_getint(&argvec[1]) );
    return;

  } else if ( symbol_matches( selector, "pwmWrite" ) && argcount == 2) {
    // "The Raspberry Pi has one on-board PWM pin, pin 1 (BMC_GPIO 18, Phys 12)
    // and the range is 0-1024."
    pwmWrite( atom_getint(&argvec[0]), atom_getint(&argvec[1]) );
    if (sys_mode) post("wiringPi: Warning, pwmWrite has no effect in Sys mode.");
    return;

  } else if ( symbol_matches( selector, "pinMode" ) && argcount == 2) {
    int mode = atom_to_pin_mode( &argvec[1] );
    set_pin_mode( atom_getint(&argvec[0]), mode );
    return;

  } else if ( symbol_matches( selector, "wpiPinToGpio" ) && argcount == 1) {
    outlet_float( x->x_outlet, (float) wpiPinToGpio( atom_getint(&argvec[0]) ));
    return;

  } else if ( symbol_matches( selector, "physPinToGpio" ) && argcount == 1) {
    outlet_float( x->x_outlet, (float) physPinToGpio( atom_getint(&argvec[0]) ));
    return;

  } else if ( symbol_matches( selector, "piBoardRev" ) && argcount == 0) {
    outlet_float( x->x_outlet, (float) piBoardRev() );
    return;






  } else if ( symbol_matches( selector, "load_spi_driver" ) && argcount == 0) {
    char *command = "gpio load spi";
    post("wiringPi: running '%s' to make sure SPI drivers are loaded.", command );
    system(command);
    return;








  } else if ( symbol_matches( selector, "spi_init" )) {
    // specialize an object instance to represent an SPI port
    //  [ spi_init <spi-number> <spi-speed> ]
    if (argcount == 2) {
		
		x->spi_channel = atom_getint( &argvec[0] );
		x->spi_speed   = atom_getint( &argvec[1] );
		x->spi_fd      = wiringPiSPISetup ( x->spi_channel, x->spi_speed);   
		setup();

      if (x->spi_fd == -1) {
	post("wiringPiSPISetup returned error %d.", errno );
      } else {
	post("wiringPi: opened SPI channel %d at speed %d. fd %d", x->spi_channel, x->spi_speed,x->spi_fd);
      }
    } else {
      post("wiringPi error: spi_init requires channel and speed values.");
    }




  } else if ( symbol_matches( selector, "spi_write" )) {
	  
    if (argcount == 3) {
		
		int spichannel = atom_getint( &argvec[0] );
		int channel = atom_getint( &argvec[1] );
		int value   = (atom_getint( &argvec[2]));
		WriteAnalog(spichannel,channel,value);
		//post("wiringPi : attempt write spichan %d chan %d val %d ", spichannel, channel, value);
		return;


    } else {
      post("wiringPi error: spi_init requires spi_channel , channel and cv values");
    }

  } else if ( symbol_matches( selector, "reboot" )) {
		system("reboot");
  } 

else {
    post("wiringPi: unrecognized input for selector %s.", selector->s_name );
  }
}










/****************************************************************/
/// Create an instance of a Pd 'wiringPi' object.
///
/// The creation arguments are all optional and are interpreted as follows:
///  [ wiringPi pin <pin-number> <mode-symbol> ] make instance pin-specific

static void *pdwiringPi_new(t_symbol *selector, int argcount, t_atom *argvec)
{
  t_pdwiringPi *x = (t_pdwiringPi *) pd_new(pdwiringPi_class);

  // initialize default values for a generic instance
  x->pin  = -1;
  x->mode = 0;

  x->spi_channel = -1;
  x->spi_speed   = -1;
  x->spi_fd      = -1;


 

  if (argcount > 0) {
    // check the initial creation argument
    t_atom *arg0 = &argvec[0];

    // define the object instance as 'pin-specific' representing a single pin
    if ( atom_matches( arg0, "pin" )) {
      // "Note that only wiringPi pin 1 (BCM_GPIO 18) supports PWM output and
      // only wiringPi pin 7 (BCM_GPIO 4) supports CLOCK output modes."

      if (argcount == 3 ) {
	x->pin  = atom_getint( &argvec[1] );
	x->mode = atom_to_pin_mode( &argvec[2] );
	set_pin_mode( x->pin, x->mode );
	// post("Initialized wiringPi object for pin %d in mode %d.", x->pin, x->mode );

      } else {
	post("wiringPi: incorrect number of creation arguments for pin.");
      }
    } else {
      post("wiringPi: unrecognized creation arguments.");
    }
  }

  // create an outlet on which to return values
  x->x_outlet = outlet_new( &x->x_ob, NULL );
	//create signal inlet
  x->x_in2 = inlet_new(&x->x_ob, &x->x_ob.ob_pd, &s_signal, &s_signal); 
  x->x_in3 = floatinlet_new (&x->x_ob, 0);
  return (void *)x;

}








/****************************************************************/
/// Release an instance of a Pd 'wiringPi' object.
static void pdwiringPi_free(t_pdwiringPi *x)
{
  if (x) {
    outlet_free( x->x_outlet );
	inlet_free(x->x_in2);  
	inlet_free(x->x_in3);   
    x->x_outlet = NULL;
  }
}










/****************************************************************/
/// Initialization entry point for the Pd 'wiringPi' external.  This is
/// automatically called by Pd after loading the dynamic module to initialize
/// the class interface.
void wiringPi_setup(void)
{
  // specify "A_GIMME" as creation argument for both the creation
  // routine and the method (callback) for the "eval" message.


;


  pdwiringPi_class = class_new( gensym("wiringPi"),              // t_symbol *name
				(t_newmethod) pdwiringPi_new,    // t_newmethod newmethod
				(t_method) pdwiringPi_free,      // t_method freemethod
				sizeof(t_pdwiringPi),            // size_t size
				0,                               // int flags
				A_GIMME, 0);                     // t_atomtype arg1, ...

  // instances specialized to specific pins can accept bangs and floats
  class_addbang ( pdwiringPi_class, pdwiringPi_bang );    // t_class *c, t_method fn
  class_addfloat( pdwiringPi_class, pdwiringPi_float );  // t_class *c, t_method fn

  // general inputs represent function calls and more elaborate I/O operations
  class_addanything( pdwiringPi_class, (t_method) pdwiringPi_eval );   // (t_class *c, t_method fn)

  // static initialization follows
  if (geteuid() == 0) {
    wiringPiSetupGpio();


    sys_mode = 0;
    post("Initializing wiringPi in Gpio mode to use direct hardware access.\nUsing Broadcom GPIO pin numbering scheme.");

  } else {
    wiringPiSetupSys();    
    sys_mode = 1;


    post("Warning: initializing wiringPi in Sys mode to use /sys/class/gpio interface.\nFor a faster interface, relaunch pd as root.\nUsing Broadcom GPIO pin numbering scheme.");
  }




}

/****************************************************************/
