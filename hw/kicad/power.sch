EESchema Schematic File Version 2
LIBS:74xgxx
LIBS:74xx
LIBS:ac-dc
LIBS:actel
LIBS:adc-dac
LIBS:Altera
LIBS:analog_devices
LIBS:analog_switches
LIBS:atmel
LIBS:audio
LIBS:brooktre
LIBS:cmos_ieee
LIBS:cmos4000
LIBS:conn
LIBS:contrib
LIBS:cypress
LIBS:dc-dc
LIBS:device
LIBS:digital-audio
LIBS:display
LIBS:dsp
LIBS:elec-unifil
LIBS:ESD_Protection
LIBS:ftdi
LIBS:gennum
LIBS:graphic
LIBS:hc11
LIBS:intel
LIBS:interface
LIBS:ir
LIBS:Lattice
LIBS:linear
LIBS:logo
LIBS:maxim
LIBS:memory
LIBS:microchip_dspic33dsc
LIBS:microchip_pic10mcu
LIBS:microchip_pic12mcu
LIBS:microchip_pic16mcu
LIBS:microchip_pic18mcu
LIBS:microchip_pic32mcu
LIBS:microchip
LIBS:microcontrollers
LIBS:motor_drivers
LIBS:motorola
LIBS:msp430
LIBS:nordicsemi
LIBS:nxp_armmcu
LIBS:onsemi
LIBS:opto
LIBS:Oscillators
LIBS:philips
LIBS:Power_Management
LIBS:power
LIBS:powerint
LIBS:pspice
LIBS:references
LIBS:regul
LIBS:relays
LIBS:rfcom
LIBS:sensors
LIBS:silabs
LIBS:siliconi
LIBS:stm8
LIBS:stm32
LIBS:supertex
LIBS:switches
LIBS:texas
LIBS:transf
LIBS:transistors
LIBS:ttl_ieee
LIBS:valves
LIBS:video
LIBS:Xicor
LIBS:xilinx
LIBS:Zilog
LIBS:drv8301
LIBS:MotCtrl-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L DRV8301 U?
U 1 1 58153B9C
P 3450 2450
F 0 "U?" H 3450 3950 60  0000 C CNN
F 1 "DRV8301" H 3750 900 60  0000 C CNN
F 2 "footprints:TSSOP-56-PP" H 4100 800 60  0001 C CNN
F 3 "" H 3450 2450 60  0000 C CNN
	1    3450 2450
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR?
U 1 1 5815B3DF
P 4500 1800
F 0 "#PWR?" H 4500 1650 50  0001 C CNN
F 1 "+3.3V" H 4500 1940 50  0000 C CNN
F 2 "" H 4500 1800 60  0000 C CNN
F 3 "" H 4500 1800 60  0000 C CNN
	1    4500 1800
	1    0    0    -1  
$EndComp
Text HLabel 2550 2100 0    60   Input ~ 0
SPI_CLK
Text HLabel 2550 1900 0    60   Input ~ 0
SPI_MOSI
Text HLabel 2550 2000 0    60   Output ~ 0
SPI_MISO
Text HLabel 2550 1800 0    60   Input ~ 0
SPI_CS
$Comp
L VPP #PWR?
U 1 1 5815B5FB
P 4950 950
F 0 "#PWR?" H 4950 800 50  0001 C CNN
F 1 "VPP" H 4950 1100 50  0000 C CNN
F 2 "" H 4950 950 60  0000 C CNN
F 3 "" H 4950 950 60  0000 C CNN
	1    4950 950 
	1    0    0    -1  
$EndComp
$Comp
L C C3
U 1 1 5815B645
P 5150 1100
F 0 "C3" V 5000 1050 50  0000 L CNN
F 1 "100n" V 5100 1150 50  0000 L CNN
F 2 "" H 5188 950 30  0000 C CNN
F 3 "" H 5150 1100 60  0000 C CNN
	1    5150 1100
	0    1    1    0   
$EndComp
$Comp
L C C2
U 1 1 5815B675
P 5150 1300
F 0 "C2" V 5300 1250 50  0000 L CNN
F 1 "10u" V 5200 1350 50  0000 L CNN
F 2 "" H 5188 1150 30  0000 C CNN
F 3 "" H 5150 1300 60  0000 C CNN
	1    5150 1300
	0    1    1    0   
$EndComp
$Comp
L GND #PWR?
U 1 1 5815B9EC
P 5400 1300
F 0 "#PWR?" H 5400 1050 50  0001 C CNN
F 1 "GND" H 5400 1150 50  0000 C CNN
F 2 "" H 5400 1300 60  0000 C CNN
F 3 "" H 5400 1300 60  0000 C CNN
	1    5400 1300
	0    -1   -1   0   
$EndComp
$Comp
L C C?
U 1 1 5815BBC3
P 4500 1100
F 0 "C?" V 4650 1050 50  0000 L CNN
F 1 "10n" V 4550 900 50  0000 L CNN
F 2 "" H 4538 950 30  0000 C CNN
F 3 "" H 4500 1100 60  0000 C CNN
	1    4500 1100
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 5815BC1C
P 4650 1100
F 0 "#PWR?" H 4650 850 50  0001 C CNN
F 1 "GND" H 4650 950 50  0000 C CNN
F 2 "" H 4650 1100 60  0000 C CNN
F 3 "" H 4650 1100 60  0000 C CNN
	1    4650 1100
	0    -1   -1   0   
$EndComp
$Comp
L C C?
U 1 1 5815BE53
P 4500 1500
F 0 "C?" V 4350 1450 50  0000 L CNN
F 1 "100n" V 4450 1550 50  0000 L CNN
F 2 "" H 4538 1350 30  0000 C CNN
F 3 "" H 4500 1500 60  0000 C CNN
	1    4500 1500
	0    1    1    0   
