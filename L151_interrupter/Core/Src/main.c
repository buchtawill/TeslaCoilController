/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "W25X40CL.h"
#include "usbd_cdc_if.h"
#include "string.h"
#include "Timers.h"
#include "I2C_LCD.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

/* USER CODE BEGIN PV */
//W25X40CL flash;
LCD lcd;
uint64_t micros = 0;
int16_t rotaryVal = 0, prevRotaryVal = 0;
//char  USB_Rx_Buf[256], USB_Tx_Buf[256];
uint16_t adcVal;

// onTime[0] is for the left fiber optic, onTime[1] is for the right fiber optic
uint8_t onTime[2] = {40, 40};
uint8_t prevOnTime[2] = {0, 0};
uint32_t time1 = 0, time2 = 0;
uint32_t diff = 0;
uint8_t dirPos = 0;

volatile uint16_t adc_dma_results[2];
const int         adcChannelCount = sizeof(adc_dma_results) / sizeof(adc_dma_results[0]);
volatile boolean  adc_conv_complete = false; //set by callback

boolean buttonPushed = false;

uint16_t noteFreq[60] = {
		//c,      c#,     d,      d#,     e,      f,      f#,     G,      g#,     a,      a#,     b
		 33,      35,     37,     39,     41,     44,     46,     49,     52,     55,     58,     61, //1's
		 65,      69,     73,     78,     82,     87,     93,     98,     104,    110,    117,    123, //2's
		 131,     139,    147,    156,    165,    175,    185,    196,    208,    220,    233,    247, //3's
		 262,     277,    294,    311,    330,    349,    370,    392,    415,    440,    466,    494, //4's
		 523,     554,    587,    622,    659,    698,    740,    784,    831,    880,    932,    988 //5's
};

FATFS fs;
FIL fil;
FRESULT fresult;
FILINFO fno;
DIR dir;
uint32_t time = 0, timeStarted = 0, adcTime = 0;
uint8_t midiBuf[6], numEventsSplit[2];
uint16_t numEvents, eventCounter = 0;
UINT bytesRead = 0;
boolean firstOpenDir = true;
uint8_t state = MODE_SELECT;
uint8_t modeNum = 1;
uint8_t songNum = 0;
boolean isDirOpen = false;
boolean isPlaying = false;
boolean printed = false;
uint8_t scrollPosition = 0;
uint16_t frequency = 10, prevFrequency = 10;
uint16_t t_on = MAX_TIME_ON;
uint16_t t_off = MAX_TIME_OFF;
boolean inCycle = false;
uint8_t digit_mode = 0;
uint8_t digit = 0;
uint8_t field_select = 0;
uint8_t submode = -1;
boolean inSubmode = false;
boolean updateTimeScroll = true;
uint32_t timeForScroll = 0;
int fileCount = 0;
char fileNames[MAX_FILE_LENGTH][MAX_FILENAME_LENGTH];
char displayedText[MAX_CHAR_ON_SCREEN];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM11_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

//could turn these into a struct, but ugh
boolean coil1On = false, coil2On = false, coil3On = false, coil4On = false,
		coil5On = false;
uint16_t coil1Freq, coil2Freq, coil3Freq, coil4Freq, coil5Freq;
//TODO: remake this ungodly function
/**
 * @brief	given a track, set the corresponding timer's (coil's) frequency and pulse width
 * 			Will bump notes played at the same time down to the next coil. ex if coil1 is playing
 * 			and two other notes come on simultaneously, they'll be pushed to coils 2 and 3. If all
 * 			5 coils are on, the note will not play
 * 			To turn off a coil, this function will find whichever coil is playing that frequency
 */
void setTimersAccordingly(uint8_t coil, uint16_t freq, uint8_t velocity) {
	//if velocity is 0, turn off that coil
	//if velocity is not 0, push the note down the chain
	//scaling not yet implemented
	if (velocity != 0) {
		switch (coil) {
		case 1:
			if (!coil1On) {
				setTimerFrequencyPulseWidth(&COIL1, freq, velocity, COIL1_CH);
				coil1On = true;
				coil1Freq = freq;
			} else
				setTimersAccordingly(2, freq, velocity);
			break;
		case 2:
			if (!coil2On) {
				setTimerFrequencyPulseWidth(&COIL2, freq, velocity, COIL2_CH);
				coil2On = true;
				coil2Freq = freq;
			} else
				setTimersAccordingly(3, freq, velocity);
			break;
		case 3:
			if (!coil3On) {
				setTimerFrequencyPulseWidth(&COIL3, freq, velocity, COIL3_CH);
				coil3On = true;
				coil3Freq = freq;
			} else
				setTimersAccordingly(4, freq, velocity);
			break;
		case 4:
			if (!coil4On) {
				setTimerFrequencyPulseWidth(&COIL4, freq, velocity, COIL4_CH);
				coil4On = true;
				coil4Freq = freq;
			} else
				setTimersAccordingly(5, freq, velocity);
			break;
		case 5:
			if (!coil5On) {
				setTimerFrequencyPulseWidth(&COIL5, freq, velocity, COIL5_CH);
				coil5On = true;
				coil5Freq = freq;
			}
			//uncommenting the else statement below will create a loop
			//else setTimersAccordingly(1, freq, velocity);
			break;
		}
	}			//end if (velocity != 0)
	else if (velocity == 0) {
		switch (coil) {
		case 1:
			if (coil1On && coil1Freq == freq) {
				setTimerFrequencyPulseWidth(&COIL1, 0, 0, COIL1_CH);
				coil1On = false;
				coil1Freq = 0;
			} else
				setTimersAccordingly(2, freq, 0);
			break;
		case 2:
			if (coil2On && coil2Freq == freq) {
				setTimerFrequencyPulseWidth(&COIL2, 0, 0, COIL2_CH);
				coil2On = false;
				coil2Freq = 0;
			} else
				setTimersAccordingly(3, freq, 0);
			break;
		case 3:
			if (coil3On && coil3Freq == freq) {
				setTimerFrequencyPulseWidth(&COIL3, 0, 0, COIL3_CH);
				coil3On = false;
				coil3Freq = 0;
			} else
				setTimersAccordingly(4, freq, 0);
			break;
		case 4:
			if (coil4On && coil4Freq == freq) {
				setTimerFrequencyPulseWidth(&COIL4, 0, 0, COIL4_CH);
				coil4On = false;
				coil4Freq = 0;
			} else
				setTimersAccordingly(5, freq, 0);
			break;
		case 5:
			if (coil5On && coil5Freq == freq) {
				setTimerFrequencyPulseWidth(&COIL5, 0, 0, COIL5_CH);
				coil5On = false;
				coil5Freq = 0;
			} else {	//I could have it wrap around, but no chances!!
				setTimerFrequencyPulseWidth(&COIL1, 0, 0, COIL1_CH);
				setTimerFrequencyPulseWidth(&COIL2, 0, 0, COIL2_CH);
				setTimerFrequencyPulseWidth(&COIL3, 0, 0, COIL3_CH);
				setTimerFrequencyPulseWidth(&COIL4, 0, 0, COIL4_CH);
				setTimerFrequencyPulseWidth(&COIL5, 0, 0, COIL5_CH);
			}
			break;
		} //end of switch
	} //end if velocity == 0
}

