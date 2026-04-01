/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include <string.h> // 需要用到 memcpy
#include <stdio.h>
#include <stdlib.h> // atoi
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define META_ADDRESS    0x08008000  // Sector 2 (16KB) - 存储版本信息
#define APP_ADDRESS     0x0800C000  // Sector 3 (16KB) - APP 起始地址 (注意修改 APP 的 VTOR)

#define META_MAGIC      0xA5A5A5A5  // 有效标志
#define RX_BUFFER_SIZE  1040        // 接收缓冲区
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
void PrintAppVector(void)
{
    uint32_t app_stack = *(__IO uint32_t*)APP_ADDRESS;
    uint32_t app_reset = *(__IO uint32_t*)(APP_ADDRESS + 4);

    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "APP Vectors:");
    OLED_Printf(0, 10, OLED_6X8, "SP   = 0x%08X", app_stack);
    OLED_Printf(0, 20, OLED_6X8, "Reset= 0x%08X", app_reset);

    uint8_t thumb = app_reset & 1;
    OLED_Printf(0, 30, OLED_6X8, "Thumb bit=%d", thumb);

    uint8_t sp_ok = ((app_stack & 0x2FFE0000U) == 0x20000000U);
    OLED_Printf(0, 40, OLED_6X8, "SP OK=%d", sp_ok);

    OLED_Update();
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

typedef struct {
    uint32_t magic_word; // 用于判断Flash是否为空，例如 0xA5A5A5A5
    char version[32];
    char publisher[32];
    char date[32];
} AppMetadata_t;

AppMetadata_t serverMeta; // 暂存从网络获取的版本信息

uint16_t crc16_modbus(const uint8_t* data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}


// 必须加回这个函数，用于清洗串口缓存，防止命令粘包
void UART_Flush(void) {
    uint8_t temp;
    while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE)) {
        HAL_UART_Receive(&huart1, &temp, 1, 10);
    }
}

// 辅助函数：跳过回显，解析整数响应
// 输入示例: "AT+GetBinSize\r\n323264\r\nOK" -> 返回 323264
uint32_t Parse_Int_Response(char* str) {
    // 1. 如果以 AT 开头，跳过第一行 (回显)
    if (strncmp(str, "AT", 2) == 0) {
        char* next = strchr(str, '\n');
        if (next) str = next + 1;
    }
    
    // 2. 跳过可能的空行
    while (*str == '\r' || *str == '\n') str++;
    
    // 3. 转换数字
    return (uint32_t)atoi(str);
}

// 定义全局 buffer (复用之前的定义)
uint8_t buffer[RX_BUFFER_SIZE]; 

// ==========================================
// 新增: 发送 AT 指令并等待 OK
// ==========================================
uint8_t ESP_Send_Cmd(char* cmd, char* ack, uint32_t timeout) {
    UART_Flush(); // 发送前清洗
    HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), 100);
    
    memset(buffer, 0, RX_BUFFER_SIZE);
    // 接收回复
    HAL_UART_Receive(&huart1, buffer, RX_BUFFER_SIZE - 1, timeout);
    
    // 检查是否包含期望的响应 (如 "OK")
    if (strstr((char*)buffer, ack)) {
        return 1;
    }
    return 0;
}

// ==========================================
// 新增: 初始化 ESP8266 配置 (WiFi & URL)
// ==========================================
void ESP_Config_Init(void) {
    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "Config ESP8266...");
    OLED_Update();
    
    // 1. 测试通信 (握手)
    uint8_t retry = 0;
    while(!ESP_Send_Cmd("AT\r\n", "OK", 500)) {
        retry++;
        OLED_Printf(0, 10, OLED_6X8, "Wait ESP.. %d", retry);
        OLED_Update();
        HAL_Delay(500);
        if(retry > 20) return; // 超时放弃
    }

    // 2. 设置 WiFi (ESP 端会返回 OK_RECONNECTING)
    // 注意: 命令中不要带空格
    OLED_Printf(0, 20, OLED_6X8, "Set WiFi...");
    OLED_Update();
    if(ESP_Send_Cmd("AT+SetWifi=aaa,aaaaaaaa\r\n", "OK", 2000)) {
        OLED_Printf(0, 20, OLED_6X8, "Set WiFi OK  ");
    } else {
        OLED_Printf(0, 20, OLED_6X8, "Set WiFi Err ");
    }
    OLED_Update();

    // 3. 设置版本文件 URL
    OLED_Printf(0, 30, OLED_6X8, "Set Ver URL...");
    OLED_Update();
		//if(ESP_Send_Cmd("AT+SetVerUrl=http://192.168.3.102/version.txt\r\n", "OK", 1000)) {
    if(ESP_Send_Cmd("AT+SetVerUrl=http://111.231.116.18/version.txt\r\n", "OK", 1000)) {
        OLED_Printf(0, 30, OLED_6X8, "Ver URL OK   ");
    } else {
        OLED_Printf(0, 30, OLED_6X8, "Ver URL Err  ");
    }
    OLED_Update();

    // 4. 设置固件 URL
    OLED_Printf(0, 40, OLED_6X8, "Set FW URL...");
    OLED_Update();
		//if(ESP_Send_Cmd("AT+SetFwUrl=http://192.168.3.102/app.bin\r\n", "OK", 1000)) {
    if(ESP_Send_Cmd("AT+SetFwUrl=http://111.231.116.18/app.bin\r\n", "OK", 1000)) {
        OLED_Printf(0, 40, OLED_6X8, "FW URL OK    ");
    } else {
        OLED_Printf(0, 40, OLED_6X8, "FW URL Err   ");
    }
    OLED_Update();
    
    HAL_Delay(1000); // 展示一下配置结果
}