$EndComp
$Comp
L INDUCTOR L?
U 1 1 5815C032
P 6200 1600
F 0 "L?" V 6150 1600 50  0000 C CNN
F 1 "22u" V 6300 1600 50  0000 C CNN
F 2 "" H 6200 1600 60  0000 C CNN
F 3 "" H 6200 1600 60  0000 C CNN
	1    6200 1600
	0    -1   -1   0   
$EndComp
$Comp
L D_Schottky D?
U 1 1 5815C0E1
P 5800 1800
F 0 "D?" H 5800 1900 50  0000 C CNN
F 1 "D_Schottky" H 5800 1700 50  0000 C CNN
F 2 "" H 5800 1800 60  0000 C CNN
F 3 "" H 5800 1800 60  0000 C CNN
	1    5800 1800
	0    1    1    0   
$EndComp
$Comp
L CP C?
U 1 1 5815C260
P 6650 1800
F 0 "C?" H 6675 1900 50  0000 L CNN
F 1 "47u" H 6675 1700 50  0000 L CNN
F 2 "" H 6688 1650 30  0000 C CNN
F 3 "" H 6650 1800 60  0000 C CNN
	1    6650 1800
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 5815C46B
P 900 1650
F 0 "R2" V 980 1650 50  0000 C CNN
F 1 "10k" V 900 1650 50  0000 C CNN
F 2 "" V 830 1650 30  0000 C CNN
F 3 "" H 900 1650 30  0000 C CNN
F 4 "1%" V 800 1650 60  0000 C CNN " Tolerance"
	1    900  1650
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR?
U 1 1 5815C63D
P 900 1800
F 0 "#PWR?" H 900 1550 50  0001 C CNN
F 1 "GND" H 900 1650 50  0000 C CNN
F 2 "" H 900 1800 60  0000 C CNN
F 3 "" H 900 1800 60  0000 C CNN
	1    900  1800
	1    0    0    -1  
$EndComp
Text Notes 1000 1500 0    31   ~ 0
Vout = 0.8(R1/R2 + 1)
$Comp
L R R1
U 1 1 5815CB9A
P 900 1250
F 0 "R1" V 980 1250 50  0000 C CNN
F 1 "31k6" V 900 1250 50  0000 C CNN
F 2 "" V 830 1250 30  0000 C CNN
F 3 "" H 900 1250 30  0000 C CNN
F 4 "1%" V 800 1250 60  0000 C CNN " Tolerance"
	1    900  1250
	-1   0    0    1   
$EndComp
$Comp
L R R?
U 1 1 5815CF97
P 2500 1100
F 0 "R?" V 2600 1100 50  0000 C CNN
F 1 "200k" V 2500 1100 50  0000 C CNN
F 2 "" V 2430 1100 30  0000 C CNN
F 3 "" H 2500 1100 30  0000 C CNN
	1    2500 1100
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 5815D02C
P 2350 1100
F 0 "#PWR?" H 2350 850 50  0001 C CNN
F 1 "GND" H 2350 950 50  0000 C CNN
F 2 "" H 2350 1100 60  0000 C CNN
F 3 "" H 2350 1100 60  0000 C CNN
	1    2350 1100
	0    1    1    0   
$EndComp
$Comp
L R R?
U 1 1 5815DA21
P 1750 1200
F 0 "R?" V 1830 1200 50  0000 C CNN
F 1 "16k2" V 1750 1200 50  0000 C CNN
F 2 "" V 1680 1200 30  0000 C CNN
F 3 "" H 1750 1200 30  0000 C CNN
	1    1750 1200
	0    1    1    0   
$EndComp
$Comp
L C C?
U 1 1 5815DAE6
P 1550 950
F 0 "C?" V 1700 900 50  0000 L CNN
F 1 "120p" V 1600 700 50  0000 L CNN
F 2 "" H 1588 800 30  0000 C CNN
F 3 "" H 1550 950 60  0000 C CNN
	1    1550 950 
	0    -1   -1   0   
$EndComp
$Comp
L C C?
U 1 1 5815DBA0
P 1350 1200
F 0 "C?" V 1500 1150 50  0000 L CNN
F 1 "6n8" V 1400 1000 50  0000 L CNN
F 2 "" H 1388 1050 30  0000 C CNN
F 3 "" H 1350 1200 60  0000 C CNN
	1    1350 1200
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 5815E11F
P 1100 1250
F 0 "#PWR?" H 1100 1000 50  0001 C CNN
F 1 "GND" H 1100 1100 50  0000 C CNN
F 2 "" H 1100 1250 60  0000 C CNN
F 3 "" H 1100 1250 60  0000 C CNN
	1    1100 1250
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR?
U 1 1 5815E769
P 6650 1400
F 0 "#PWR?" H 6650 1250 50  0001 C CNN
F 1 "+3.3V" H 6650 1540 50  0000 C CNN
F 2 "" H 6650 1400 60  0000 C CNN
F 3 "" H 6650 1400 60  0000 C CNN
	1    6650 1400
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR?
U 1 1 5815E7DD
P 900 900
F 0 "#PWR?" H 900 750 50  0001 C CNN
F 1 "+3.3V" H 900 1040 50  0000 C CNN
F 2 "" H 900 900 60  0000 C CNN
F 3 "" H 900 900 60  0000 C CNN
	1    900  900 
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 5815F177
P 2350 2400
F 0 "C1" V 2300 2450 50  0000 L CNN
F 1 "22n" V 2300 2200 50  0000 L CNN
F 2 "" H 2388 2250 30  0000 C CNN
F 3 "" H 2350 2400 60  0000 C CNN
	1    2350 2400
	0    1    1    0   
