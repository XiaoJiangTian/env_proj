#include "w25q64.h"
 
 /**********************************************************************************
  * 函数功能: 模块初始化
  */
uint8_t BSP_W25Qx_Init(void)
{ 	
	BSP_W25Qx_Reset();	
	return BSP_W25Qx_GetStatus();
}
 

static void	BSP_W25Qx_Reset(void)
{
	uint8_t cmd[2] = {RESET_ENABLE_CMD,RESET_MEMORY_CMD};
	
	W25Qx_Enable();
	/* Send the reset command */
	HAL_SPI_Transmit(&hspi1, cmd, 2, W25Qx_TIMEOUT_VALUE);	
	W25Qx_Disable();
 
}
 
 /**********************************************************************************
  * 函数功能: 获取设备状态
  */
static uint8_t BSP_W25Qx_GetStatus(void)
{
	uint8_t cmd[] = {READ_STATUS_REG1_CMD};
	uint8_t status;
	
	W25Qx_Enable();
	/* Send the read status command */
	HAL_SPI_Transmit(&hspi1, cmd, 1, W25Qx_TIMEOUT_VALUE);	
	/* Reception of the data */
	//接收状态
	HAL_SPI_Receive(&hspi1,&status, 1, W25Qx_TIMEOUT_VALUE);
	W25Qx_Disable();
	
	/* Check the value of the register */
  if((status & W25Q128FV_FSR_BUSY) != 0)
  {
    return W25Qx_BUSY;
  }
	else
	{
		return W25Qx_OK;
	}		
	//根据检测到的状态而返回
}
 
 /**********************************************************************************
  * 函数功能: 写使能
  */
uint8_t BSP_W25Qx_WriteEnable(void)
{
	uint8_t cmd[] = {WRITE_ENABLE_CMD};
	uint32_t tickstart = HAL_GetTick();
 
	/*Select the FLASH: Chip Select low */
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 1, W25Qx_TIMEOUT_VALUE);	
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable();
	
	/* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() == W25Qx_BUSY)
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
    {        
			return W25Qx_TIMEOUT;
    }
	}
	
	return W25Qx_OK;
}
 
 /**********************************************************************************
  * 函数功能: 获取设备ID
  */
void BSP_W25Qx_Read_ID(uint8_t *ID)
{
	uint8_t cmd[4] = {READ_ID_CMD,0x00,0x00,0x00};
	
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 4, W25Qx_TIMEOUT_VALUE);	
	/* Reception of the data */
	HAL_SPI_Receive(&hspi1,ID, 2, W25Qx_TIMEOUT_VALUE);
	W25Qx_Disable();
		
}
 
 /**********************************************************************************
  * 函数功能: 读数据
  * 输入参数: 缓存数组指针、读地址、字节数
  */
uint8_t BSP_W25Qx_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
	uint8_t cmd[4];
 
	/* Configure the command */
	cmd[0] = READ_CMD;
	cmd[1] = (uint8_t)(ReadAddr >> 16);
	cmd[2] = (uint8_t)(ReadAddr >> 8);
	cmd[3] = (uint8_t)(ReadAddr);
	
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 4, W25Qx_TIMEOUT_VALUE);	
	/* Reception of the data */
	if (HAL_SPI_Receive(&hspi1, pData,Size,W25Qx_TIMEOUT_VALUE) != HAL_OK)
  {
    return W25Qx_ERROR;
  }
	W25Qx_Disable();
	return W25Qx_OK;
}
 
 /**********************************************************************************
  * 函数功能: 写数据
  * 输入参数: 缓存数组指针、写地址、字节数
  */