// 跳转到 APP
typedef void (*pFunction)(void);
void JumpToApplication(void) {
    OLED_Printf(0, 50, OLED_6X8, "Jumping to APP...");
    OLED_Update();
    
    // 检查栈顶地址是否合法 (粗略检查)
    if (((*(__IO uint32_t*)APP_ADDRESS) & 0x2FFE0000) == 0x20000000) {
        
        // 2. 准备跳转变量
        uint32_t JumpAddress = *(__IO uint32_t*)(APP_ADDRESS + 4);
        pFunction JumpToApp = (pFunction)JumpAddress;

        // 3. 关闭所有外设和中断 (清理现场)
        __disable_irq(); // 关总中断

        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL  = 0;

        for (uint32_t i = 0; i < 8; i++) {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }
                
        HAL_RCC_DeInit(); 
        HAL_DeInit();
                
        // 4. 重定向中断向量表
        SCB->VTOR = APP_ADDRESS;

        // 5. 设置主堆栈指针 (这是最后一步操作)
        __set_MSP(*(__IO uint32_t*)APP_ADDRESS);
        __enable_irq();
        // 6. 跳转
        JumpToApp();
    } else {
        OLED_Clear();
        OLED_Printf(0, 0, OLED_6X8, "Err: No APP!");
        OLED_Printf(0, 10, OLED_6X8, "Addr: 0x%X", APP_ADDRESS);
        OLED_Update();
        while(1); 
    }
}

// ===== Flash 操作: 擦除 APP 区域 (Sector 3 - 7) =====
void Flash_Erase_App(void) {
    FLASH_EraseInitTypeDef EraseInit;
    uint32_t PageError;
    HAL_FLASH_Unlock();
    
    // F407VET6: Sec3(16K), Sec4(64K), Sec5(128K), Sec6(128K), Sec7(128K)
    EraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    EraseInit.Sector       = FLASH_SECTOR_3; 
    EraseInit.NbSectors    = 5; // 擦除 3,4,5,6,7
    EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    if (HAL_FLASHEx_Erase(&EraseInit, &PageError) != HAL_OK) {
        OLED_Printf(0, 50, OLED_6X8, "Erase APP Fail");
        OLED_Update();
    }
    HAL_FLASH_Lock();
}

// ===== Flash 操作: 更新 Sector 2 的版本信息 =====
void Flash_Update_Metadata(AppMetadata_t *newMeta) {
    FLASH_EraseInitTypeDef EraseInit;
    uint32_t PageError;
    
    HAL_FLASH_Unlock();
    
    // 1. 擦除 Sector 2
    EraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    EraseInit.Sector       = FLASH_SECTOR_2;
    EraseInit.NbSectors    = 1; 
    EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    HAL_FLASHEx_Erase(&EraseInit, &PageError);

    // 2. 写入结构体数据
    uint32_t *pData = (uint32_t*)newMeta;
    for(uint32_t i = 0; i < sizeof(AppMetadata_t); i += 4) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, META_ADDRESS + i, *(pData + i/4));
    }
    
    HAL_FLASH_Lock();
}