$EndComp
Text Notes 6700 750  0    60   ~ 0
VPP-rated components:\nC1, C2, C3, C4, C5, C6, C7, C8, C9
$Comp
L C C?
U 1 1 5815F5EC
P 1750 2300
F 0 "C?" V 1600 2250 50  0000 L CNN
F 1 "2u2/16V" V 1700 2350 50  0000 L CNN
F 2 "" H 1788 2150 30  0000 C CNN
F 3 "" H 1750 2300 60  0000 C CNN
	1    1750 2300
	0    1    1    0   
$EndComp
$Comp
L GND #PWR?
U 1 1 5815F822
P 1500 2300
F 0 "#PWR?" H 1500 2050 50  0001 C CNN
F 1 "GND" H 1500 2150 50  0000 C CNN
F 2 "" H 1500 2300 60  0000 C CNN
F 3 "" H 1500 2300 60  0000 C CNN
	1    1500 2300
	0    1    1    0   
$EndComp
$Comp
L C C?
U 1 1 5815FA03
P 1750 3700
F 0 "C?" V 1600 3650 50  0000 L CNN
F 1 "1u/10V" V 1700 3750 50  0000 L CNN
F 2 "" H 1788 3550 30  0000 C CNN
F 3 "" H 1750 3700 60  0000 C CNN
	1    1750 3700
	0    1    1    0   
$EndComp
$Comp
L GNDA #PWR?
U 1 1 5815FAFA
P 1500 3700
F 0 "#PWR?" H 1500 3450 50  0001 C CNN
F 1 "GNDA" H 1500 3550 50  0000 C CNN
F 2 "" H 1500 3700 60  0000 C CNN
F 3 "" H 1500 3700 60  0000 C CNN
	1    1500 3700
	0    1    1    0   
$EndComp
$Comp
L C C?
U 1 1 5815FC5F
P 1750 3300
F 0 "C?" V 1600 3250 50  0000 L CNN
F 1 "1u/10V" V 1700 3350 50  0000 L CNN
F 2 "" H 1788 3150 30  0000 C CNN
F 3 "" H 1750 3300 60  0000 C CNN
	1    1750 3300
	0    1    1    0   
$EndComp
$Comp
L GNDA #PWR?
U 1 1 5815FD30
P 1500 3300
F 0 "#PWR?" H 1500 3050 50  0001 C CNN
F 1 "GNDA" H 1500 3150 50  0000 C CNN
F 2 "" H 1500 3300 60  0000 C CNN
F 3 "" H 1500 3300 60  0000 C CNN
	1    1500 3300
	0    1    1    0   
$EndComp
$Comp
L C C5
U 1 1 58160B47
P 4750 4000
F 0 "C5" H 4650 4100 50  0000 L CNN
F 1 "10u" H 4600 3900 50  0000 L CNN
F 2 "" H 4788 3850 30  0000 C CNN
F 3 "" H 4750 4000 60  0000 C CNN
	1    4750 4000
	-1   0    0    1   
$EndComp
$Comp
L C C4
U 1 1 58160BC2
P 4500 4000
F 0 "C4" H 4400 4100 50  0000 L CNN
F 1 "100n" H 4300 3900 50  0000 L CNN
F 2 "" H 4538 3850 30  0000 C CNN
F 3 "" H 4500 4000 60  0000 C CNN
	1    4500 4000
	-1   0    0    1   
$EndComp
$Comp
L VPP #PWR?
U 1 1 58160E98
P 5000 3750
F 0 "#PWR?" H 5000 3600 50  0001 C CNN
F 1 "VPP" H 5000 3900 50  0000 C CNN
F 2 "" H 5000 3750 60  0000 C CNN
F 3 "" H 5000 3750 60  0000 C CNN
	1    5000 3750
	1    0    0    -1  
$EndComp
$Comp
L C C6
U 1 1 581617CE
P 5000 4000
F 0 "C6" H 4900 4100 50  0000 L CNN
F 1 "10u" H 4850 3900 50  0000 L CNN
F 2 "" H 5038 3850 30  0000 C CNN
F 3 "" H 5000 4000 60  0000 C CNN
	1    5000 4000
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR?
U 1 1 5816270C
P 6650 2050
F 0 "#PWR?" H 6650 1800 50  0001 C CNN
F 1 "GND" H 6650 1900 50  0000 C CNN
F 2 "" H 6650 2050 60  0000 C CNN
F 3 "" H 6650 2050 60  0000 C CNN
	1    6650 2050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 58162FB5
P 3450 4250
F 0 "#PWR?" H 3450 4000 50  0001 C CNN
F 1 "GND" H 3450 4100 50  0000 C CNN
F 2 "" H 3450 4250 60  0000 C CNN
F 3 "" H 3450 4250 60  0000 C CNN
	1    3450 4250
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 5816345A
P 5000 1900
F 0 "C?" V 4850 1850 50  0000 L CNN
F 1 "100n/16V" V 4950 1950 50  0000 L CNN
F 2 "" H 5038 1750 30  0000 C CNN
F 3 "" H 5000 1900 60  0000 C CNN
	1    5000 1900
	0    1    1    0   
$EndComp
$Comp
L C C?
U 1 1 581638E7
P 5000 2400
F 0 "C?" V 4850 2350 50  0000 L CNN
F 1 "100n/16V" V 4950 2450 50  0000 L CNN
F 2 "" H 5038 2250 30  0000 C CNN
F 3 "" H 5000 2400 60  0000 C CNN
	1    5000 2400
	0    1    1    0   
