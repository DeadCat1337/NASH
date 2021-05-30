#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Buttons.h"              // ::Board Support:Buttons
#include "it.h"
#include "encrypt.h"
#include "blockchain.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>


#include <stdio.h>
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
   if (DEMCR & TRCENA) {

while (ITM_Port32(0) == 0){}
    ITM_Port8(0) = ch;
  }
  return(ch);
}

#define LCD_ADDR (0x27 << 1)

#define PIN_RS    (1 << 0)
#define PIN_EN    (1 << 2)
#define BACKLIGHT (1 << 3)
#define LCD_DELAY_MS 50

USART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;
ADC_HandleTypeDef hadc1;

void SystemClock_Config(void);
void Init_UART(void);
void Init_I2C(void);
void InitADC(void);
uint32_t randomADC(void);


uint8_t I2C_Scan() {
    char info[] = "\r\nScanning I2C bus...\r\n";
    HAL_USART_Transmit(&huart2, (uint8_t*)info, strlen(info), HAL_MAX_DELAY);

	uint8_t num = 0;
	//printf("here\n");
    HAL_StatusTypeDef res;
    for(uint16_t i = 0; i < 128; i++) {
        res = HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 1, 10);
        if(res == HAL_OK) {
            char msg[64];
            snprintf(msg, sizeof(msg), "\r\n0x%02X\r\n", i);
            HAL_USART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
					//printf("I2C OK: %d\r\n", i);
					Delay(20);
					num++;
        } else {
            HAL_USART_Transmit(&huart2, (uint8_t*)".", 1, HAL_MAX_DELAY);
					//printf("%d ", i);
					//HAL_Delay(100);
					Delay(20);
					//for(uint32_t j = 0; j < 50000; j++) { }
        }
    }
//		if(num == 0) {
//			printf("Scan Failed\r\n");
//		}
		return num;
    //HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
}

HAL_StatusTypeDef LCD_SendInternal(uint8_t lcd_addr, uint8_t data, uint8_t flags) {
    HAL_StatusTypeDef res;
    for(;;) {
        res = HAL_I2C_IsDeviceReady(&hi2c1, lcd_addr, 1, HAL_MAX_DELAY);
        if(res == HAL_OK)
            break;
    }

    uint8_t up = data & 0xF0;
    uint8_t lo = (data << 4) & 0xF0;

    uint8_t data_arr[4];
    data_arr[0] = up|flags|BACKLIGHT|PIN_EN;
    data_arr[1] = up|flags|BACKLIGHT;
    data_arr[2] = lo|flags|BACKLIGHT|PIN_EN;
    data_arr[3] = lo|flags|BACKLIGHT;

    res = HAL_I2C_Master_Transmit(&hi2c1, lcd_addr, data_arr, sizeof(data_arr), HAL_MAX_DELAY);
		//HAL_Delay(LCD_DELAY_MS);
    Delay(LCD_DELAY_MS);
    return res;
}

void LCD_SendCommand(uint8_t lcd_addr, uint8_t cmd) {
    LCD_SendInternal(lcd_addr, cmd, 0);
}

void LCD_SendData(uint8_t lcd_addr, uint8_t data) {
    LCD_SendInternal(lcd_addr, data, PIN_RS);
}

void LCD_Init(uint8_t lcd_addr) {
    // 4-bit mode, 2 lines, 5x7 format
    LCD_SendCommand(lcd_addr, 0b00110000);
    // display & cursor home (keep this!)
    LCD_SendCommand(lcd_addr, 0b00000010);
    // display on, right shift, underline off, blink off
    LCD_SendCommand(lcd_addr, 0b00001100);
    // clear display (optional here)
    LCD_SendCommand(lcd_addr, 0b00000001);
}

void LCD_SendString(uint8_t lcd_addr, char *str) {
    while(*str) {
        LCD_SendData(lcd_addr, (uint8_t)(*str));
        str++;
    }
}

void init() {
  //if(I2C_Scan() != 0) {
			LCD_Init(LCD_ADDR);
			Delay(20);
			// set address to 0x00
			LCD_SendCommand(LCD_ADDR, 0b10000000);
			Delay(20);
			LCD_SendString(LCD_ADDR, " Using 1602 LCD");
			Delay(20);

			// set address to 0x40
			LCD_SendCommand(LCD_ADDR, 0b11000000);
			Delay(20);
			LCD_SendString(LCD_ADDR, "  over I2C bus");
			Delay(20);
	//}
}