TIM_HandleTypeDef *findTimForThisCombo(uint8_t track, uint16_t freq, uint8_t velocity){
	//if velocity is 0, turn off that coil
	//if velocity is not 0, push the note down the chain
	//scaling not yet implemented
	if (velocity != 0) {
		switch (track) {
		case 1:
			if (!coil1On) {
				//setTimerFrequencyPulseWidth(&COIL1, freq, velocity, COIL1_CH);
				coil1On = true;
				coil1Freq = freq;
				return &COIL1;
			} else
				findTimForThisCombo(2, freq, velocity);
			break;
		case 2:
			if (!coil2On) {
				//setTimerFrequencyPulseWidth(&COIL2, freq, velocity, COIL2_CH);
				coil2On = true;
				coil2Freq = freq;
				return &COIL2;
			} else
				findTimForThisCombo(3, freq, velocity);
			break;
		case 3:
			if (!coil3On) {
//				setTimerFrequencyPulseWidth(&COIL3, freq, velocity, COIL3_CH);
				coil3On = true;
				coil3Freq = freq;
				return &COIL3;
			} else
				findTimForThisCombo(4, freq, velocity);
			break;
		case 4:
			if (!coil4On) {
//				setTimerFrequencyPulseWidth(&COIL4, freq, velocity, COIL4_CH);
				coil4On = true;
				coil4Freq = freq;
				return &COIL4;
			} else
				findTimForThisCombo(5, freq, velocity);
			break;
		case 5:
			if (!coil5On) {
//				setTimerFrequencyPulseWidth(&COIL5, freq, velocity, COIL5_CH);
				coil5On = true;
				coil5Freq = freq;
				return &COIL5;
			}
			break;
		}
	}			//end if (velocity != 0)
	else if (velocity == 0) {
		switch (track) {
		case 1:
			if (coil1On && coil1Freq == freq) {
				//setTimerFrequencyPulseWidth(&COIL1, 0, 0, COIL1_CH);
				coil1On = false;
				coil1Freq = 0;
				return &COIL1;
			} else
				findTimForThisCombo(2, freq, 0);
			break;
		case 2:
			if (coil2On && coil2Freq == freq) {
//				setTimerFrequencyPulseWidth(&COIL2, 0, 0, COIL2_CH);
				coil2On = false;
				coil2Freq = 0;
				return &COIL2;
			} else
				findTimForThisCombo(3, freq, 0);
			break;
		case 3:
			if (coil3On && coil3Freq == freq) {
//				setTimerFrequencyPulseWidth(&COIL3, 0, 0, COIL3_CH);
				coil3On = false;
				coil3Freq = 0;
				return &COIL3;
			} else
				findTimForThisCombo(4, freq, 0);
			break;
		case 4:
			if (coil4On && coil4Freq == freq) {
//				setTimerFrequencyPulseWidth(&COIL4, 0, 0, COIL4_CH);
				coil4On = false;
				coil4Freq = 0;
				return &COIL4;
			} else
				findTimForThisCombo(5, freq, 0);
			break;
		case 5:
			if (coil5On && coil5Freq == freq) {
//				setTimerFrequencyPulseWidth(&COIL5, 0, 0, COIL5_CH);
				coil5On = false;
				coil5Freq = 0;
				return &COIL4;
			} else {	//I could have it wrap around, but no chances!!
//				setTimerFrequencyPulseWidth(&COIL1, 0, 0, COIL1_CH);
//				setTimerFrequencyPulseWidth(&COIL2, 0, 0, COIL2_CH);
//				setTimerFrequencyPulseWidth(&COIL3, 0, 0, COIL3_CH);
//				setTimerFrequencyPulseWidth(&COIL4, 0, 0, COIL4_CH);
//				setTimerFrequencyPulseWidth(&COIL5, 0, 0, COIL5_CH);
			}
			break;
		} //end of switch
	} //end if velocity == 0
}

/**
 * Allows the user to change number (3 digits) by scrolling
 * Options are: frequency, t_on, t_off
 */