$EndComp
$Comp
L C C?
U 1 1 58163942
P 5000 2900
F 0 "C?" V 4850 2850 50  0000 L CNN
F 1 "100n/16V" V 4950 2950 50  0000 L CNN
F 2 "" H 5038 2750 30  0000 C CNN
F 3 "" H 5000 2900 60  0000 C CNN
	1    5000 2900
	0    1    1    0   
$EndComp
Text Label 4800 2000 2    60   ~ 0
GH_A
Text Label 4800 2500 2    60   ~ 0
GH_B
Text Label 4800 3000 2    60   ~ 0
GH_C
Text Label 4800 2200 2    60   ~ 0
GL_A
Text Label 4800 2300 2    60   ~ 0
SL_A
Text Label 4800 2700 2    60   ~ 0
GL_B
Text Label 4800 2800 2    60   ~ 0
SL_B
Text Label 4800 3200 2    60   ~ 0
GL_C
Text Label 4800 3300 2    60   ~ 0
SL_C
Text Label 4800 3500 2    60   ~ 0
I_SENS_P_A
Text Label 4800 3400 2    60   ~ 0
I_SENS_N_A
Text Label 4800 3600 2    60   ~ 0
I_SENS_N_B
Text Label 4800 3700 2    60   ~ 0
I_SENS_P_B
Text Label 4800 2100 2    60   ~ 0
SH_A
Text Label 4800 2600 2    60   ~ 0
SH_B
Text Label 4800 3100 2    60   ~ 0
SH_C
Text HLabel 2550 2700 0    60   Input ~ 0
PWM_H_A
Text HLabel 2550 2800 0    60   Input ~ 0
PWM_L_A
Text HLabel 2550 2900 0    60   Input ~ 0
PWM_H_B
Text HLabel 2550 3000 0    60   Input ~ 0
PWM_L_B
Text HLabel 2550 3100 0    60   Input ~ 0
PWM_H_C
Text HLabel 2550 3200 0    60   Input ~ 0
PWM_L_C
Text HLabel 2550 2600 0    60   Input ~ 0
EN_GATE
Text HLabel 2550 1500 0    60   Output ~ 0
nOCTW
Text HLabel 2550 1600 0    60   Output ~ 0
nFAULT
$Comp
L R R?
U 1 1 5816895E
P 1750 1700
F 0 "R?" V 1850 1700 50  0000 C CNN
F 1 "0" V 1750 1700 50  0000 C CNN
F 2 "" V 1680 1700 30  0000 C CNN
F 3 "" H 1750 1700 30  0000 C CNN
	1    1750 1700
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 58168A72
P 1500 1700
F 0 "#PWR?" H 1500 1450 50  0001 C CNN
F 1 "GND" H 1500 1550 50  0000 C CNN
F 2 "" H 1500 1700 60  0000 C CNN
F 3 "" H 1500 1700 60  0000 C CNN
	1    1500 1700
	0    1    1    0   
$EndComp
Text HLabel 2550 2200 0    60   Input ~ 0
DC_CAL
Text HLabel 2550 3400 0    60   Input ~ 0
I_SENS_V_REF
Text HLabel 2550 3500 0    60   Output ~ 0
I_SENS_A
Text HLabel 2550 3600 0    60   Output ~ 0
I_SENS_B
$Comp
L GNDA #PWR?
U 1 1 581BA227
P 5700 3900
F 0 "#PWR?" H 5700 3650 50  0001 C CNN
F 1 "GNDA" H 5700 3750 50  0000 C CNN
F 2 "" H 5700 3900 60  0000 C CNN
F 3 "" H 5700 3900 60  0000 C CNN
	1    5700 3900
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 581BA296
P 5950 3900
F 0 "#PWR?" H 5950 3650 50  0001 C CNN
F 1 "GND" H 5950 3750 50  0000 C CNN
F 2 "" H 5950 3900 60  0000 C CNN
F 3 "" H 5950 3900 60  0000 C CNN
	1    5950 3900
	1    0    0    -1  
$EndComp
$Comp
L Q_NMOS_SGD Q?
U 1 1 581CC953
P 2100 5400
F 0 "Q?" H 2050 5550 50  0000 R CNN
F 1 "CSD17573Q5B" V 2350 5650 50  0000 R CNN
F 2 "SON_8_5x6:SON_8_5x6" H 2300 5500 29  0001 C CNN
F 3 "" H 2100 5400 60  0000 C CNN
	1    2100 5400
	1    0    0    -1  
$EndComp
$Comp
L Q_NMOS_SGD Q?
U 1 1 581CD36C
P 2100 6150
F 0 "Q?" H 2050 6300 50  0000 R CNN
F 1 "CSD17573Q5B" V 2350 6400 50  0000 R CNN
F 2 "SON_8_5x6:SON_8_5x6" H 2300 6250 29  0001 C CNN
F 3 "" H 2100 6150 60  0000 C CNN
	1    2100 6150
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 581CE0CC
P 1650 5400
F 0 "R?" V 1750 5400 50  0000 C CNN
F 1 "4R7" V 1650 5400 50  0000 C CNN
F 2 "" V 1580 5400 30  0000 C CNN
F 3 "" H 1650 5400 30  0000 C CNN
	1    1650 5400
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 581CEC1E
P 6200 7150
F 0 "#PWR?" H 6200 6900 50  0001 C CNN
F 1 "GND" H 6200 7000 50  0000 C CNN
F 2 "" H 6200 7150 60  0000 C CNN
F 3 "" H 6200 7150 60  0000 C CNN
	1    6200 7150
	1    0    0    -1  