int main(void)
{
	HAL_Init();                               /* Initialize the HAL Library     */

  SystemClock_Config();                     /* Configure the System Clock     */
	SysTick_Config(SystemCoreClock/1000);  		/* Generate interrupt each 1 ms  */
  LED_Initialize();                         /* Initialize LED                 */
  Buttons_Initialize();                     /* Initialize Buttons             */
	Init_UART();
	Init_I2C();
	InitADC();
	LCD_Init(LCD_ADDR);
	
	halfblock_t r_keys[2*ROUND_NUM + 2];
  halfblock_t key[KEY_BLOCKS];
	
	srand ( 12345 );
	char str[16];
	for(int i = 0; i < KEY_BLOCKS; i++) {
#if HALF_SIZE==32
    key[i] = randomADC();//(uint32_t)( rand() << 16 | rand() );
#elif HALF_SIZE==64
    key[i] = randomADC();//static_cast<uint64_t>( rand() << 16 | rand() ) << 32;
    key[i] |= randomADC() << 32;//static_cast<uint64_t>( rand() << 16 | rand() );
#endif
  }
	gen_round_keys(key, r_keys);
	
	printf("%d %d %d %d %d %d\r\n", r_keys[0], r_keys[1], r_keys[2], r_keys [3], r_keys[48], r_keys[49]);
	
	//HAL_USART_Transmit(&huart2, (uint8_t*)"\r\n\n", 3, 0xFFFF);
	for(int i = 0; i < KEY_BLOCKS/2; i++) {
		snprintf(str, sizeof(str), "0x%08X ", key[2*i]);
		LCD_SendCommand(LCD_ADDR, 0b10000000);
		LCD_SendString(LCD_ADDR, str);
		//HAL_USART_Transmit(&huart2, (uint8_t*)str, strlen(str), 0xFFFF);
		snprintf(str, sizeof(str), "0x%08X ", key[2*i + 1]);
		LCD_SendCommand(LCD_ADDR, 0b11000000);
		LCD_SendString(LCD_ADDR, str);
		//HAL_USART_Transmit(&huart2, (uint8_t*)str, strlen(str), 0xFFFF);
		while(Buttons_GetState() != 1U);
		Delay(50);
	}
	LCD_SendCommand(LCD_ADDR, 0b10000000);
	LCD_SendString(LCD_ADDR, " Sending        ");
	LCD_SendCommand(LCD_ADDR, 0b11000000);
	LCD_SendString(LCD_ADDR, " message...     ");
	
	char *str1 = "Test message to confirm encryption/decryption algorithm.\r\n\0\0\0\0\0\0\0\0\0\0\0\0";
	//HAL_USART_Transmit(&huart2, (uint8_t*)str1, strlen(str1), 0xFFFF);
	
	uint32_t MEESAGE_SIZE = strlen(str1)*8/HALF_SIZE;
	halfblock_t encrypted[MEESAGE_SIZE];
  halfblock_t decrypted[MEESAGE_SIZE];
  halfblock_t * message=(halfblock_t *) str1;                         

  halfblock_t vector[2];

#if HALF_SIZE==32
  //vector[0] = randomADC();//static_cast<uint32_t>( rand() << 16 | rand() );
  //vector[1] = randomADC();//static_cast<uint32_t>( rand() << 16 | rand() );
	vector[0] = 0x01234567;
	vector[1] = 0x89ABCDEF;
#elif HALF_SIZE==64
  //vector[0] = randomADC();//static_cast<uint64_t>( rand() << 16 | rand() ) << 32;
  //vector[0] |= randomADC() << 32;//static_cast<uint64_t>( rand() << 16 | rand() );
  //vector[1] = randomADC();//static_cast<uint64_t>( rand() << 16 | rand() ) << 32;
  //vector[1] |= randomADC() << 32;//static_cast<uint64_t>( rand() << 16 | rand() );
	vector[0] = 0x01234567 << 32;
	vector[0] |= 0x89ABCDEF;
	vector[1] = 0x01234567 << 32;
	vector[1] |= 0x89ABCDEF;
#endif

  encryptblock(message, encrypted, MEESAGE_SIZE, vector, r_keys);
  decryptblock(decrypted,encrypted, MEESAGE_SIZE, vector, r_keys);
		
	HAL_USART_Transmit(&huart2, (uint8_t*)encrypted, MEESAGE_SIZE*HALF_SIZE/8, 0xFFFF);

	
	
//#define MEESAGE_SIZE        10
//  halfblock_t message[MEESAGE_SIZE];
//  halfblock_t encrypted[MEESAGE_SIZE];
//  halfblock_t decrypted[MEESAGE_SIZE];

//  for(int i = 0; i < MEESAGE_SIZE/2; i++) {
//    message[2*i] = 2*i;
//    message[2*i + 1] = 2*i + 1;

//    encrypt(message + 2*i, encrypted + 2*i, r_keys);
//    decrypt(decrypted+2*i, encrypted + 2*i, r_keys);
//  }

//	char *str2 = "Message -> Encrypted -> Decrypted:\r\n";
//	HAL_USART_Transmit(&huart2, (uint8_t*)str2, strlen(str2), 0xFFFF);
//  for(int i = 0; i < MEESAGE_SIZE; i++) {
//		snprintf(str, 14, "0x%08X -> ", message[i]);
//		HAL_USART_Transmit(&huart2, (uint8_t*)str, strlen(str), 0xFFFF);
//		snprintf(str, 14, "0x%08X -> ", encrypted[i]);
//		HAL_USART_Transmit(&huart2, (uint8_t*)str, strlen(str), 0xFFFF);
//		snprintf(str, 12, "0x%08X\r\n", decrypted[i]);
//		HAL_USART_Transmit(&huart2, (uint8_t*)str, strlen(str), 0xFFFF);
//  }


	LCD_SendCommand(LCD_ADDR, 0b11000000);
	LCD_SendString(LCD_ADDR, "                ");
	LCD_SendCommand(LCD_ADDR, 0b10000000);
	LCD_SendString(LCD_ADDR, "                ");
	while(1)
	{
//		if( HAL_USART_Receive_IT(&huart2, (uint8_t*)tmp, 1) == HAL_OK ) 
//		{
//			printf("%s", tmp);
//			num++;
//			if(num == 16 || tmp[0] == '\n')
//			{
//				recv[num] = 0;
//				LCD_SendCommand(LCD_ADDR, 0b10000000);
//				LCD_SendString(LCD_ADDR, recv);
//				LCD_SendCommand(LCD_ADDR, 0b11000000);
//				num = 0;
//			}
//			recv[num] = tmp[0];
//			LCD_SendString(LCD_ADDR, tmp);
//		}
		
		Delay(200);
		LCD_SendCommand(LCD_ADDR, 0b10000000);
	  LCD_SendString(LCD_ADDR, "                ");
		Delay(200);
		LCD_SendCommand(LCD_ADDR, 0b10000000);
		LCD_SendString(LCD_ADDR, "     Ready!     ");
		
	}
}