boolean firstEntry = true;
void changeNumber(uint16_t* number, uint16_t max, uint8_t printPosition) {
	if(firstEntry == true){
		firstEntry = false;
		LCDCursorOffBlinkOn(&lcd);
	}
	switch (digit_mode) {
	case HUNDRED_DIGIT:
		digit = *number / 100;
		setCursor(&lcd, printPosition, submode);

		// Choosing the hundreds digit 0-6
		if(rotaryVal != prevRotaryVal)
		{
			//Number goes up
			if (rotaryVal > prevRotaryVal){
				(digit == max / 100) ? digit = 0 : digit ++;
			}
			//Number goes down
			else {
				(digit == 0)? digit = max / 100 : digit --;
			}

			*number = (*number % 100) + digit * 100;
			if (*number > max) {
				*number = max;
				LCDPrintNumber(&lcd, 0, printPosition+1, submode, 1);
				LCDPrintNumber(&lcd, 0, printPosition+2, submode, 1);
			}
			if (*number == 0) {
				*number = 1;
				LCDPrintNumber(&lcd, 1, printPosition+2, submode, 1);
			}
			LCDPrintNumber(&lcd, digit, printPosition, submode, 1);
			prevRotaryVal =  rotaryVal;
		}

		if (buttonPushed) {
			buttonPushed = false;
			if (*number == max) {
				submode = -1;
				inSubmode = false;
			}
			else digit_mode = TEN_DIGIT;
		}

		break;

	case TEN_DIGIT:
		digit = (*number % 100) / 10;
		setCursor(&lcd, printPosition+1, submode);

		// Choosing the ten digit 0-9
		if(rotaryVal != prevRotaryVal)
		{
			//Number goes up
			if (rotaryVal > prevRotaryVal){
				if (digit == 9) {
					digit = 0;
					*number += 10;
				}
				else digit ++;
			}
			//Number goes down
			else {
				if (digit == 0) {
					digit = 9;
					*number -= 10;
				}
				else digit --;
			}
			*number = (*number % 10) + digit * 10 + (int)(*number / 100) * 100;
			LCDPrintNumber(&lcd, *number, printPosition, submode, 3);
			prevRotaryVal =  rotaryVal;
		}

		if (buttonPushed) {
			buttonPushed = false;
			digit_mode = SINGLE_DIGIT;
		}

		break;

	case SINGLE_DIGIT:
		digit = *number % 10;
		setCursor(&lcd, printPosition+2, submode);

		// Choosing the single digit 0-9
		if(rotaryVal != prevRotaryVal)
		{
			//Number goes up
			if (rotaryVal > prevRotaryVal){
				if (digit == 9) {
					digit = 0;
					*number = *number + 1;
				}
				else digit ++;
			}
			//Number goes down
			else {
				if (digit == 0) {
					digit = 9;
					*number = *number - 1;
				}
				else digit --;
			}

			*number = (int)(*number / 10) * 10 + digit;
			LCDPrintNumber(&lcd, *number, printPosition, submode, 3);
			prevRotaryVal =  rotaryVal;
		}

		if (buttonPushed) {
			digit_mode = HUNDRED_DIGIT;
			setCursor(&lcd, 0, submode);
			buttonPushed = false;
			submode = -1;
			inSubmode = false;

			firstEntry = true;
			LCDCursorOffBlinkOff(&lcd);
		}

		break;

	default:
		break;
	}
}

void turnOffAllCoils() {
	setTimerFrequencyPulseWidth(&COIL1, 0, 0, COIL1_CH);
	coil1On = false;
	coil1Freq = 0;

	setTimerFrequencyPulseWidth(&COIL2, 0, 0, COIL2_CH);
	coil2On = false;
	coil2Freq = 0;

	setTimerFrequencyPulseWidth(&COIL3, 0, 0, COIL3_CH);
	coil3On = false;
	coil3Freq = 0;

	setTimerFrequencyPulseWidth(&COIL4, 0, 0, COIL4_CH);
	coil4On = false;
	coil4Freq = 0;

	setTimerFrequencyPulseWidth(&COIL5, 0, 0, COIL5_CH);
	coil5On = false;
	coil5Freq = 0;

}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM9_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_FATFS_Init();
  MX_TIM6_Init();
  MX_USB_DEVICE_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
	//HAL_TIM_Base_Start_IT(&htim6);	//microseconds counter
	HAL_TIM_Base_Start_IT(&htim7);

	initLCD(&lcd, &hi2c2, MAX_ROW, 20, 0x27);
	setCursor(&lcd, 0, 0);

//	HAL_GPIO_WritePin(GPIOB, Flash__wp_Pin | Flash__Hold_Pin, GPIO_PIN_SET);//active low signals

	writeStatusLED(0b11001100);