$EndComp
$Comp
L VPP #PWR?
U 1 1 581CF0F5
P 2200 4700
F 0 "#PWR?" H 2200 4550 50  0001 C CNN
F 1 "VPP" H 2200 4850 50  0000 C CNN
F 2 "" H 2200 4700 60  0000 C CNN
F 3 "" H 2200 4700 60  0000 C CNN
	1    2200 4700
	1    0    0    -1  
$EndComp
$Comp
L C C7
U 1 1 581CF47B
P 2550 5050
F 0 "C7" H 2650 5100 50  0000 L CNN
F 1 "10u" H 2650 5000 50  0000 L CNN
F 2 "" H 2588 4900 30  0000 C CNN
F 3 "" H 2550 5050 60  0000 C CNN
	1    2550 5050
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 581CF845
P 2550 5200
F 0 "#PWR?" H 2550 4950 50  0001 C CNN
F 1 "GND" H 2550 5050 50  0000 C CNN
F 2 "" H 2550 5200 60  0000 C CNN
F 3 "" H 2550 5200 60  0000 C CNN
	1    2550 5200
	1    0    0    -1  
$EndComp
Text Label 1800 5750 0    60   ~ 0
SH_A
Text Label 1150 5400 0    60   ~ 0
GH_A
Text Label 1150 6150 0    60   ~ 0
GL_A
Text Label 1800 6600 0    60   ~ 0
SL_A
$Comp
L Q_NMOS_SGD Q?
U 1 1 581D16E0
P 4100 5400
F 0 "Q?" H 4050 5550 50  0000 R CNN
F 1 "CSD17573Q5B" V 4350 5650 50  0000 R CNN
F 2 "SON_8_5x6:SON_8_5x6" H 4300 5500 29  0001 C CNN
F 3 "" H 4100 5400 60  0000 C CNN
	1    4100 5400
	1    0    0    -1  
$EndComp
$Comp
L Q_NMOS_SGD Q?
U 1 1 581D16E7
P 4100 6150
F 0 "Q?" H 4050 6300 50  0000 R CNN
F 1 "CSD17573Q5B" V 4350 6400 50  0000 R CNN
F 2 "SON_8_5x6:SON_8_5x6" H 4300 6250 29  0001 C CNN
F 3 "" H 4100 6150 60  0000 C CNN
	1    4100 6150
	1    0    0    -1  
$EndComp
$Comp
L VPP #PWR?
U 1 1 581D16FF
P 4200 4700
F 0 "#PWR?" H 4200 4550 50  0001 C CNN
F 1 "VPP" H 4200 4850 50  0000 C CNN
F 2 "" H 4200 4700 60  0000 C CNN
F 3 "" H 4200 4700 60  0000 C CNN
	1    4200 4700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 581D170C
P 4550 5200
F 0 "#PWR?" H 4550 4950 50  0001 C CNN
F 1 "GND" H 4550 5050 50  0000 C CNN
F 2 "" H 4550 5200 60  0000 C CNN
F 3 "" H 4550 5200 60  0000 C CNN
	1    4550 5200
	1    0    0    -1  
$EndComp
Text Label 3800 5750 0    60   ~ 0
SH_B
Text Label 3150 5400 0    60   ~ 0
GH_B
Text Label 3150 6150 0    60   ~ 0
GL_B
Text Label 3800 6600 0    60   ~ 0
SL_B
$Comp
L Q_NMOS_SGD Q?
U 1 1 581D23F2
P 6100 5400
F 0 "Q?" H 6050 5550 50  0000 R CNN
F 1 "CSD17573Q5B" V 6350 5650 50  0000 R CNN
F 2 "SON_8_5x6:SON_8_5x6" H 6300 5500 29  0001 C CNN
F 3 "" H 6100 5400 60  0000 C CNN
	1    6100 5400
	1    0    0    -1  
$EndComp
$Comp
L Q_NMOS_SGD Q?
U 1 1 581D23F9
P 6100 6150
F 0 "Q?" H 6050 6300 50  0000 R CNN
F 1 "CSD17573Q5B" V 6350 6400 50  0000 R CNN
F 2 "SON_8_5x6:SON_8_5x6" H 6300 6250 29  0001 C CNN
F 3 "" H 6100 6150 60  0000 C CNN
	1    6100 6150
	1    0    0    -1  
$EndComp
$Comp
L VPP #PWR?
U 1 1 581D2411
P 6200 4700
F 0 "#PWR?" H 6200 4550 50  0001 C CNN
F 1 "VPP" H 6200 4850 50  0000 C CNN
F 2 "" H 6200 4700 60  0000 C CNN
F 3 "" H 6200 4700 60  0000 C CNN
	1    6200 4700
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 581D241E
P 6550 5200
F 0 "#PWR?" H 6550 4950 50  0001 C CNN
F 1 "GND" H 6550 5050 50  0000 C CNN
F 2 "" H 6550 5200 60  0000 C CNN
F 3 "" H 6550 5200 60  0000 C CNN
	1    6550 5200
	1    0    0    -1  
$EndComp
Text Label 5800 5750 0    60   ~ 0
SH_C
Text Label 5150 5400 0    60   ~ 0
GH_C
Text Label 5150 6150 0    60   ~ 0
GL_C
Text Label 5800 6600 0    60   ~ 0
SL_C
$Comp
L R R?
U 1 1 581D304F
P 8400 1350
F 0 "R?" V 8500 1350 50  0000 C CNN
F 1 "?" V 8400 1350 50  0000 C CNN
F 2 "" V 8330 1350 30  0000 C CNN
F 3 "" H 8400 1350 30  0000 C CNN
	1    8400 1350
	0    1    1    0   
