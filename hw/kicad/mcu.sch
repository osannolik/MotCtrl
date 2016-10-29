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
Sheet 2 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	7100 2300 6950 2300
Wire Wire Line
	7100 2400 7100 2300
$Comp
L GND #PWR?
U 1 1 58154129
P 7100 2400
F 0 "#PWR?" H 7100 2150 50  0001 C CNN
F 1 "GND" H 7100 2250 50  0000 C CNN
F 2 "" H 7100 2400 60  0000 C CNN
F 3 "" H 7100 2400 60  0000 C CNN
	1    7100 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 4550 7550 4500
Wire Wire Line
	7550 4550 8550 4550
Wire Wire Line
	7550 4150 7550 4200
Wire Wire Line
	7550 4150 8550 4150
Connection ~ 7100 4650
Wire Wire Line
	6950 4650 7100 4650
Connection ~ 7100 4750
Wire Wire Line
	6950 4750 7100 4750
Connection ~ 7100 4850
Wire Wire Line
	6950 4850 7100 4850
Wire Wire Line
	7100 4550 7100 4950
Wire Wire Line
	6950 4550 7100 4550
$Comp
L GND #PWR?
U 1 1 5815413B
P 7100 4950
F 0 "#PWR?" H 7100 4700 50  0001 C CNN
F 1 "GND" H 7100 4800 50  0000 C CNN
F 2 "" H 7100 4950 60  0000 C CNN
F 3 "" H 7100 4950 60  0000 C CNN
	1    7100 4950
	1    0    0    -1  
$EndComp
Connection ~ 7100 3800
$Comp
L VDD #PWR?
U 1 1 58154142
P 7100 3700
F 0 "#PWR?" H 7100 3550 50  0001 C CNN
F 1 "VDD" H 7100 3850 50  0000 C CNN
F 2 "" H 7100 3700 60  0000 C CNN
F 3 "" H 7100 3700 60  0000 C CNN
	1    7100 3700
	1    0    0    -1  
$EndComp
Connection ~ 7100 3900
Wire Wire Line
	6950 3900 7100 3900
Connection ~ 7100 4000
Wire Wire Line
	6950 4000 7100 4000
Connection ~ 7100 4100
Wire Wire Line
	6950 4100 7100 4100
Wire Wire Line
	7100 4200 6950 4200
Wire Wire Line
	7100 3700 7100 4200
Wire Wire Line
	6950 3800 7100 3800
Connection ~ 8550 4150
Connection ~ 8550 4550
Connection ~ 8300 4150
Wire Wire Line
	8300 4200 8300 4150
Connection ~ 8300 4550
Wire Wire Line
	8300 4500 8300 4550
Connection ~ 8050 4550
Wire Wire Line
	8050 4500 8050 4550
Connection ~ 8050 4150
Wire Wire Line
	8050 4200 8050 4150
Connection ~ 7800 4150
Wire Wire Line
	7800 4200 7800 4150
Connection ~ 7800 4550
Wire Wire Line
	7800 4500 7800 4550
Wire Wire Line
	8550 4500 8550 4650
Wire Wire Line
	8550 4050 8550 4200
$Comp
L C C?
U 1 1 58154161
P 8550 4350
F 0 "C?" H 8575 4450 50  0000 L CNN
F 1 "4u7" H 8575 4250 50  0000 L CNN
F 2 "" H 8588 4200 30  0000 C CNN
F 3 "" H 8550 4350 60  0000 C CNN
	1    8550 4350
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 58154168
P 8300 4350
F 0 "C?" H 8325 4450 50  0000 L CNN
F 1 "100n" H 8325 4250 50  0000 L CNN
F 2 "" H 8338 4200 30  0000 C CNN
F 3 "" H 8300 4350 60  0000 C CNN
	1    8300 4350
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 5815416F
P 8050 4350
F 0 "C?" H 8075 4450 50  0000 L CNN
F 1 "100n" H 8075 4250 50  0000 L CNN
F 2 "" H 8088 4200 30  0000 C CNN
F 3 "" H 8050 4350 60  0000 C CNN
	1    8050 4350
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 58154176
P 7800 4350
F 0 "C?" H 7825 4450 50  0000 L CNN
F 1 "100n" H 7825 4250 50  0000 L CNN
F 2 "" H 7838 4200 30  0000 C CNN
F 3 "" H 7800 4350 60  0000 C CNN
	1    7800 4350
	1    0    0    -1  
$EndComp
$Comp
L VDD #PWR?
U 1 1 5815417D
P 8550 4050
F 0 "#PWR?" H 8550 3900 50  0001 C CNN
F 1 "VDD" H 8550 4200 50  0000 C CNN
F 2 "" H 8550 4050 60  0000 C CNN
F 3 "" H 8550 4050 60  0000 C CNN
	1    8550 4050
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 58154183
P 7550 4350
F 0 "C?" H 7575 4450 50  0000 L CNN
F 1 "100n" H 7575 4250 50  0000 L CNN
F 2 "" H 7588 4200 30  0000 C CNN
F 3 "" H 7550 4350 60  0000 C CNN
	1    7550 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 5600 6950 5600
Wire Wire Line
	7100 5650 7100 5600
$Comp
L GND #PWR?
U 1 1 5815418C
P 7100 5950
F 0 "#PWR?" H 7100 5700 50  0001 C CNN
F 1 "GND" H 7100 5800 50  0000 C CNN
F 2 "" H 7100 5950 60  0000 C CNN
F 3 "" H 7100 5950 60  0000 C CNN
	1    7100 5950
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 58154192
P 7100 5800
F 0 "C?" H 7125 5900 50  0000 L CNN
F 1 "2u2" H 7125 5700 50  0000 L CNN
F 2 "" H 7138 5650 30  0000 C CNN
F 3 "" H 7100 5800 60  0000 C CNN
	1    7100 5800
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 58154199
P 8550 4650
F 0 "#PWR?" H 8550 4400 50  0001 C CNN
F 1 "GND" H 8550 4500 50  0000 C CNN
F 2 "" H 8550 4650 60  0000 C CNN
F 3 "" H 8550 4650 60  0000 C CNN
	1    8550 4650
	1    0    0    -1  
$EndComp
$Comp
L STM32F446R(C-E)Tx U?
U 1 1 5815419F
P 5200 3500
F 0 "U?" H 5200 3600 50  0000 C CNN
F 1 "STM32F446R(C-E)Tx" H 5200 3400 50  0000 C CNN
F 2 "LQFP64" H 5200 3300 50  0000 C CIN
F 3 "" H 5200 3500 50  0000 C CNN
	1    5200 3500
	1    0    0    -1  
$EndComp
$EndSCHEMATC