uint8_t BSP_W25Qx_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
	uint8_t cmd[4];
	uint32_t end_addr, current_size, current_addr;
	uint32_t tickstart = HAL_GetTick();
	
	/* Calculation of the size between the write address and the end of the page */
  current_addr = 0;
 
  while (current_addr <= WriteAddr)
  {
    current_addr += W25Q128FV_PAGE_SIZE;
  }
	//计算从起始地址到页尾的剩余空间
  current_size = current_addr - WriteAddr;
 
  /* Check if the size of the data is less than the remaining place in the page */
	//如果满足就变成，要写入的大小
  if (current_size > Size)
  {
    current_size = Size;
  }
 
  /* Initialize the adress variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;//结尾地址
	
  /* Perform the write page by page */
  do
  {
		/* Configure the command */
		cmd[0] = PAGE_PROG_CMD; //写指令
		cmd[1] = (uint8_t)(current_addr >> 16); //低位在前，总共24位地址
		cmd[2] = (uint8_t)(current_addr >> 8);
		cmd[3] = (uint8_t)(current_addr);
 
		/* Enable write operations */
		BSP_W25Qx_WriteEnable();
	
		W25Qx_Enable();
    /* Send the command */
    if (HAL_SPI_Transmit(&hspi1,cmd, 4, W25Qx_TIMEOUT_VALUE) != HAL_OK)
    {
      return W25Qx_ERROR;
    }
    
    /* Transmission of the data */
    if (HAL_SPI_Transmit(&hspi1, pData,current_size, W25Qx_TIMEOUT_VALUE) != HAL_OK)
    {
      return W25Qx_ERROR;
    }
			W25Qx_Disable();
    	/* Wait the end of Flash writing */
		while(BSP_W25Qx_GetStatus() == W25Qx_BUSY);
		{
			/* Check for the Timeout */
			if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
			{        
				return W25Qx_TIMEOUT;
			}
		}
    
    /* Update the address and size variables for next page programming */
    current_addr += current_size; //更改起始地址
    pData += current_size; //更改数据位置
    current_size = ((current_addr + W25Q128FV_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25Q128FV_PAGE_SIZE; //左边是不足一页，右边是还大于1页要写
  } while (current_addr < end_addr);//没写完
 
	
	return W25Qx_OK;
}
 
 /**********************************************************************************
  * 函数功能: 扇区擦除
  * 输入参数: 地址
  */
uint8_t BSP_W25Qx_Erase_Block(uint32_t Address)
{
	uint8_t cmd[4];
	uint32_t tickstart = HAL_GetTick();
	cmd[0] = SECTOR_ERASE_CMD;
	cmd[1] = (uint8_t)(Address >> 16);
	cmd[2] = (uint8_t)(Address >> 8);
	cmd[3] = (uint8_t)(Address);
	
	/* Enable write operations */
	BSP_W25Qx_WriteEnable();
	
	/*Select the FLASH: Chip Select low */
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 4, W25Qx_TIMEOUT_VALUE);	
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable();
	
	/* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() == W25Qx_BUSY);
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > W25Q128FV_SECTOR_ERASE_MAX_TIME)
    {        
			return W25Qx_TIMEOUT;
    }
	}
	return W25Qx_OK;
}
 
 /**********************************************************************************
  * 函数功能: 芯片擦除
  */
uint8_t BSP_W25Qx_Erase_Chip(void)
{
	uint8_t cmd[4];
	uint32_t tickstart = HAL_GetTick();
	cmd[0] = CHIP_ERASE_CMD;
	
	/* Enable write operations */
	BSP_W25Qx_WriteEnable();
	
	/*Select the FLASH: Chip Select low */
	W25Qx_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&hspi1, cmd, 1, W25Qx_TIMEOUT_VALUE);	
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable();
	
	/* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() != W25Qx_BUSY)
	{
		/* Check for the Timeout */
    if((HAL_GetTick() - tickstart) > W25Q128FV_BULK_ERASE_MAX_TIME)
    {        
			return W25Qx_TIMEOUT;
    }
	}
	return W25Qx_OK;
}
 
void w25q64_proc(uint8_t *count_write,uint8_t*Dwrite,uint8_t*Dread,uint8_t*write_index,uint8_t *num)
{
	if((*count_write)==12) //写24个数据
	{
		(*count_write)=0;
		BSP_W25Qx_Write(Dwrite,0x00,24);//写入24字节
		(*write_index)=0;
		(*num)++;		
		BSP_W25Qx_Read(Dread,0x00,24);		
	}
}


void w25q64_init(uint8_t *ID)
{
	//W25q64测试
	
	//校验1
	BSP_W25Qx_Init();
	BSP_W25Qx_Read_ID(ID);
	if(ID[0]!=0xEF | ID[1]!=0x16)
	{
		
		OLED_ShowChar(1, 13, '1');
	}        
	//擦除块
	if(BSP_W25Qx_Erase_Block(0)==W25Qx_OK)
	{
		
		OLED_ShowChar(1, 13, '2');
	}
}