$EndComp
$Comp
L R R?
U 1 1 581D3318
P 8700 1650
F 0 "R?" H 8800 1650 50  0000 C CNN
F 1 "?" V 8700 1650 50  0000 C CNN
F 2 "" V 8630 1650 30  0000 C CNN
F 3 "" H 8700 1650 30  0000 C CNN
	1    8700 1650
	1    0    0    -1  
$EndComp
$Comp
L C C8
U 1 1 581D3E2D
P 4550 5050
F 0 "C8" H 4650 5100 50  0000 L CNN
F 1 "10u" H 4650 5000 50  0000 L CNN
F 2 "" H 4588 4900 30  0000 C CNN
F 3 "" H 4550 5050 60  0000 C CNN
	1    4550 5050
	1    0    0    -1  
$EndComp
$Comp
L C C9
U 1 1 581D4009
P 6550 5050
F 0 "C9" H 6650 5100 50  0000 L CNN
F 1 "10u" H 6650 5000 50  0000 L CNN
F 2 "" H 6588 4900 30  0000 C CNN
F 3 "" H 6550 5050 60  0000 C CNN
	1    6550 5050
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 581D41DD
P 9000 1650
F 0 "C?" H 9100 1700 50  0000 L CNN
F 1 "?" H 9100 1600 50  0000 L CNN
F 2 "" H 9038 1500 30  0000 C CNN
F 3 "" H 9000 1650 60  0000 C CNN
	1    9000 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 2100 2550 2100
Wire Wire Line
	2550 1900 2650 1900
Wire Wire Line
	2650 2000 2550 2000
Wire Wire Line
	2550 1800 2650 1800
Wire Wire Line
	4250 1400 4350 1400
Wire Wire Line
	4350 1400 4350 1300
Connection ~ 4350 1300
Wire Wire Line
	4250 1300 5000 1300
Wire Wire Line
	5300 1300 5400 1300
Wire Wire Line
	5350 1300 5350 1100
Wire Wire Line
	5350 1100 5300 1100
Connection ~ 5350 1300
Wire Wire Line
	5000 1100 4950 1100
Wire Wire Line
	4950 950  4950 1300
Connection ~ 4950 1300
Connection ~ 4950 1100
Wire Wire Line
	4350 1100 4250 1100
Wire Wire Line
	4350 1500 4250 1500
Wire Wire Line
	4250 1700 4350 1700
Wire Wire Line
	4350 1700 4350 1600
Wire Wire Line
	4250 1600 5900 1600
Wire Wire Line
	4950 1600 4950 1500
Wire Wire Line
	4950 1500 4650 1500
Connection ~ 4350 1600
Connection ~ 4950 1600
Wire Wire Line
	5800 1650 5800 1600
Connection ~ 5800 1600
Wire Wire Line
	6500 1600 6650 1600
Wire Wire Line
	6650 1400 6650 1650
Connection ~ 6650 1600
Wire Wire Line
	5800 1950 5800 2000
Wire Wire Line
	5800 2000 6650 2000
Wire Wire Line
	6650 1950 6650 2050
Connection ~ 900  1450
Wire Wire Line
	900  1400 900  1500
Wire Wire Line
	1700 950  2000 950 
Wire Wire Line
	1900 1200 2650 1200
Connection ~ 2000 1200
Wire Wire Line
	1600 1200 1500 1200
Wire Wire Line
	1200 1200 1100 1200
Wire Wire Line
	1100 950  1100 1250
Wire Wire Line
	1100 950  1400 950 
Wire Wire Line
	2000 950  2000 1200
Connection ~ 1100 1200
Wire Wire Line
	900  1450 2000 1450
Wire Wire Line
	2000 1450 2000 1300
Wire Wire Line
	2000 1300 2650 1300
Wire Wire Line
	900  900  900  1100
Wire Wire Line
	2500 2400 2650 2400
Wire Wire Line
	2150 2500 2650 2500
Wire Wire Line
	1900 2300 2650 2300
Wire Wire Line
	1900 3300 2650 3300
Wire Wire Line
	1900 3700 2650 3700
Wire Wire Line
	1500 3300 1600 3300
Wire Wire Line
	1500 3700 1600 3700
Wire Wire Line
	1550 3800 2650 3800
Wire Wire Line
	1550 3800 1550 3700
Connection ~ 1550 3700
Wire Wire Line
	4250 3800 5000 3800
Wire Wire Line
	4500 3850 4500 3800
Connection ~ 4500 3800
Wire Wire Line
	4500 4200 4500 4150
Wire Wire Line
	3450 4200 5000 4200
Wire Wire Line
	4750 4200 4750 4150
Connection ~ 4750 3800
Wire Wire Line
	5000 3750 5000 3850
Connection ~ 4750 4200
Connection ~ 5000 3800
Wire Wire Line
	4750 3850 4750 3800
Connection ~ 6650 2000
Wire Wire Line
	4500 1800 4250 1800
Wire Wire Line
	3450 4250 3450 4200
Wire Wire Line
	5300 2100 5300 1900
Wire Wire Line
	5300 1900 5150 1900
Wire Wire Line
	5300 2600 5300 2400
Wire Wire Line
	5300 2400 5150 2400
Wire Wire Line
	5300 3100 5300 2900
Wire Wire Line
	5300 2900 5150 2900
Wire Wire Line
	4250 2000 4800 2000
Wire Wire Line
	4250 3000 4800 3000
Wire Wire Line
	4250 2500 4800 2500
Wire Wire Line
	4250 2200 4800 2200
Wire Wire Line
	4250 2300 4800 2300
Wire Wire Line
	4250 2700 4800 2700
Wire Wire Line
	4250 2800 4800 2800
Wire Wire Line
	4250 3200 4800 3200
Wire Wire Line
	4250 3300 4800 3300
Wire Wire Line
	4800 3400 4250 3400
