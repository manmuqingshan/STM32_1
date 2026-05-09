/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mongoose_glue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma location=0x30000000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30000080
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30000000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30000080))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDescripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDescripSection")));   /* Ethernet Tx DMA Descriptors */
#endif

ETH_TxPacketConfig TxConfig;

ADC_HandleTypeDef hadc1;

ETH_HandleTypeDef heth;

RNG_HandleTypeDef hrng;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_RNG_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
bool mg_random(void *buf, size_t len) {  // Use on-board RNG
  for (size_t n = 0; n < len; n += sizeof(uint32_t)) {
    uint32_t r;
    HAL_RNG_GenerateRandomNumber(&hrng, &r);
    memcpy((char *) buf + n, &r, n + sizeof(r) > len ? len - n : sizeof(r));
  }
  return true; // TODO(): ensure successful RNG init, then return on false above
}

uint64_t mg_millis(void) {
  return HAL_GetTick();
}

int _write(int fd, unsigned char *buf, int len) {
  if (fd == 1 || fd == 2) {                     // stdout or stderr ?
    HAL_UART_Transmit(&huart3, buf, len, 999);  // Print to the UART
  }
  return len;
}


/*************** INPUTS *********************/

//typedef struct {
//	GPIO_TypeDef *port;
//	uint16_t pin;
//}input_t;
//
//
//input_t inputs[] ={
//		{INPUT1_GPIO_Port, INPUT1_Pin},
//		{INPUT2_GPIO_Port, INPUT2_Pin},
//		{INPUT3_GPIO_Port, INPUT3_Pin},
//		{INPUT4_GPIO_Port, INPUT4_Pin},
//		{INPUT5_GPIO_Port, INPUT5_Pin},
//		{INPUT6_GPIO_Port, INPUT6_Pin},
//		{INPUT7_GPIO_Port, INPUT7_Pin},
//		{INPUT8_GPIO_Port, INPUT8_Pin},
//		{INPUT9_GPIO_Port, INPUT9_Pin},
//		{INPUT10_GPIO_Port, INPUT10_Pin},
//};
//
//
//bool READ_INPUTS (uint16_t i)
//{
//	return (HAL_GPIO_ReadPin(inputs[i].port, inputs[i].pin));
//}
//
//void my_modbus_handler(struct mg_modbus_req *req) {
//  if (req->func == MG_MODBUS_FUNC_READ_DISCRETE_INPUTS) {
//    for (uint16_t i = 0; i < req->len; i++) {
//      req->u.bits[i] = READ_INPUTS(i);
//    }
//  }
//  else {
//    req->error = MG_MODBUS_ERR_DEVICE_FAILURE;
//  }
//}
//
//
//static struct inputs dis_inputs[] = {
//  {false},{false},{false},{false},{false},{false},{false},{false},{false},{false},
//};
//
//bool my_get_inputs(struct inputs *data, size_t i) {
//  size_t array_size = sizeof(dis_inputs) / sizeof(dis_inputs[0]);
//  if (i >= array_size) return false;
//
//  for (int j =0; j<array_size; j++)
//  {
//	  dis_inputs[j].level = READ_INPUTS(j);
//  }
//  *data = dis_inputs[i];  // Sync with your device
//  return true;
//}



/*************** COILS *********************/