// ===== Flash 操作: 写入数据 =====
HAL_StatusTypeDef Flash_Write_Data(uint32_t addr, uint8_t *data, uint32_t len) {
    HAL_StatusTypeDef status = HAL_OK;
    HAL_FLASH_Unlock();
    for(uint32_t i=0; i<len; i+=4) {
        uint32_t word = *(uint32_t*)(data+i);
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i, word);
        if(status != HAL_OK) break;
    }
    HAL_FLASH_Lock();
    return status;
}

// ===== 辅助: 解析版本字符串 =====
void Parse_Version_String(char* buffer, AppMetadata_t* meta) {
    char *token;
    
    // 1. Version
    token = strtok(buffer, "\r\n");
    if(token) strncpy(meta->version, token, 32);
    
    // 2. Publisher
    token = strtok(NULL, "\r\n");
    if(token) strncpy(meta->publisher, token, 32);
    
    // 3. Date
    token = strtok(NULL, "\r\n");
    if(token) strncpy(meta->date, token, 32);
    
    meta->magic_word = META_MAGIC; // 设置有效标志
}

void OTA_CheckAndDownload(void) {
    
    AppMetadata_t *localMeta = (AppMetadata_t*)META_ADDRESS;
    char oled_buf[32];
    uint32_t firmware_total_size = 0; // 新增：固件总大小

    OLED_Clear();
    OLED_Printf(0, 0, OLED_6X8, "Checking Update...");
    OLED_Update();
    
    // 1. 清洗串口
    UART_Flush();

    // 2. 发送获取版本指令
    memset(buffer, 0, RX_BUFFER_SIZE);
    HAL_UART_Transmit(&huart1, (uint8_t*)"AT+GetNewVersion\r\n", 18, 100);
    // 稍微读多一点，防止截断
    HAL_UART_Receive(&huart1, buffer, 100, 2000); 

    // 简单处理回显：如果buffer开头是AT，手动偏移指针
    char* resp_ptr = (char*)buffer;
    if (strncmp(resp_ptr, "AT", 2) == 0) {
        char* p = strchr(resp_ptr, '\n');
        if(p) resp_ptr = p + 1;
    }
    while(*resp_ptr == '\r' || *resp_ptr == '\n') resp_ptr++; // 跳空行

    // 3. 检查是否收到错误
    if (strncmp(resp_ptr, "ERR", 3) == 0 || strlen(resp_ptr) < 5) {
        OLED_Printf(0, 10, OLED_6X8, "Net Error");
        OLED_Update();
        HAL_Delay(1000);
        return;
    }

    // 4. 解析服务器版本
    memset(&serverMeta, 0, sizeof(AppMetadata_t));
    Parse_Version_String(resp_ptr, &serverMeta);

    // 显示信息
    OLED_Printf(0, 10, OLED_6X8, "Srv: %s", serverMeta.version);
    if (localMeta->magic_word == META_MAGIC) {
        OLED_Printf(0, 20, OLED_6X8, "Loc: %s", localMeta->version);
    } else {
        OLED_Printf(0, 20, OLED_6X8, "Loc: None");
    }
    OLED_Update();
    HAL_Delay(1000);

    // 5. 比较版本
    if (localMeta->magic_word != META_MAGIC || 
        strcmp(localMeta->version, serverMeta.version) != 0) {
        
        OLED_Printf(0, 30, OLED_6X8, "New Ver Found!");
        OLED_Update();
        HAL_Delay(500);

        // ==========================================
        // 新增步骤：获取固件大小
        // ==========================================
        OLED_Printf(0, 40, OLED_6X8, "Get Size...");
        OLED_Update();
        
        UART_Flush(); // 清洗
        HAL_UART_Transmit(&huart1, (uint8_t*)"AT+GetBinSize\r\n", 15, 100);
        memset(buffer, 0, 64);
        HAL_UART_Receive(&huart1, buffer, 63, 1000);
        
        firmware_total_size = Parse_Int_Response((char*)buffer);
        
        // 显示总大小 (例如: Size: 320KB)
        sprintf(oled_buf, "Size: %u KB", firmware_total_size / 1024);
        OLED_Printf(0, 40, OLED_6X8, "%-16s", oled_buf); // 覆盖上一行
        OLED_Update();
        HAL_Delay(500);

        if (firmware_total_size == 0) {
             OLED_Printf(0, 50, OLED_6X8, "Size Err!");
             OLED_Update();
             firmware_total_size = 1; // 防止除以0
        }

        // ==========================================
        // 1. 擦除 APP 区域
        // ==========================================
        OLED_Printf(0, 50, OLED_6X8, "Erasing...");
        OLED_Update();
        Flash_Erase_App();
        
        // 2. 请求固件
        UART_Flush(); // 再次清洗
        HAL_UART_Transmit(&huart1, (uint8_t*)"AT+GetBin\r\n", 11, 100);

        uint32_t write_addr = APP_ADDRESS;
        uint32_t total_received = 0;
        uint8_t rx_byte;
        
        OLED_Printf(0, 50, OLED_6X8, "Downloading...  "); // 准备显示进度
        OLED_Update();

        while(1) {
            // 等待数据头 (超时稍微给大点)
            if(HAL_UART_Receive(&huart1, &rx_byte, 1, 10000) != HAL_OK) { 
                OLED_Printf(0, 50, OLED_6X8, "Rx Timeout!");
                OLED_Update();
                return; 
            }

            // 检查 END
            if(rx_byte == 'E') {
                char end_buf[3] = {0};
                HAL_UART_Receive(&huart1, (uint8_t*)end_buf, 2, 100);
                if(strncmp(end_buf, "ND", 2) == 0) {
                    HAL_UART_Transmit(&huart1, (uint8_t*)"OK\r\n", 4, 100);
                    
                    // 7. 更新版本信息
                    OLED_Printf(0, 50, OLED_6X8, "Update Meta...");
                    OLED_Update();
                    Flash_Update_Metadata(&serverMeta);

                    OLED_Clear();
                    OLED_Printf(0, 10, OLED_6X8, "Update Done!");
                    OLED_Printf(0, 20, OLED_6X8, "%s", serverMeta.version);
                    OLED_Update();
                    HAL_Delay(1000);
                    
                    NVIC_SystemReset();
                }
            }

            // 解析包头 0xAA
            if(rx_byte == 0xAA) {
                uint8_t header[2];
                HAL_UART_Receive(&huart1, header, 2, 100);
                uint16_t len = header[0] | (header[1] << 8);

                if (len > RX_BUFFER_SIZE) len = RX_BUFFER_SIZE;

                HAL_UART_Receive(&huart1, buffer, len, 800); // 增加接收超时
                
                uint8_t tail[3]; // crcL, crcH, 0x55
                HAL_UART_Receive(&huart1, tail, 3, 100);
                
                uint16_t recv_crc = tail[0] | (tail[1] << 8);
                uint16_t cal_crc = crc16_modbus(buffer, len);

                if (cal_crc == recv_crc && tail[2] == 0x55) {
                    if (Flash_Write_Data(write_addr, buffer, len) == HAL_OK) {
                        write_addr += len;
                        total_received += len;
                        HAL_UART_Transmit(&huart1, (uint8_t*)"OK", 2, 100);
                        
                        // 显示百分比进度
                        if ((total_received % 1024) == 0) {
                            uint8_t percent = (uint8_t)((total_received * 100) / firmware_total_size);
                            if(percent > 100) percent = 100;
                            sprintf(oled_buf, "%d%% %uKB", percent, total_received / 1024);
                            OLED_Printf(0, 50, OLED_6X8, "%-16s", oled_buf); 
                            OLED_Update();
                        }
                    } else {
                        HAL_UART_Transmit(&huart1, (uint8_t*)"ERR_FLASH", 9, 100);
                    }
                } else {
                    HAL_UART_Transmit(&huart1, (uint8_t*)"ERR_CRC", 7, 100);
                }
            }
        }
    } else {
        OLED_Printf(0, 30, OLED_6X8, "Version Match");
        OLED_Update();
        HAL_Delay(500);
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
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(1000);
  OLED_Init();
  OLED_Clear();
  OLED_Printf(0, 0, OLED_6X8, "Bootloader v2.1");
  OLED_Update();

  // 1. 检查 PA0 强行更新 (防止APP写坏导致死循环)
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
      OLED_Printf(0, 10, OLED_6X8, "Force Update...");
      OLED_Update();
      AppMetadata_t emptyMeta = {0}; 
      Flash_Update_Metadata(&emptyMeta); 
  }
  
  // 2. 初始化 ESP8266 配置 (WiFi & URL)
  // 即使 ESP8266 EEPROM 里有了，这里覆盖一下也能确保配置正确
  ESP_Config_Init();

  // 3. 执行 OTA 检查与下载
  OTA_CheckAndDownload();
    
  PrintAppVector();
  HAL_Delay(2000); 
    
  JumpToApplication();
    
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_Delay(100);
    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  * where the assert_param error has occurred.
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