Wire Wire Line
	4250 3500 4800 3500
Wire Wire Line
	4800 3600 4250 3600
Wire Wire Line
	4250 3700 4800 3700
Wire Wire Line
	5300 2100 4250 2100
Wire Wire Line
	4250 1900 4850 1900
Wire Wire Line
	4250 2400 4850 2400
Wire Wire Line
	4250 2600 5300 2600
Wire Wire Line
	4250 2900 4850 2900
Wire Wire Line
	4250 3100 5300 3100
Wire Wire Line
	2550 3200 2650 3200
Wire Wire Line
	2550 3100 2650 3100
Wire Wire Line
	2550 3000 2650 3000
Wire Wire Line
	2550 2900 2650 2900
Wire Wire Line
	2550 2800 2650 2800
Wire Wire Line
	2550 2700 2650 2700
Wire Wire Line
	2550 2600 2650 2600
Wire Wire Line
	1500 2300 1600 2300
Connection ~ 4500 4200
Connection ~ 3450 4200
Wire Wire Line
	5000 4200 5000 4150
Wire Wire Line
	2550 1600 2650 1600
Wire Wire Line
	2650 1500 2550 1500
Wire Wire Line
	1900 1700 2650 1700
Wire Wire Line
	1500 1700 1600 1700
Wire Wire Line
	2150 2500 2150 2400
Wire Wire Line
	2150 2400 2200 2400
Wire Wire Line
	2550 2200 2650 2200
Wire Wire Line
	2550 3400 2650 3400
Wire Wire Line
	2550 3500 2650 3500
Wire Wire Line
	2650 3600 2550 3600
Wire Wire Line
	5950 3900 5950 3800
Wire Wire Line
	5700 3800 6200 3800
Wire Wire Line
	5700 3800 5700 3900
Wire Wire Line
	1800 5400 1900 5400
Wire Wire Line
	2200 5950 2200 5600
Wire Wire Line
	1800 6150 1900 6150
Wire Wire Line
	2200 4700 2200 5200
Wire Wire Line
	2550 4900 2550 4800
Wire Wire Line
	2550 4800 2200 4800
Connection ~ 2200 4800
Wire Wire Line
	2200 5750 1800 5750
Connection ~ 2200 5750
Wire Wire Line
	1500 5400 1150 5400
Wire Wire Line
	1500 6150 1150 6150
Wire Wire Line
	2200 6350 2200 6600
Wire Wire Line
	2200 6600 1800 6600
Wire Wire Line
	3800 5400 3900 5400
Wire Wire Line
	4200 5950 4200 5600
Wire Wire Line
	3800 6150 3900 6150
Wire Wire Line
	4200 4700 4200 5200
Wire Wire Line
	4550 4900 4550 4800
Wire Wire Line
	4550 4800 4200 4800
Connection ~ 4200 4800
Wire Wire Line
	4200 5750 3800 5750
Connection ~ 4200 5750
Wire Wire Line
	3500 5400 3150 5400
Wire Wire Line
	3500 6150 3150 6150
Wire Wire Line
	4200 6350 4200 6600
Wire Wire Line
	4200 6600 3800 6600
Wire Wire Line
	5800 5400 5900 5400
Wire Wire Line
	6200 5950 6200 5600
Wire Wire Line
	5800 6150 5900 6150
Wire Wire Line
	6200 4700 6200 5200
Wire Wire Line
	6550 4900 6550 4800
Wire Wire Line
	6550 4800 6200 4800
Connection ~ 6200 4800
Wire Wire Line
	6200 5750 5800 5750
Connection ~ 6200 5750
Wire Wire Line
	5500 5400 5150 5400
Wire Wire Line
	5500 6150 5150 6150
Wire Wire Line
	6200 6350 6200 7150
Wire Wire Line
	6200 6600 5800 6600
Connection ~ 6200 6600
Wire Wire Line
	8550 1350 9300 1350
Wire Wire Line
	9000 1350 9000 1500
Wire Wire Line
	8700 1500 8700 1350
Connection ~ 8700 1350
$Comp
L GND #PWR?
U 1 1 581D58E3
P 8850 2000
F 0 "#PWR?" H 8850 1750 50  0001 C CNN
F 1 "GND" H 8850 1850 50  0000 C CNN
F 2 "" H 8850 2000 60  0000 C CNN
F 3 "" H 8850 2000 60  0000 C CNN
	1    8850 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 1800 8700 1900
Wire Wire Line
	8700 1900 9000 1900
Wire Wire Line
	8850 1900 8850 2000
Wire Wire Line
	9000 1900 9000 1800
Connection ~ 8850 1900
Text Label 7900 1350 0    60   ~ 0
SH_A
Wire Wire Line
	8250 1350 7900 1350
Text HLabel 9300 1350 2    60   Output ~ 0
V_EMF_SENSE_A
Connection ~ 9000 1350
$Comp
L R R?
U 1 1 581D79FC
P 8400 2250
F 0 "R?" V 8500 2250 50  0000 C CNN
F 1 "?" V 8400 2250 50  0000 C CNN
F 2 "" V 8330 2250 30  0000 C CNN
F 3 "" H 8400 2250 30  0000 C CNN
	1    8400 2250
	0    1    1    0   
$EndComp
$Comp
L R R?
U 1 1 581D7A03
P 8700 2550
F 0 "R?" H 8800 2550 50  0000 C CNN
F 1 "?" V 8700 2550 50  0000 C CNN
F 2 "" V 8630 2550 30  0000 C CNN
F 3 "" H 8700 2550 30  0000 C CNN
	1    8700 2550
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 581D7A0A
P 9000 2550
F 0 "C?" H 9100 2600 50  0000 L CNN
F 1 "?" H 9100 2500 50  0000 L CNN
F 2 "" H 9038 2400 30  0000 C CNN
F 3 "" H 9000 2550 60  0000 C CNN
	1    9000 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	8550 2250 9300 2250