//	HAL_ADC_Start_IT(&hadc);
//	HAL_StatusTypeDef bruh = HAL_ADC_Start_DMA(&hadc, (uint32_t*) adc_dma_results, adcChannelCount);

	HAL_Delay(50);
	fresult = f_mount(&fs, "", 1);
	if (fresult != FR_OK)
		printToUSB("Error mounting SD card");


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	//Open SD card and store all file names in an array
	fresult = f_opendir(&dir, "");


	if (fresult == FR_OK) {
		// Read the directory and store file names
		for (;;) {
			fresult = f_readdir(&dir, &fno);
			if (fresult != FR_OK || fno.fname[0] == 0) {
				break; // No more files in the directory or an error occurred
			}
			if (fno.fattrib & AM_DIR) {
				// Skip directories
				continue;
			}

			// Copy the file name to the array
			strncpy(fileNames[fileCount++], fno.fname, 30);
		}
		f_closedir(&dir);
	}

	//The last option is always "BACK" to mode selection
	strncpy(fileNames[fileCount], "BACK", 30);
	(void)LCDCursorOffBlinkOff(&lcd);


	while (1) {
		time = HAL_GetTick();

		switch (state) {
		case MODE_SELECT:
			songNum = 0;

			//List all modes
			if (!printed) {
				LCDPrintAtPos(&lcd, "Select mode:", 0, 0);
				LCDPrintAtPos(&lcd, ">", 1, modeNum);
				LCDPrintAtPos(&lcd, "1.SD Card", 2, 1);
				LCDPrintAtPos(&lcd, "2.Burst", 2, 2);
				LCDPrintAtPos(&lcd, "3.Fixed", 2, 3);
				setCursor(&lcd, 1, modeNum);
				printed = true;
			}

			// Choosing the mode
			if(rotaryVal != prevRotaryVal)
			{
				LCDPrintAtPos(&lcd, " ", 1, modeNum);

				//Move down
				if (rotaryVal > prevRotaryVal){
					modeNum == MODE_NUM ? modeNum = 1 : modeNum ++;
				}
				//Move up
				else {
					(modeNum == 1)? modeNum = MODE_NUM : modeNum --;
				}

				LCDPrintAtPos(&lcd, ">", 1, modeNum);
				setCursor(&lcd, 1, modeNum);
				prevRotaryVal =  rotaryVal;
			}

			// Button pushed, change state
			if (buttonPushed) {
				buttonPushed = false;
				printed = false;
				clearDisplay(&lcd);

				switch (modeNum)
				{
				case SD_MODE:
					state = SD_MODE;
					break;
				case BURST_MODE:
					state = BURST_MODE;
					break;
				case FIXED_MODE:
					state = FIXED_MODE;
					break;
				default:
					break;
				}
			}

			break;

		case SD_MODE:
			SDMode();
			break;


		case BURST_MODE:
			if (!printed) {
				LCDPrintAtPos(&lcd, ">", 0, field_select);
				LCDPrintAtPos(&lcd, "Freq:", 1, 0);
				LCDPrintNumber(&lcd, frequency, FREQ_DISP_POS, 0, 3);

				LCDPrintAtPos(&lcd, "T_on:", 1, 1);
				LCDPrintNumber(&lcd, t_on, T_ON_DISP_POS, 1, 3);
				LCDPrintAtPos(&lcd, "BURST", 15, 0);

				LCDPrintAtPos(&lcd, "T_off:", 1, 2);
				LCDPrintNumber(&lcd, t_off, T_OFF_DISP_POS, 2, 3);

				LCDPrintAtPos(&lcd, "BACK", 16, 2);
				LCDPrintAtPos(&lcd, "Ontime:", 8, 3);
				LCDPrintNumber(&lcd, onTime[0], 15, 3, 3);
				LCDPrintAtPos(&lcd, "us", 18, 3);
				setCursor(&lcd, 0, field_select);
			}


			if (!isPlaying) {
				if (!printed) {
					LCDPrintAtPos(&lcd, "Play", 1, 3);
					printed = true;
					setCursor(&lcd, 0, field_select);
				}
				if (onTime[0] != prevOnTime[0]) {
					prevOnTime[0] = onTime[0];
					LCDPrintNumber(&lcd, onTime[0], 15, 3, 3);
					setCursor(&lcd, 0, field_select);
				}

			}

			//isPlaying = true
			else {
				if (!printed) {
					LCDPrintAtPos(&lcd, "Pause", 1, 3);
					printed = true;
					setCursor(&lcd, 0, field_select);
				}
				coil1On = true;
				if (!inCycle) {
					timeStarted = time;
				}
				if (time <= timeStarted + t_on + t_off) {
					inCycle = true;
					if (time <= timeStarted + t_on) {
						if(firstOpenDir || frequency != prevFrequency){
							prevFrequency = frequency;
							setTimerFrequencyPulseWidth(&COIL1, frequency, onTime[0], COIL1_CH);
							firstOpenDir = false;
						}
					}
					else {
						setTimerFrequencyPulseWidth(&COIL1, 0, 0, COIL1_CH);
						firstOpenDir = true;
					}
				}
				else inCycle = false;


			}

			switch (submode) {

			case B_FREQUENCY:
				inSubmode = true;
				changeNumber(&frequency, MAX_FREQUENCY, FREQ_DISP_POS);

				break;

			case B_TON:
				inSubmode = true;
				changeNumber(&t_on, MAX_TIME_ON, T_ON_DISP_POS);


				break;

			case B_TOFF:
				inSubmode = true;
				changeNumber(&t_off, MAX_TIME_OFF, T_OFF_DISP_POS);

				break;

			default:
				break;
			}

			if (!inSubmode) {
				// Choosing the field to config
				if(rotaryVal != prevRotaryVal)
				{
					//BACK is displayed at the bottom right
					if (field_select == B_BACK) {
						LCDPrintAtPos(&lcd, " ", 15, 2);
					}
					else LCDPrintAtPos(&lcd, " ", 0, field_select);

					//Move down
					if (rotaryVal > prevRotaryVal){
						field_select == BURST_STATE_NUM ? field_select = 0 : field_select ++;
					}
					//Move up
					else {
						(field_select == 0)? field_select = BURST_STATE_NUM : field_select --;
					}

					//BACK is displayed at the bottom right
					if (field_select == B_BACK) {
						LCDPrintAtPos(&lcd, ">", 15, 2);
					}
					else LCDPrintAtPos(&lcd, ">", 0, field_select);
					setCursor(&lcd, 0, field_select);

					prevRotaryVal =  rotaryVal;
				}

				if (buttonPushed) {
					buttonPushed = false;

					switch (field_select) {
					case B_FREQUENCY:
						submode = B_FREQUENCY;
						break;

					case B_TON:
						submode = B_TON;
						break;

					case B_TOFF:
						submode = B_TOFF;
						break;

					case B_PLAY_PAUSE:
						printed = false;
						clearDisplay(&lcd);
						if (isPlaying) {
							turnOffAllCoils();
						}
						isPlaying = !isPlaying;

						break;

					case B_BACK:
						submode = -1;
						printed = false;
						clearDisplay(&lcd);
						field_select = 0;
						if (isPlaying) {
							turnOffAllCoils();
							isPlaying = false;
						}
						state = MODE_SELECT;
						break;

					default:
						break;

					}
				}
			}


			break;

			case FIXED_MODE:
				if (!printed) {
					LCDPrintAtPos(&lcd, ">", 0, field_select);
					LCDPrintAtPos(&lcd, "Freq:", 1, 0);
					LCDPrintNumber(&lcd, frequency, FREQ_DISP_POS, 0, 3);

					LCDPrintAtPos(&lcd, "FIXED", 15, 0);
					LCDPrintAtPos(&lcd, "BACK", 1, 2);

					LCDPrintAtPos(&lcd, "Ontime:", 1, 3);
					LCDPrintNumber(&lcd, onTime[0], 8, 3, 3);
					LCDPrintAtPos(&lcd, "us", 11, 3);
					setCursor(&lcd, 0, field_select);
				}


				if (!isPlaying) {
					if (!printed) {
						LCDPrintAtPos(&lcd, "Play", 1, 1);
						printed = true;
						setCursor(&lcd, 0, field_select);
					}
					if (onTime[0] != prevOnTime[0]) {
						prevOnTime[0] = onTime[0];
						LCDPrintNumber(&lcd, onTime[0], 8, 3, 3);
						setCursor(&lcd, 0, field_select);
					}
				}

				//isPlaying = true
				else {
					if (!printed) {
						LCDPrintAtPos(&lcd, "Pause", 1, 1);
						printed = true;
						setCursor(&lcd, 0, field_select);
					}

					if (!coil1On) {
						setTimerFrequencyPulseWidth(&COIL1, frequency, onTime[0], COIL1_CH);
						coil1On = true;
					}

					if (prevFrequency != frequency || onTime[0] != prevOnTime[0]) {
						turnOffAllCoils();
						setTimerFrequencyPulseWidth(&COIL1, frequency, (uint16_t)onTime[0], COIL1_CH);
						prevFrequency = frequency;
						prevOnTime[0] = onTime[0];
						LCDPrintNumber(&lcd, onTime[0], 8, 3, 3);
						coil1On = true;
						setCursor(&lcd, 0, field_select);
					}
				}



				switch (submode) {
				case F_FREQUENCY:
					inSubmode = true;
					changeNumber(&frequency, MAX_FREQUENCY, FREQ_DISP_POS);
					break;

				default:
					break;
				}



				if (!inSubmode) {
					// Choosing the field to config
					if(rotaryVal != prevRotaryVal)
					{
						//BACK is displayed at the bottom right
						if (field_select == B_BACK) {
							LCDPrintAtPos(&lcd, " ", 0, 2);
						}
						else LCDPrintAtPos(&lcd, " ", 0, field_select);

						//Move down
						if (rotaryVal > prevRotaryVal){
							field_select == FIXED_STATE_NUM ? field_select = 0 : field_select ++;
						}
						//Move up
						else {
							(field_select == 0)? field_select = FIXED_STATE_NUM : field_select --;
						}

						LCDPrintAtPos(&lcd, ">", 0, field_select);
						setCursor(&lcd, 0, field_select);

						prevRotaryVal =  rotaryVal;
					}


					if (buttonPushed) {
						buttonPushed = false;

						switch (field_select) {
						case F_FREQUENCY:
							submode = F_FREQUENCY;
							break;

						case F_PLAY_PAUSE:
							printed = false;
							clearDisplay(&lcd);
							if (isPlaying) {
								turnOffAllCoils();
							}
							isPlaying = !isPlaying;

							break;

						case F_BACK:
							submode = -1;
							printed = false;
							clearDisplay(&lcd);
							field_select = 0;
							if (isPlaying) {
								turnOffAllCoils();
								isPlaying = false;
							}
							state = MODE_SELECT;
							break;

						default:
							break;

						}
					}
				}

				break;

				default:
					break;
		}

		//https://www.youtube.com/watch?v=AloHXBk6Bfk
		if((time - adcTime) > 50){
//			uint32_t t1 = HAL_GetTick();
			HAL_ADC_Stop_DMA(&hadc);  // Stop the DMA to ensure it resets
			HAL_StatusTypeDef bruh = HAL_ADC_Start_DMA(&hadc, (uint32_t*) adc_dma_results, adcChannelCount);
//			uint32_t t2 = HAL_GetTick();
			adcTime = time;
		}
		if(adc_conv_complete){
			adc_conv_complete = false;

			//onTime = (uint8_t)((((float)adcVal) / 255)*MAX_PULSE_WIDTH);
			onTime[0] = (adc_dma_results[0] * MAX_PULSE_WIDTH) >> 8;
			onTime[1] = (adc_dma_results[1] * MAX_PULSE_WIDTH) >> 8;
			writeStatusLED(onTime[0]);
		}
	}
}


  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_8B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc.Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc.Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.NbrOfConversion = 2;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 70-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 3200-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 160-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 3200-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 10000-1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM9 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM9_Init(void)
{

  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 0;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 65535;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim9, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM9_Init 2 */

  /* USER CODE END TIM9_Init 2 */
  HAL_TIM_MspPostInit(&htim9);

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 0;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim10, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */
  HAL_TIM_MspPostInit(&htim10);

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 0;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 65535;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim11, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */
  HAL_TIM_MspPostInit(&htim11);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(stat595_SER_GPIO_Port, stat595_SER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, stat595_SCK_Pin|SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_Heartbeat_Pin|Flash__wp_Pin|Flash__Hold_Pin|SPI2_SS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : stat595_SER_Pin */
  GPIO_InitStruct.Pin = stat595_SER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(stat595_SER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : stat595_SCK_Pin */
  GPIO_InitStruct.Pin = stat595_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(stat595_SCK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Heartbeat_Pin Flash__wp_Pin Flash__Hold_Pin SPI2_SS_Pin */
  GPIO_InitStruct.Pin = LED_Heartbeat_Pin|Flash__wp_Pin|Flash__Hold_Pin|SPI2_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ROT_BUT_Pin */
  GPIO_InitStruct.Pin = ROT_BUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ROT_BUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ROT_CLK_Pin */
  GPIO_InitStruct.Pin = ROT_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ROT_CLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ROT_DAT_Pin */
  GPIO_InitStruct.Pin = ROT_DAT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ROT_DAT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_CS_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

void writeStatusLED(uint8_t status) {
	HAL_GPIO_WritePin(stat595_SER_GPIO_Port, stat595_SER_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(stat595_SCK_GPIO_Port, stat595_SCK_Pin, GPIO_PIN_RESET);
	//	delayMicroseconds(5);

	for (int i = 0; i < 8; i++) {
		HAL_GPIO_WritePin(stat595_SER_GPIO_Port, stat595_SER_Pin,
				(status & (1 << i)) >> i);
		//delayMicroseconds(5);
		HAL_GPIO_WritePin(stat595_SCK_GPIO_Port, stat595_SCK_Pin, GPIO_PIN_SET);
		//delayMicroseconds(5);
		HAL_GPIO_WritePin(stat595_SCK_GPIO_Port, stat595_SCK_Pin,
				GPIO_PIN_RESET);
	}
	//delayMicroseconds(5);
	HAL_GPIO_WritePin(stat595_SCK_GPIO_Port, stat595_SCK_Pin, GPIO_PIN_SET);
	//delayMicroseconds(5);
	HAL_GPIO_WritePin(stat595_SCK_GPIO_Port, stat595_SCK_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  EXTI line detection callbacks.
 * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == ROT_BUT_Pin) {
		buttonPushed = true;
	} else if (GPIO_Pin == ROT_CLK_Pin) {
		if (HAL_GPIO_ReadPin(ROT_DAT_GPIO_Port, ROT_DAT_Pin) == 1) {
			rotaryVal--;
		} else
			rotaryVal++;
	}
}

/**
 * @brief	Delays for a given number of microseconds. Accurate to nearest multiple
 * 			of 5 microseconds
 * @param	delay - number of microseconds to delay
 */
void delayMicroseconds(uint64_t delay) {
	uint64_t startTime = micros;
	//do nothing during the delay
	while ((micros - startTime) < delay)
		;
}

uint64_t getMicros() {
	return micros;
}

/**
 * @brief	print a string to the USB COM port
 */
void printToUSB(char *s) {
	int len = strlen(s);
	CDC_Transmit_FS((uint8_t*)s, len);
}
void USBDataReceived_IT(uint8_t *Buf, uint32_t *len) {
	//	memcpy(midiData[eventPointer], Buf, 6);
	//	eventPointer++;
	//	if(eventPointer < 512){
	//		midiData[eventPointer][0] = *Buf;
	//		midiData[eventPointer][1] = *(Buf+1);
	//		midiData[eventPointer][2] = *(Buf+2);
	//		midiData[eventPointer][3] = *(Buf+3);
	//		midiData[eventPointer][4] = *(Buf+4);
	//		midiData[eventPointer][5] = *(Buf+5);
	//		eventPointer++;
	//	}
	//	writeStatusLED((uint8_t)*len);

	for (int i = 0; i < (*len) / 3; i++) {
		uint8_t track = *Buf;
		Buf++;
		uint16_t freq = noteFreq[*Buf - 24];
		Buf++;
		uint16_t velocity = *Buf;
		Buf++;
		if (track == 1)
			setTimerFrequencyPulseWidth(&htim3, freq, velocity, TIM_CHANNEL_1);
	}

	//CDC_Transmit_FS(Buf, *len);
	//clear the buffer
	//	memset(USB_Rx_Buf, '\0', sizeof(USB_Rx_Buf));
	//
	//	//then copy contents
	//	memcpy(USB_Rx_Buf, Buf, (uint8_t)*len);
	//writeStatusLED(*Buf);
	//writeStatusLED((uint8_t)(*len));

}

/**
 * 2 main timers: one for micros, one for heartbeat LED
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	if (htim == &htim6) {
		//auto reload every 160 cycles (5uS)
		micros += 5;
	} else if (htim == &htim7) {
		HAL_GPIO_TogglePin(LED_Heartbeat_GPIO_Port, LED_Heartbeat_Pin);
	}
}

//void sendUSB(char *s, uint16_t bytes){
//	CDC_Transmit_FS(s, bytes);
//}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	adc_conv_complete = true;
}

void SDModeChooseSong(){
	// No need to print again if no need to refresh
	// e.g. Going to a new page
	if (!printed) {
		clearDisplay(&lcd);
		LCDPrintAtPos(&lcd, ">", 0, songNum % MAX_ROW );
		for (int i = 0; i < MAX_ROW; i++) {
			if (((int)(songNum / MAX_ROW) * MAX_ROW + i) == fileCount + 1) break;
			strncpy(displayedText, &fileNames[(int)(songNum / MAX_ROW) * MAX_ROW + i][0], MAX_CHAR_ON_SCREEN);
			displayedText[MAX_CHAR_ON_SCREEN] = '\0';
			LCDPrintAtPos(&lcd, displayedText,1, i);
		}
		printed = true;
		setCursor(&lcd, 0, songNum % MAX_ROW);
	}

	// If the current selected song is longer than what the screen can display,
	// give it scroll effect

	if (strlen(fileNames[songNum]) > MAX_CHAR_ON_SCREEN) {
		//begin non blocking delay
		if(updateTimeScroll == true){
			updateTimeScroll = false;
			timeForScroll = HAL_GetTick();
			strncpy(displayedText, &fileNames[songNum][scrollPosition], MAX_CHAR_ON_SCREEN);
			displayedText[MAX_CHAR_ON_SCREEN] = '\0';  // Null-terminate the string

			// Display the portion on the LCD
			LCDPrintAtPos(&lcd, displayedText, 1, songNum % MAX_ROW);

			// Increment the scroll position and wrap around
			scrollPosition++;
			if (scrollPosition > strlen(fileNames[songNum]) - MAX_CHAR_ON_SCREEN)
				scrollPosition = 0;
		}
		else{
			if((HAL_GetTick() - timeForScroll) > 750){
				updateTimeScroll = true;
			}
		}
		//end nonblocking delay
	}

	// Scrolling up and down to choose the song
	if(rotaryVal != prevRotaryVal){
		LCDPrintAtPos(&lcd, " ", 0, songNum % MAX_ROW);

		//Move down
		if (rotaryVal > prevRotaryVal){
			//We need to go to a new page, so display need to refresh
			if (songNum % MAX_ROW == MAX_ROW-1) printed = false;

			// If the previous song had the scroll effect, we make the screen
			// display what it can
			else {
				if (strlen(fileNames[songNum]) > MAX_CHAR_ON_SCREEN) {
					strncpy(displayedText, &fileNames[songNum][0], MAX_CHAR_ON_SCREEN);
					displayedText[MAX_CHAR_ON_SCREEN] = '\0';
					LCDPrintAtPos(&lcd, displayedText, 1, songNum % MAX_ROW);
				}
			}

			if (songNum == fileCount) {
				songNum = 0;
				printed = false;
			}
			else {
				songNum ++;
			}
		}

		//Move up
		else{
			//We need to go to a new page, so display need to refresh
			if (songNum % MAX_ROW == 0) {
				printed = false;
			}

			// If the previous song had the scroll effect, we make the screen
			// display what it can
			else {
				if (strlen(fileNames[songNum]) > MAX_CHAR_ON_SCREEN) {
					strncpy(displayedText, &fileNames[songNum][0], MAX_CHAR_ON_SCREEN);
					displayedText[MAX_CHAR_ON_SCREEN] = '\0';
					LCDPrintAtPos(&lcd, displayedText,1, songNum % MAX_ROW);
				}
			}

			if (songNum == 0) {
				//songNum = (int)(fileCount / MAX_ROW) * MAX_ROW;
//				songNum = fileCount;
				songNum = fileCount;
			}
//			else if (songNum % MAX_ROW == 0) {
//				songNum -= MAX_ROW;
//			}
			else {
				songNum --;
			}
		}

		LCDPrintAtPos(&lcd, ">", 0, songNum % MAX_ROW);
		setCursor(&lcd, 0, songNum % MAX_ROW);
		prevRotaryVal =  rotaryVal;
	}

	// Song selected, either a song or "BACK" button
	if (buttonPushed) {
		buttonPushed = false;
		printed = false;
		clearDisplay(&lcd);

		if (songNum == fileCount) {
			isDirOpen = false;
			f_closedir(&dir);
			state = MODE_SELECT;
		}
		else {
			isPlaying = true;
		}
	}
}

void SDDealWithScreen(){
	//Print song information, instruction to return, and volume
	if (!printed) {
		LCDPrintAtPos(&lcd, "Playing...", 0, 0);
		strncpy(displayedText, &fileNames[songNum][0], MAX_CHAR_ON_SCREEN);
		displayedText[MAX_CHAR_ON_SCREEN] = '\0';
		LCDPrintAtPos(&lcd, "Click to return", 0, 2);
		LCDPrintAtPos(&lcd, displayedText, 1, 1);

		LCDPrintAtPos(&lcd, "Ton A:   us B:   us", 0, 3);
		LCDPrintNumber(&lcd, onTime[0], 6, 3, 3);
		LCDPrintNumber(&lcd, onTime[1], 14, 3, 3);

		setCursor(&lcd, 16, 3);
		printed = true;
	}
	if(onTime[0] != prevOnTime[0] || onTime[1] != prevOnTime[1]){
		prevOnTime[0] = onTime[0];
		prevOnTime[1] = onTime[1];
		LCDPrintNumber(&lcd, onTime[0], 6, 3, 3);
		LCDPrintNumber(&lcd, onTime[1], 14, 3, 3);
		setCursor(&lcd, 16, 3);
	}
}

#define SD_GET_FIRST	0
#define SD_FETCH_NEXT	1
#define	SD_CALC_NEXT	2
#define SD_WAIT_NEXT	3
#define SD_READ_ERR		4

typedef struct noteEvent{
	uint32_t timeOfEvent;
	uint32_t bitsForPWM[2];
	uint8_t track;
	uint8_t noteNum;
	uint16_t frequency, prescaler, autoReloadReg;
	uint8_t velocity;
}NoteEvent;

uint8_t sdModeState = SD_GET_FIRST, track, noteNum;
uint32_t tNext;
NoteEvent nextEvent;
float velRatio;
TIM_HandleTypeDef *doThisCoil;

void SDMode(){
	if (!isDirOpen) {
		fresult = f_opendir(&dir, "");
		isDirOpen = true;
	}

	if (!isPlaying) SDModeChooseSong();

	// isPlaying = true
	else {
		SDDealWithScreen();
		switch(sdModeState){
		case SD_GET_FIRST:
			f_open(&fil, fileNames[songNum], FA_READ);
			fresult = f_read(&fil, &numEventsSplit[0], 2, &bytesRead);
			if(fresult != FR_OK){
				sdModeState = SD_READ_ERR;
				break;
			}
			fresult = f_read(&fil, &midiBuf[0], 6, &bytesRead);
			if(fresult != FR_OK){
				sdModeState = SD_READ_ERR;
				break;
			}
			numEvents = numEventsSplit[0] | (numEventsSplit[1] << 8);
			eventCounter++;
			nextEvent.timeOfEvent = (midiBuf[1]) | (midiBuf[2]<<8) | (midiBuf[3]<<16);
			nextEvent.noteNum = midiBuf[4];
			nextEvent.frequency = noteFreq[midiBuf[4]-12];
			nextEvent.track = midiBuf[0];
			nextEvent.velocity = midiBuf[5];
//			SDDealWithScreen();
			HAL_Delay(1000);

			sdModeState = SD_CALC_NEXT;
			timeStarted = time;
			break;
		case SD_FETCH_NEXT:
			if (eventCounter < numEvents){
				fresult = f_read(&fil, &midiBuf[0], 6, &bytesRead);
				if(fresult != FR_OK){
					sdModeState = SD_READ_ERR;
					break;
				}
				nextEvent.timeOfEvent = (midiBuf[1]) | (midiBuf[2]<<8) | (midiBuf[3]<<16);
				nextEvent.noteNum = midiBuf[4];
				nextEvent.frequency = noteFreq[midiBuf[4]-12];
				nextEvent.track = midiBuf[0];
				nextEvent.velocity = midiBuf[5];
				eventCounter++;
				sdModeState = SD_CALC_NEXT;
			}
			else {
				turnOffAllCoils();
				eventCounter = 0;
				timeStarted = 0;
				fresult = f_close(&fil);

				clearDisplay(&lcd);
				printed = false;

				isPlaying = false;
				sdModeState = SD_GET_FIRST;
			}

			break;
		case SD_CALC_NEXT: {
			//uint32_t tBeforeCalc = HAL_GetTick();
			//(velocity / 127) * onTime

			// Calculate both bits for PWM since we don't know which note this coil is going on yet. 
			// We don't want a note to go on the wrong coil at a really different ontime.
			uint16_t actualOnTimeSD[2];
			actualOnTimeSD[0] = (nextEvent.velocity * onTime[0]) >> 7;
			actualOnTimeSD[1] = (nextEvent.velocity * onTime[1]) >> 7;

			if(actualOnTimeSD[0] > MAX_PULSE_WIDTH) actualOnTimeSD[0] = MAX_PULSE_WIDTH;
			if(actualOnTimeSD[1] > MAX_PULSE_WIDTH) actualOnTimeSD[1] = MAX_PULSE_WIDTH;

			if(nextEvent.frequency > MAX_FREQUENCY) nextEvent.frequency = MAX_FREQUENCY;

			if(nextEvent.frequency == 1)        nextEvent.prescaler = 512 - 1;
			else if(nextEvent.frequency <= 3)   nextEvent.prescaler = 256 - 1;
			else if(nextEvent.frequency <= 7)   nextEvent.prescaler = 128 - 1;
			else if(nextEvent.frequency <= 15)  nextEvent.prescaler = 64 - 1;
			else if(nextEvent.frequency <= 32)  nextEvent.prescaler = 32 - 1;
			else if(nextEvent.frequency <= 63)  nextEvent.prescaler = 16 - 1;
			else if(nextEvent.frequency <= 127) nextEvent.prescaler = 8 - 1;
			else if(nextEvent.frequency <= 255) nextEvent.prescaler = 4 - 1;
			else if(nextEvent.frequency <= 511) nextEvent.prescaler = 2 - 1;
			else nextEvent.prescaler = 1 - 1;

			nextEvent.autoReloadReg = CPU_CLK / ((nextEvent.prescaler+1) * nextEvent.frequency);
			double usPerBit = (double)(nextEvent.prescaler+1) / 32.0; //will get optimized by compiler
			nextEvent.bitsForPWM[0] = (uint32_t)((double)actualOnTimeSD[0] / usPerBit);
			nextEvent.bitsForPWM[1] = (uint32_t)((double)actualOnTimeSD[1] / usPerBit);
			//uint32_t timeForCalc = HAL_GetTick() - tBeforeCalc;

			sdModeState = SD_WAIT_NEXT;
			break;
		}

		case SD_WAIT_NEXT:{
			if((HAL_GetTick() - timeStarted > nextEvent.timeOfEvent)){

				// Find the coil that this note event is going on
				doThisCoil = findTimForThisCombo(nextEvent.track, nextEvent.frequency, nextEvent.velocity);
				uint32_t specific_bits;

				// If COIL1 or COIL2, this is the big coil so use ontime[0]W
				if(doThisCoil == &COIL1 || doThisCoil == &COIL2){
					specific_bits = nextEvent.bitsForPWM[0];
				}
				else specific_bits = nextEvent.bitsForPWM[1];

				// Finally, set the bits
				if(nextEvent.velocity > 0){
					doThisCoil->Instance->CCR1 = 0;
					doThisCoil->Instance->CCR2 = 0;
					__disable_irq();
					doThisCoil->Instance->ARR = nextEvent.autoReloadReg;
					doThisCoil->Instance->PSC = nextEvent.prescaler;
					doThisCoil->Instance->CCR1 = specific_bits;
					doThisCoil->Instance->CCR2 = specific_bits;
					__enable_irq();
				}
				else{
					doThisCoil->Instance->CCR1 = 0;
					doThisCoil->Instance->CCR2 = 0;
				}
				sdModeState = SD_FETCH_NEXT;
			}
			break;
		}

		case SD_READ_ERR:
			while(1){

			}
			break;
		}

		// Return to song select
		if (buttonPushed) {
			buttonPushed = false;

			turnOffAllCoils();
			eventCounter = 0;
			timeStarted = 0;
			fresult = f_close(&fil);

			clearDisplay(&lcd);
			printed = false;

			isPlaying = false;
			sdModeState = SD_GET_FIRST;
		}

	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