//uint8_t Coils_Data[2] = {0x00, 0x00};
//
//typedef struct {
//	GPIO_TypeDef *port;
//	uint16_t pin;
//}coil_t;
//
//
//coil_t coils[] ={
//		{COIL1_GPIO_Port, COIL1_Pin},
//		{COIL2_GPIO_Port, COIL2_Pin},
//		{COIL3_GPIO_Port, COIL3_Pin},
//		{COIL4_GPIO_Port, COIL4_Pin},
//		{COIL5_GPIO_Port, COIL5_Pin},
//		{COIL6_GPIO_Port, COIL6_Pin},
//		{COIL7_GPIO_Port, COIL7_Pin},
//		{COIL8_GPIO_Port, COIL8_Pin},
//		{COIL9_GPIO_Port, COIL9_Pin},
//		{COIL10_GPIO_Port, COIL10_Pin},
//};
//
//bool READ_COIL (uint16_t i)
//{
//	uint16_t startByte = i/8;
//	uint8_t bitPosition = i%8;
//
//	bool val = (Coils_Data[startByte] >> bitPosition) & 0x01;
//	return val;
//}
//
//void WRITE_COIL (uint16_t i, bool val)
//{
//	uint16_t startByte = i/8;
//	uint8_t bitPosition = i%8;
//
//	if (val == true) Coils_Data[startByte] |= 1<<bitPosition;
//	else Coils_Data[startByte] &= ~(1<<bitPosition);
//
//	HAL_GPIO_WritePin(coils[i].port, coils[i].pin, val);
//}
//
//
//void my_modbus_handler(struct mg_modbus_req *req) {
//
//  if (req->func == MG_MODBUS_FUNC_READ_COILS) {
//    for (uint16_t i = 0; i < req->len; i++) {
//      req->u.bits[i] = READ_COIL(req->addr + i);
//    }
//  }
//
//  else if (req->func == MG_MODBUS_FUNC_WRITE_SINGLE_COIL) {
//        WRITE_COIL(req->addr, req->u.bits[0]);
//  }
//
//  else if (req->func == MG_MODBUS_FUNC_WRITE_MULTIPLE_COILS) {
//    for (uint16_t i = 0; i < req->len; i++) {
//      WRITE_COIL(req->addr + i, req->u.bits[i]);
//    }
//  } else {
//    req->error = MG_MODBUS_ERR_DEVICE_FAILURE;
//  }
//}
//
//
//
//static struct coils my_coils[] = {
//  {false},{false},{false},{false},{false},{false},{false},{false},{false},{false},
//};
//
//bool my_get_coils(struct coils *data, size_t i) {
//  size_t array_size = sizeof(my_coils) / sizeof(my_coils[0]);
//  if (i >= array_size) return false;
//
//  for (int j=0; j<array_size; j++) my_coils[j].level = READ_COIL(j);
//
//
//  *data = my_coils[i];  // Sync with your device
//  return true;
//}
//
//
//void my_set_coils(struct coils *data, size_t i) {
//  size_t array_size = sizeof(my_coils) / sizeof(my_coils[0]);
//  if (i < array_size) my_coils[i] = *data; // Sync with your device
//
//  for (int j=0; j<array_size; j++) WRITE_COIL(j, my_coils[j].level);
//
//}


/*************** HOLDING REGISTERS *********************/

uint16_t Holding_Reg_Data [12] = {0,2222,3333,4444,5555,6666,7777,8888,9999,1234,5678,9012};

uint16_t READ_Holding_REG (uint16_t i)
{
	return (Holding_Reg_Data[i]);
}

void WRITE_Holding_REG (uint16_t i, uint16_t val)
{
	if (i == 0)
	{
		TIM2->CCR4 = val;
	}

	Holding_Reg_Data[i] = val;
}

//void my_modbus_handler(struct mg_modbus_req *req)
//{
//  if (req->func == MG_MODBUS_FUNC_READ_HOLDING_REGISTERS) {
//    for (uint16_t i = 0; i < req->len; i++) {
//      req->u.regs[i] = READ_Holding_REG(req->addr + i);
//    }
//  }
//
//  else if (req->func == MG_MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS) {
//    for (uint16_t i = 0; i < req->len; i++) {
//      WRITE_Holding_REG(req->addr + i, req->u.regs[i]);
//    }
//  }
//
//  else if (req->func == MG_MODBUS_FUNC_WRITE_SINGLE_REGISTER) {
//      WRITE_Holding_REG(req->addr, req->u.regs[0]);
//  }
//
//  else {
//    req->error = MG_MODBUS_ERR_DEVICE_FAILURE;
//  }
//}