Wire Wire Line
	9000 2250 9000 2400
Wire Wire Line
	8700 2400 8700 2250
Connection ~ 8700 2250
$Comp
L GND #PWR?
U 1 1 581D7A15
P 8850 2900
F 0 "#PWR?" H 8850 2650 50  0001 C CNN
F 1 "GND" H 8850 2750 50  0000 C CNN
F 2 "" H 8850 2900 60  0000 C CNN
F 3 "" H 8850 2900 60  0000 C CNN
	1    8850 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 2700 8700 2800
Wire Wire Line
	8700 2800 9000 2800
Wire Wire Line
	8850 2800 8850 2900
Wire Wire Line
	9000 2800 9000 2700
Connection ~ 8850 2800
Text Label 7900 2250 0    60   ~ 0
SH_B
Wire Wire Line
	8250 2250 7900 2250
Text HLabel 9300 2250 2    60   Output ~ 0
V_EMF_SENSE_B
Connection ~ 9000 2250
$Comp
L R R?
U 1 1 581D7AEB
P 8400 3150
F 0 "R?" V 8500 3150 50  0000 C CNN
F 1 "?" V 8400 3150 50  0000 C CNN
F 2 "" V 8330 3150 30  0000 C CNN
F 3 "" H 8400 3150 30  0000 C CNN
	1    8400 3150
	0    1    1    0   
$EndComp
$Comp
L R R?
U 1 1 581D7AF2
P 8700 3450
F 0 "R?" H 8800 3450 50  0000 C CNN
F 1 "?" V 8700 3450 50  0000 C CNN
F 2 "" V 8630 3450 30  0000 C CNN
F 3 "" H 8700 3450 30  0000 C CNN
	1    8700 3450
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 581D7AF9
P 9000 3450
F 0 "C?" H 9100 3500 50  0000 L CNN
F 1 "?" H 9100 3400 50  0000 L CNN
F 2 "" H 9038 3300 30  0000 C CNN
F 3 "" H 9000 3450 60  0000 C CNN
	1    9000 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	8550 3150 9300 3150
Wire Wire Line
	9000 3150 9000 3300
Wire Wire Line
	8700 3300 8700 3150
Connection ~ 8700 3150
$Comp
L GND #PWR?
U 1 1 581D7B04
P 8850 3800
F 0 "#PWR?" H 8850 3550 50  0001 C CNN
F 1 "GND" H 8850 3650 50  0000 C CNN
F 2 "" H 8850 3800 60  0000 C CNN
F 3 "" H 8850 3800 60  0000 C CNN
	1    8850 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	8700 3600 8700 3700
Wire Wire Line
	8700 3700 9000 3700
Wire Wire Line
	8850 3700 8850 3800
Wire Wire Line
	9000 3700 9000 3600
Connection ~ 8850 3700
Text Label 7900 3150 0    60   ~ 0
SH_C
Wire Wire Line
	8250 3150 7900 3150
Text HLabel 9300 3150 2    60   Output ~ 0
V_EMF_SENSE_C
Connection ~ 9000 3150
$Comp
L GNDD #PWR?
U 1 1 581DFDC9
P 6200 3900
F 0 "#PWR?" H 6200 3650 50  0001 C CNN
F 1 "GNDD" H 6200 3750 50  0000 C CNN
F 2 "" H 6200 3900 60  0000 C CNN
F 3 "" H 6200 3900 60  0000 C CNN
	1    6200 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 3800 6200 3900
Connection ~ 5950 3800
$Comp
L R R?
U 1 1 581E16C9
P 1650 6150
F 0 "R?" V 1750 6150 50  0000 C CNN
F 1 "4R7" V 1650 6150 50  0000 C CNN
F 2 "" V 1580 6150 30  0000 C CNN
F 3 "" H 1650 6150 30  0000 C CNN
	1    1650 6150
	0    -1   -1   0   
$EndComp
$Comp
L R R?
U 1 1 581E176E
P 3650 5400
F 0 "R?" V 3750 5400 50  0000 C CNN
F 1 "4R7" V 3650 5400 50  0000 C CNN
F 2 "" V 3580 5400 30  0000 C CNN
F 3 "" H 3650 5400 30  0000 C CNN
	1    3650 5400
	0    -1   -1   0   
$EndComp
$Comp
L R R?
U 1 1 581E182B
P 3650 6150
F 0 "R?" V 3750 6150 50  0000 C CNN
F 1 "4R7" V 3650 6150 50  0000 C CNN
F 2 "" V 3580 6150 30  0000 C CNN
F 3 "" H 3650 6150 30  0000 C CNN
	1    3650 6150
	0    -1   -1   0   
$EndComp
$Comp
L R R?
U 1 1 581E18D5
P 5650 5400
F 0 "R?" V 5750 5400 50  0000 C CNN
F 1 "4R7" V 5650 5400 50  0000 C CNN
F 2 "" V 5580 5400 30  0000 C CNN
F 3 "" H 5650 5400 30  0000 C CNN
	1    5650 5400
	0    -1   -1   0   
$EndComp
$Comp
L R R?
U 1 1 581E1995
P 5650 6150
F 0 "R?" V 5750 6150 50  0000 C CNN
F 1 "4R7" V 5650 6150 50  0000 C CNN
F 2 "" V 5580 6150 30  0000 C CNN
F 3 "" H 5650 6150 30  0000 C CNN
	1    5650 6150
	0    -1   -1   0   
$EndComp
$EndSCHEMATC
