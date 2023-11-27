#include "main.h"
#include "string.h"
#include "stdio.h"

extern unsigned char buffer_data[70];
extern int flag3;// flag of gps
int rfidok = 0; 
int espok = 0;
char RFID[14];
char outword[27]; // mang id
int c, len;// bien phuc vu chuyen doi id
int t = 0; // bien xoa outword
uint8_t esp32[2];
int cnt=0; // phuc vu tao chuoi json
const char n[2]=","; // phuc vu tach mang
int flagwferror = 0;

char *token2; //$GNGGA
char *time; //time
char *vido; //Vi do
char *kinhdo; //Kinh do
char s[200]="{\"ID\":\"\",\"Latitude\":\"\",\"Longitude\":\"\",\"Time\":\"\"}"; //json data
char sgps[200]="{\"Latitude\":\"\",\"Longitude\":\"\",\"Time\":\"\"}"; //json data ko khai bao o ngoai thi ko doc dc mang

TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1; // gps
UART_HandleTypeDef huart2; // esp
UART_HandleTypeDef huart3;


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);

void LED_Busy() // San sang quet the
{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);   // trang
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET); // do
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET); // xanh	
}
void LED_Error() // Dang xu ly
{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);	
}
void LED_Ready()
{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);	
}

void chen(char *s,char *a,int vt);
void clearjson();
void Data_RFIDGPS();
void Data_GPS();
void clearjsongps();
void checkwifi();

int main(void)
{
  
  HAL_Init();

 
  SystemClock_Config();

  
  MX_GPIO_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
	
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	HAL_TIM_Base_Start_IT(&htim4);
			t=0;
	for(t=0;t<20;t++) //Starting...
	 {
		 HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_14);
		 HAL_Delay(100);
	 }
  
  while (1)
  {
     
    HAL_UART_Receive_IT(&huart2,(uint8_t*)(&esp32[0]),2); //Nhan du lieu ESP32
		if(espok == 3 & flagwferror==1)
		{checkwifi();}
		if(espok != 1 & espok !=3)
		{LED_Error();}
		if(espok == 1)
		{
			LED_Ready();
			HAL_UART_Receive_IT(&huart3,(uint8_t*) (&RFID[0]),14); //Nhan du lieu RFID
			/* USER CODE END WHILE */
			if(flag3 == 1 && rfidok == 1) 
			{
				LED_Busy();
				Data_RFIDGPS();
				flag3 = 0;
				rfidok = 0;
				espok = 0;
			}
    }  
  }
}
//check wifi
void checkwifi()
{
while (flagwferror == 1)
{
HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_13);
HAL_Delay(500);
HAL_UART_Receive_IT(&huart2,(uint8_t*)(&esp32[0]),2);
}
}

//Ham tao chuoi json
void chen(char *s,char *a,int vt)
{
	char c[200];
	strcpy(c,s+vt);/*chuyen s tu vi tri vt  den c*/
	strcpy(s+vt,a);/*thay a vao s bat dau tu vt*/
	strcat(s,c);/*noi c vao lai s*/
}

//Ham xu ly du lieu GPS
void Data_RFIDGPS(void)
{
  char s[200]="{\"ID\":\"\",\"Latitude\":\"\",\"Longitude\":\"\",\"Time\":\"\"}"; //json data ko khai bao o ngoai thi ko doc dc mang
	cnt=1;
	token2=strtok((char*)buffer_data,n); //Tach tung chuoi con
	while( token2 != NULL ) 
	{
		switch(cnt)
		{
			case 2:
				time=token2;
			break;
			case 3:
				vido=token2;
			break;
			case 5:
				kinhdo=token2;
			break;
		}
		cnt++;			 
		token2 = strtok(NULL, n);
	}
//Convert ascii char to hex char		
	len = 14;
	//memset(outword,0,27);
	for(int t=0; t < 28; t++)
	{
	outword[t]=0;// xoa outword
	}
	for(c = 0; c<len; c++)// outword nhan gia tri rfid dang hex
	{
			sprintf(outword+c*2, "%02X", RFID[c]);/*in ra dang hex cua c* o day dung c*2 la vi khi chuyen sang he 16 thì thành so co 2 gia tri*/
	}
// xoa chuoi json truoc
	clearjson();
	
//Tao chuoi json
	 chen(s,time,46);
	 chen(s,kinhdo,36);
	 chen(s,vido,21);
	 chen(s,outword,7);

	 HAL_UART_Transmit(&huart2,(uint8_t*)s,strlen(s),300); // gui len esp32
}

void Data_GPS(void)
{
	char sgps[200]="{\"Latitude\":\"\",\"Longitude\":\"\",\"Time\":\"\"}"; //json data ko khai bao o ngoai thi ko doc dc mang
	cnt=1;
	token2=strtok((char*)buffer_data,n); //Tach tung chuoi con
	while( token2 != NULL ) 
	{
		switch(cnt)
		{
			case 2:
				time=token2;
			break;
			case 3:
				vido=token2;
			break;
			case 5:
				kinhdo=token2;
			break;
		}
		cnt++;			 
		token2 = strtok(NULL, n);
	}
// xoa chuoi json truoc
	clearjsongps();
	
//Tao chuoi json
	 chen(sgps,time,38);//chen tu lon den be
	 chen(sgps,kinhdo,28);
	 chen(sgps,vido,13);
//truyen sang esp32
	 HAL_UART_Transmit(&huart2,(uint8_t*)sgps,strlen(sgps),300); // gui len esp32
}

void clearjsongps()
{
  for(int t=0; t < 176; t++)
	{
	sgps[13+t]=sgps[23+t];
  }
	for(int t=0; t < 160; t++)
	{
	sgps[28+t]=sgps[39+t];
	}
	for(int t=0 ;t < 151; t++)
	{
  sgps[38+t]=sgps[48+t];
	}
}

void clearjson()
{
   for(int t=0; t < 164; t++)
	{
	s[7+t]=s[35+t];
  }
	for(int t=0; t < 168; t++)
	{
	s[21+t]=s[31+t];
	}
	for(int t=0 ;t < 152; t++)
	{
  s[36+t]=s[47+t];
	}
  for(int t=0 ;t < 143; t++)
	{
	s[46+t]=s[56+t];
	} 	 
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{	
	if(huart->Instance==USART3 && espok == 1) {rfidok = 1;} // RFID OK
	if(huart->Instance==USART2) // nhan lenh tu ESP32
	{
		if(esp32[0]=='o' && esp32[1]=='k') {espok=1;flagwferror = 0;}
		else if(esp32[0]=='n' && esp32[1]=='o') {espok=2;rfidok=0;}
		else if(esp32[0]=='w' && esp32[1]=='f') {espok=3;rfidok=0;flagwferror = 1;}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==htim4.Instance && flag3 == 1 && rfidok == 0 && espok == 1) // 2s update data GPS 1 lan
	{
		Data_GPS();
	}
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
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

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 31999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
  while (1)
  {
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