static struct holding_registers my_holding_registers[] = {
  {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
};

bool my_get_holding_registers(struct holding_registers *data, size_t i) {
  size_t array_size = sizeof(my_holding_registers) / sizeof(my_holding_registers[0]);
  if (i >= array_size) return false;
  my_holding_registers[i].value = READ_Holding_REG(i);
  *data = my_holding_registers[i];  // Sync with your device
  return true;
}

void my_set_holding_registers(struct holding_registers *data, size_t i) {
  size_t array_size = sizeof(my_holding_registers) / sizeof(my_holding_registers[0]);
  if (i < array_size) my_holding_registers[i] = *data; // Sync with your device
  WRITE_Holding_REG(i, my_holding_registers[i].value);
}


/*************** INPUT REGISTERS *********************/

uint16_t ReadADC (void)
{
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	uint16_t adcVal = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return adcVal;
}


uint16_t Input_Reg_Data [6] = {0,2222,3333,4444,5555,6666};

uint16_t READ_Input_REG (uint16_t i)
{
	if (i == 0)
	{
		return ReadADC();
	}
	return (Input_Reg_Data[i]);
}

static struct input_registers my_input_registers[] = {
  {0},{0},{0},{0},{0},{0},
};

bool my_get_input_registers(struct input_registers *data, size_t i) {
  size_t array_size = sizeof(my_input_registers) / sizeof(my_input_registers[0]);
  if (i >= array_size) return false;
  my_input_registers[i].value = READ_Input_REG(i);
  *data = my_input_registers[i];  // Sync with your device
  return true;
}




void my_modbus_handler(struct mg_modbus_req *req)
{
  if (req->func == MG_MODBUS_FUNC_READ_HOLDING_REGISTERS) {
    for (uint16_t i = 0; i < req->len; i++) {
      req->u.regs[i] = READ_Holding_REG(req->addr + i);
    }
  }

  else if (req->func == MG_MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS) {
    for (uint16_t i = 0; i < req->len; i++) {
      WRITE_Holding_REG(req->addr + i, req->u.regs[i]);
    }
  }

  else if (req->func == MG_MODBUS_FUNC_WRITE_SINGLE_REGISTER) {
      WRITE_Holding_REG(req->addr, req->u.regs[0]);
  }

  else if (req->func == MG_MODBUS_FUNC_READ_INPUT_REGISTERS) {
	    for (uint16_t i = 0; i < req->len; i++) {
	      req->u.regs[i] = READ_Input_REG(req->addr + i);
	    }
	  }

  else {
    req->error = MG_MODBUS_ERR_DEVICE_FAILURE;
  }
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
//  int32_t timeout;
/* USER CODE END Boot_Mode_Sequence_0 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
  /* Wait until CPU2 boots and enters in stop mode or timeout*/
//  timeout = 0xFFFF;
//  while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0));
//  if ( timeout < 0 )
//  {
//  Error_Handler();
//  }
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
/* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
HSEM notification */
/*HW semaphore Clock enable*/
//__HAL_RCC_HSEM_CLK_ENABLE();
///*Take HSEM */
//HAL_HSEM_FastTake(HSEM_ID_0);
///*Release HSEM in order to notify the CPU2(CM4)*/
//HAL_HSEM_Release(HSEM_ID_0,0);
///* wait until CPU2 wakes up from stop mode */
//timeout = 0xFFFF;
//while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
//if ( timeout < 0 )
//{
//Error_Handler();
//}
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_RNG_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  mongoose_init();
  mongoose_set_modbus_handler(my_modbus_handler);
  mongoose_set_http_handlers("holding_registers", my_get_holding_registers, my_set_holding_registers);
  mongoose_set_http_handlers("input_registers", my_get_input_registers, NULL);
  mongoose_add_ws_reporter(200, "holding_registers");
  mongoose_add_ws_reporter(200, "input_registers");
  for (;;) {
    mongoose_poll();
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.Oversampling.Ratio = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1536;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

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

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
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
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
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
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(COIL4_GPIO_Port, COIL4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, COIL6_Pin|COIL7_Pin|COIL8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, COIL9_Pin|COIL10_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : COIL4_Pin */
  GPIO_InitStruct.Pin = COIL4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(COIL4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : COIL6_Pin COIL7_Pin COIL8_Pin */
  GPIO_InitStruct.Pin = COIL6_Pin|COIL7_Pin|COIL8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : COIL9_Pin COIL10_Pin */
  GPIO_InitStruct.Pin = COIL9_Pin|COIL10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
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
#ifdef USE_FULL_ASSERT
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