uint32_t randomADC() {
	uint32_t res = 0;
	//HAL_ADC_Start(&hadc1);
	for(int i = 0; i < 32; ) {
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 1000000) == HAL_OK) {
			res |= (HAL_ADC_GetValue(&hadc1) & 0x01) << i;
			i++;
		}
		HAL_ADC_Stop(&hadc1);
	}
	//HAL_ADC_Stop(&hadc1);
	return res;
}

void Init_UART(void) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef PORT_SETUP;
	PORT_SETUP.Pin = GPIO_PIN_2|GPIO_PIN_3;
	PORT_SETUP.Mode = GPIO_MODE_AF_PP;
	PORT_SETUP.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	PORT_SETUP.Pull = GPIO_PULLDOWN;
	PORT_SETUP.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &PORT_SETUP);
	
	__HAL_RCC_USART2_CLK_ENABLE();
	huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = USART_WORDLENGTH_8B;
  huart2.Init.StopBits = USART_STOPBITS_1;
  huart2.Init.Parity = USART_PARITY_NONE;
  huart2.Init.Mode = USART_MODE_TX_RX;
	huart2.Init.CLKPhase = USART_PHASE_1EDGE;
	huart2.Init.CLKLastBit = USART_LASTBIT_DISABLE;
	huart2.Init.CLKPolarity = USART_POLARITY_LOW;
  if (HAL_USART_Init(&huart2) != HAL_OK)
  {
    while(1);
  }
	
}

void Init_I2C(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
    /* Peripheral clock enable */
  __HAL_RCC_I2C1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	//I2C_HandleTypeDef hi2c1;
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0x0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    while(1);
  }
	
}

void InitADC(void) {
    
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
  ADC_ChannelConfTypeDef sConfig;
	GPIO_InitTypeDef GPIO_InitStruct;
	/**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
	*/
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV8;
	hadc1.Init.Resolution = ADC_RESOLUTION12b;
	hadc1.Init.ScanConvMode = ENABLE;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.NbrOfDiscConversion = 1;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 8;
	hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.EOCSelection = EOC_SEQ_CONV;
	HAL_ADC_Init(&hadc1);
	/**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
	*/
	sConfig.Channel = ADC_CHANNEL_10;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);
	
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the
     device is clocked below the maximum system frequency (see datasheet). */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;				              //8;				<- 168.000.000
  RCC_OscInitStruct.PLL.PLLN = 192;										//336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;					//RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;											//7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
	
	SystemCoreClock = 96000000;					//168000000;
}