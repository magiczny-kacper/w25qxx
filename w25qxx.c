#include "w25qxx.h"

#if (_W25QXX_DEBUG == 1)
#include <stdio.h>
#endif

#ifdef _W25QXX_DEBUG
#define W25QXX_DBG_LOG()
#else
#define W25QXX_DBG_LOG()
#endif /* _W25QXX_DEBUG */

typedef enum W25QXX_StatusRegisters {
	STATUS_REGISTER_1 = 0,
	STATUS_REGISTER_2,
	STATUS_REGISTER_3,
	STATUS_REGISTER_CNT,
} W25QXX_StatusRegisters_t;

typedef union W25QXX_StatusRegister1 {
	struct {
		uint8_t BUSY :1;
		uint8_t WEL :1;
		uint8_t BP0 :1;
		uint8_t BP1 :1;
		uint8_t BP2 :1;
		uint8_t TB :1;
		uint8_t SEC :1;
		uint8_t SRP0 :1;
	};
	uint8_t value;
} W25QXX_StatusRegister1_t;

typedef union W25QXX_StatusRegister2 {
	struct {
		uint8_t SRP1 :1;
		uint8_t QE :1;
		uint8_t R :1;
		uint8_t LB1 :1;
		uint8_t LB2 :1;
		uint8_t LB3 :1;
		uint8_t CMP :1;
		uint8_t SUS :1;
	};
	uint8_t value;
} W25QXX_StatusRegister2_t;

static uint32_t W25qxx_ReadID(w25qxx_t *mem);
static void W25qxx_ReadUniqID(w25qxx_t *mem);
static void W25qxx_WriteEnable(w25qxx_t *mem);
static void W25qxx_WriteDisable(w25qxx_t *mem);
static uint8_t W25qxx_ReadStatusRegister(w25qxx_t *mem, W25QXX_StatusRegisters_t register);
static void W25qxx_WriteStatusRegister(w25qxx_t *mem, W25QXX_StatusRegisters_t register, uint8_t status);
static void W25qxx_WaitForWriteEnd(w25qxx_t *mem);
static void W25qxx_WaitLock(w25qxx_t *mem);

//###################################################################################################################
bool W25qxx_Init(w25qxx_t *mem)
{
	uint32_t id;
	mem->Lock = 1;
	W25qxx_Delay(1);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
	W25qxx_Delay(100);
	
	W25QXX_DBG_LOG("w25qxx Init Begin...\r\n");

	id = W25qxx_ReadID(mem);
	W25QXX_DBG_LOG("w25qxx ID:0x%X\r\n", id);

	switch (id & 0x000000FF)
	{
		case 0x20: // 	w25q512
			mem->ID = W25Q512;
			mem->BlockCount = 1024;
			W25QXX_DBG_LOG("w25qxx Chip: w25q512\r\n");
			break;
		case 0x19: // 	w25q256
			mem->ID = W25Q256;
			mem->BlockCount = 512;
			W25QXX_DBG_LOG("w25qxx Chip: w25q256\r\n");
			break;
		case 0x18: // 	w25q128
			mem->ID = W25Q128;
			mem->BlockCount = 256;
			W25QXX_DBG_LOG("w25qxx Chip: w25q128\r\n");
			break;
		case 0x17: //	w25q64
			mem->ID = W25Q64;
			mem->BlockCount = 128;
			W25QXX_DBG_LOG("w25qxx Chip: w25q64\r\n");
			break;
		case 0x16: //	w25q32
			mem->ID = W25Q32;
			mem->BlockCount = 64;
			W25QXX_DBG_LOG("w25qxx Chip: w25q32\r\n");
			break;
		case 0x15: //	w25q16
			mem->ID = W25Q16;
			mem->BlockCount = 32;
			W25QXX_DBG_LOG("w25qxx Chip: w25q16\r\n");
			break;
		case 0x14: //	w25q80
			mem->ID = W25Q80;
			mem->BlockCount = 16;
			W25QXX_DBG_LOG("w25qxx Chip: w25q80\r\n");
			break;
		case 0x13: //	w25q40
			mem->ID = W25Q40;
			mem->BlockCount = 8;
			W25QXX_DBG_LOG("w25qxx Chip: w25q40\r\n");
			break;
		case 0x12: //	w25q20
			mem->ID = W25Q20;
			mem->BlockCount = 4;
			W25QXX_DBG_LOG("w25qxx Chip: w25q20\r\n");
			break;
		case 0x11: //	w25q10
			mem->ID = W25Q10;
			mem->BlockCount = 2;
			W25QXX_DBG_LOG("w25qxx Chip: w25q10\r\n");
			break;
		default:
			W25QXX_DBG_LOG("w25qxx Unknown ID\r\n");
			mem->Lock = 0;
			return false;
	}
	mem->PageSize = 256;
	mem->SectorSize = 0x1000;
	mem->SectorCount = mem->BlockCount * 16;
	mem->PageCount = (mem->SectorCount * mem->SectorSize) / mem->PageSize;
	mem->BlockSize = mem->SectorSize * 16;
	mem->CapacityInKiloByte = (mem->SectorCount * mem->SectorSize) / 1024;
	W25qxx_ReadUniqID(mem);
	W25qxx_ReadStatusRegister(mem, STATUS_REGISTER_1);
	W25qxx_ReadStatusRegister(mem, STATUS_REGISTER_2);
	W25qxx_ReadStatusRegister(mem, STATUS_REGISTER_3);
	W25QXX_DBG_LOG("w25qxx Page Size: %d Bytes\r\n", mem->PageSize);
	W25QXX_DBG_LOG("w25qxx Page Count: %d\r\n", mem->PageCount);
	W25QXX_DBG_LOG("w25qxx Sector Size: %d Bytes\r\n", mem->SectorSize);
	W25QXX_DBG_LOG("w25qxx Sector Count: %d\r\n", mem->SectorCount);
	W25QXX_DBG_LOG("w25qxx Block Size: %d Bytes\r\n", mem->BlockSize);
	W25QXX_DBG_LOG("w25qxx Block Count: %d\r\n", mem->BlockCount);
	W25QXX_DBG_LOG("w25qxx Capacity: %d KiloBytes\r\n", mem->CapacityInKiloByte);
	W25QXX_DBG_LOG("w25qxx Init Done\r\n");
	mem->Lock = 0;
	return true;
}
//###################################################################################################################
void W25qxx_EraseChip(w25qxx_t *mem)
{
	uint8_t cmd = 0xC7;
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
#if (_W25QXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	W25QXX_DBG_LOG("w25qxx EraseChip Begin...\r\n");
#endif
	W25qxx_WriteEnable(mem);
	MEM_SELECT(mem->cs_port, mem->cs_pin);
	spi_write(mem->spi, &cmd, 1);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
	W25qxx_WaitForWriteEnd(mem);
	W25QXX_DBG_LOG("w25qxx EraseBlock done after %d ms!\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(10);
	mem->Lock = 0;
}
//###################################################################################################################
void W25qxx_EraseSector(w25qxx_t *mem, uint32_t SectorAddr)
{
	uint8_t cmd[5] = {0};
	uint8_t cmd_len = 0;
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
#if (_W25QXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	W25QXX_DBG_LOG("w25qxx EraseSector %d Begin...\r\n", SectorAddr);
#endif
	W25qxx_WaitForWriteEnd(mem);
	SectorAddr = SectorAddr * mem->SectorSize;
	W25qxx_WriteEnable(mem);
	if (mem->ID >= W25Q256){
		cmd[cmd_len] = 0x21;
		cmd_len ++;
		cmd[cmd_len] = (SectorAddr & 0xFF000000) >> 24;
		cmd_len ++;
	}else{
		cmd[cmd_len] = 0x20;
		cmd_len ++;
	}
	cmd[cmd_len] = (SectorAddr & 0xFF0000) >> 16;
	cmd_len ++;
	cmd[cmd_len] = (SectorAddr & 0xFF00) >> 8;
	cmd_len ++;
	cmd[cmd_len] = (SectorAddr & 0xFF);
	cmd_len ++;

	MEM_SELECT(mem->cs_port, mem->cs_pin);
	spi_write(mem->spi, cmd, cmd_len);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);

	W25qxx_WaitForWriteEnd(mem);
	W25QXX_DBG_LOG("w25qxx EraseSector done after %d ms\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(1);
	mem->Lock = 0;
}
//###################################################################################################################
void W25qxx_EraseBlock(w25qxx_t *mem, uint32_t BlockAddr)
{
	uint8_t cmd[5] = {0};
	uint8_t cmd_len = 0;
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
#if (_W25QXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	W25QXX_DBG_LOG("w25qxx EraseSector %d Begin...\r\n", SectorAddr);
#endif
	W25qxx_WaitForWriteEnd(mem);
	BlockAddr = BlockAddr * mem->SectorSize;
	W25qxx_WriteEnable(mem);
	if (mem->ID >= W25Q256){
		cmd[cmd_len] = 0x21;
		cmd_len ++;
		cmd[cmd_len] = (BlockAddr & 0xFF000000) >> 24;
		cmd_len ++;
	}else{
		cmd[cmd_len] = 0xD8;
		cmd_len ++;
	}
	cmd[cmd_len] = (BlockAddr & 0xFF0000) >> 16;
	cmd_len ++;
	cmd[cmd_len] = (BlockAddr & 0xFF00) >> 8;
	cmd_len ++;
	cmd[cmd_len] = (BlockAddr & 0xFF);
	cmd_len ++;

	MEM_SELECT(mem->cs_port, mem->cs_pin);
	spi_write(mem->spi, cmd, cmd_len);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);

	W25qxx_WaitForWriteEnd(mem);
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx EraseBlock done after %d ms\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(100);
#endif
	W25qxx_Delay(1);
	mem->Lock = 0;
}
//###################################################################################################################
uint32_t W25qxx_PageToSector(w25qxx_t *mem, uint32_t PageAddress)
{
	return ((PageAddress * mem->PageSize) / mem->SectorSize);
}
//###################################################################################################################
uint32_t W25qxx_PageToBlock(w25qxx_t *mem, uint32_t PageAddress)
{
	return ((PageAddress * mem->PageSize) / mem->BlockSize);
}
//###################################################################################################################
uint32_t W25qxx_SectorToBlock(w25qxx_t *mem, uint32_t SectorAddress)
{
	return ((SectorAddress * mem->SectorSize) / mem->BlockSize);
}
//###################################################################################################################
uint32_t W25qxx_SectorToPage(w25qxx_t *mem, uint32_t SectorAddress)
{
	return (SectorAddress * mem->SectorSize) / mem->PageSize;
}
//###################################################################################################################
uint32_t W25qxx_BlockToPage(w25qxx_t *mem, uint32_t BlockAddress)
{
	return (BlockAddress * mem->BlockSize) / mem->PageSize;
}
//###################################################################################################################
bool W25qxx_IsEmptyPage(w25qxx_t *mem, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize)
{
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
	if (((NumByteToCheck_up_to_PageSize + OffsetInByte) > mem->PageSize) || (NumByteToCheck_up_to_PageSize == 0))
		NumByteToCheck_up_to_PageSize = mem->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckPage:%d, Offset:%d, Bytes:%d begin...\r\n", Page_Address, OffsetInByte, NumByteToCheck_up_to_PageSize);
	W25qxx_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	uint8_t pBuffer[32];
	uint32_t WorkAddress;
	uint32_t i;
	for (i = OffsetInByte; i < mem->PageSize; i += sizeof(pBuffer))
	{
		MEM_SELECT(mem->cs_port, mem->cs_pin);
		WorkAddress = (i + Page_Address * mem->PageSize);
		if (mem->ID >= W25Q256)
		{
			W25qxx_Spi(0x0C);
			W25qxx_Spi((WorkAddress & 0xFF000000) >> 24);
		}
		else
		{
			W25qxx_Spi(0x0B);
		}
		W25qxx_Spi((WorkAddress & 0xFF0000) >> 16);
		W25qxx_Spi((WorkAddress & 0xFF00) >> 8);
		W25qxx_Spi(WorkAddress & 0xFF);
		W25qxx_Spi(0);
		HAL_SPI_Receive(&_W25QXX_SPI, pBuffer, sizeof(pBuffer), 100);
		MEM_DESELECT(mem->cs_port, mem->cs_pin);
		for (uint8_t x = 0; x < sizeof(pBuffer); x++)
		{
			if (pBuffer[x] != 0xFF)
				goto NOT_EMPTY;
		}
	}
	if ((mem->PageSize + OffsetInByte) % sizeof(pBuffer) != 0)
	{
		i -= sizeof(pBuffer);
		for (; i < mem->PageSize; i++)
		{
			MEM_SELECT(mem->cs_port, mem->cs_pin);
			WorkAddress = (i + Page_Address * mem->PageSize);
			W25qxx_Spi(0x0B);
			if (mem->ID >= W25Q256)
			{
				W25qxx_Spi(0x0C);
				W25qxx_Spi((WorkAddress & 0xFF000000) >> 24);
			}
			else
			{
				W25qxx_Spi(0x0B);
			}
			W25qxx_Spi((WorkAddress & 0xFF0000) >> 16);
			W25qxx_Spi((WorkAddress & 0xFF00) >> 8);
			W25qxx_Spi(WorkAddress & 0xFF);
			W25qxx_Spi(0);
			HAL_SPI_Receive(&_W25QXX_SPI, pBuffer, 1, 100);
			MEM_DESELECT(mem->cs_port, mem->cs_pin);
			if (pBuffer[0] != 0xFF)
				goto NOT_EMPTY;
		}
	}
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckPage is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(100);
#endif
	mem->Lock = 0;
	return true;
NOT_EMPTY:
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckPage is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(100);
#endif
	mem->Lock = 0;
	return false;
}
//###################################################################################################################
bool W25qxx_IsEmptySector(w25qxx_t *mem, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize)
{
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
	if ((NumByteToCheck_up_to_SectorSize > mem->SectorSize) || (NumByteToCheck_up_to_SectorSize == 0))
		NumByteToCheck_up_to_SectorSize = mem->SectorSize;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckSector:%d, Offset:%d, Bytes:%d begin...\r\n", Sector_Address, OffsetInByte, NumByteToCheck_up_to_SectorSize);
	W25qxx_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	uint8_t pBuffer[32];
	uint32_t WorkAddress;
	uint32_t i;
	for (i = OffsetInByte; i < mem->SectorSize; i += sizeof(pBuffer))
	{
		MEM_SELECT(mem->cs_port, mem->cs_pin);
		WorkAddress = (i + Sector_Address * mem->SectorSize);
		W25qxx_Spi(0x0B);
		if (mem->ID >= W25Q256)
		{
			W25qxx_Spi(0x0C);
			W25qxx_Spi((WorkAddress & 0xFF000000) >> 24);
		}
		else
		{
			W25qxx_Spi(0x0B);
		}
		W25qxx_Spi((WorkAddress & 0xFF0000) >> 16);
		W25qxx_Spi((WorkAddress & 0xFF00) >> 8);
		W25qxx_Spi(WorkAddress & 0xFF);
		W25qxx_Spi(0);
		HAL_SPI_Receive(&_W25QXX_SPI, pBuffer, sizeof(pBuffer), 100);
		MEM_DESELECT(mem->cs_port, mem->cs_pin);
		for (uint8_t x = 0; x < sizeof(pBuffer); x++)
		{
			if (pBuffer[x] != 0xFF)
				goto NOT_EMPTY;
		}
	}
	if ((mem->SectorSize + OffsetInByte) % sizeof(pBuffer) != 0)
	{
		i -= sizeof(pBuffer);
		for (; i < mem->SectorSize; i++)
		{
			MEM_SELECT(mem->cs_port, mem->cs_pin);
			WorkAddress = (i + Sector_Address * mem->SectorSize);
			if (mem->ID >= W25Q256)
			{
				W25qxx_Spi(0x0C);
				W25qxx_Spi((WorkAddress & 0xFF000000) >> 24);
			}
			else
			{
				W25qxx_Spi(0x0B);
			}
			W25qxx_Spi((WorkAddress & 0xFF0000) >> 16);
			W25qxx_Spi((WorkAddress & 0xFF00) >> 8);
			W25qxx_Spi(WorkAddress & 0xFF);
			W25qxx_Spi(0);
			HAL_SPI_Receive(&_W25QXX_SPI, pBuffer, 1, 100);
			MEM_DESELECT(mem->cs_port, mem->cs_pin);
			if (pBuffer[0] != 0xFF)
				goto NOT_EMPTY;
		}
	}
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckSector is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(100);
#endif
	mem->Lock = 0;
	return true;
NOT_EMPTY:
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckSector is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(100);
#endif
	mem->Lock = 0;
	return false;
}
//###################################################################################################################
bool W25qxx_IsEmptyBlock(w25qxx_t *mem, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize)
{
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
	if ((NumByteToCheck_up_to_BlockSize > mem->BlockSize) || (NumByteToCheck_up_to_BlockSize == 0))
		NumByteToCheck_up_to_BlockSize = mem->BlockSize;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckBlock:%d, Offset:%d, Bytes:%d begin...\r\n", Block_Address, OffsetInByte, NumByteToCheck_up_to_BlockSize);
	W25qxx_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	uint8_t pBuffer[32];
	uint32_t WorkAddress;
	uint32_t i;
	for (i = OffsetInByte; i < mem->BlockSize; i += sizeof(pBuffer))
	{
		MEM_SELECT(mem->cs_port, mem->cs_pin);
		WorkAddress = (i + Block_Address * mem->BlockSize);

		if (mem->ID >= W25Q256)
		{
			W25qxx_Spi(0x0C);
			W25qxx_Spi((WorkAddress & 0xFF000000) >> 24);
		}
		else
		{
			W25qxx_Spi(0x0B);
		}
		W25qxx_Spi((WorkAddress & 0xFF0000) >> 16);
		W25qxx_Spi((WorkAddress & 0xFF00) >> 8);
		W25qxx_Spi(WorkAddress & 0xFF);
		W25qxx_Spi(0);
		HAL_SPI_Receive(&_W25QXX_SPI, pBuffer, sizeof(pBuffer), 100);
		MEM_DESELECT(mem->cs_port, mem->cs_pin);
		for (uint8_t x = 0; x < sizeof(pBuffer); x++)
		{
			if (pBuffer[x] != 0xFF)
				goto NOT_EMPTY;
		}
	}
	if ((mem->BlockSize + OffsetInByte) % sizeof(pBuffer) != 0)
	{
		i -= sizeof(pBuffer);
		for (; i < mem->BlockSize; i++)
		{
			MEM_SELECT(mem->cs_port, mem->cs_pin);
			WorkAddress = (i + Block_Address * mem->BlockSize);

			if (mem->ID >= W25Q256)
			{
				W25qxx_Spi(0x0C);
				W25qxx_Spi((WorkAddress & 0xFF000000) >> 24);
			}
			else
			{
				W25qxx_Spi(0x0B);
			}
			W25qxx_Spi((WorkAddress & 0xFF0000) >> 16);
			W25qxx_Spi((WorkAddress & 0xFF00) >> 8);
			W25qxx_Spi(WorkAddress & 0xFF);
			W25qxx_Spi(0);
			HAL_SPI_Receive(&_W25QXX_SPI, pBuffer, 1, 100);
			MEM_DESELECT(mem->cs_port, mem->cs_pin);
			if (pBuffer[0] != 0xFF)
				goto NOT_EMPTY;
		}
	}
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckBlock is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(100);
#endif
	mem->Lock = 0;
	return true;
NOT_EMPTY:
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx CheckBlock is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
	W25qxx_Delay(100);
#endif
	mem->Lock = 0;
	return false;
}
//###################################################################################################################
void W25qxx_WriteByte(w25qxx_t *mem, uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
#if (_W25QXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	W25QXX_DBG_LOG("w25qxx WriteByte 0x%02X at address %d begin...", pBuffer, WriteAddr_inBytes);
#endif
	W25qxx_WaitForWriteEnd();
	W25qxx_WriteEnable();
	MEM_SELECT(mem->cs_port, mem->cs_pin);

	if (mem->ID >= W25Q256)
	{
		W25qxx_Spi(0x12);
		W25qxx_Spi((WriteAddr_inBytes & 0xFF000000) >> 24);
	}
	else
	{
		W25qxx_Spi(0x02);
	}
	W25qxx_Spi((WriteAddr_inBytes & 0xFF0000) >> 16);
	W25qxx_Spi((WriteAddr_inBytes & 0xFF00) >> 8);
	W25qxx_Spi(WriteAddr_inBytes & 0xFF);
	W25qxx_Spi(pBuffer);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
	W25qxx_WaitForWriteEnd();
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx WriteByte done after %d ms\r\n", HAL_GetTick() - StartTime);
#endif
	mem->Lock = 0;
}
//###################################################################################################################
void W25qxx_WritePage(w25qxx_t *mem, uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize)
{
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
	if (((NumByteToWrite_up_to_PageSize + OffsetInByte) > mem->PageSize) || (NumByteToWrite_up_to_PageSize == 0))
		NumByteToWrite_up_to_PageSize = mem->PageSize - OffsetInByte;
	if ((OffsetInByte + NumByteToWrite_up_to_PageSize) > mem->PageSize)
		NumByteToWrite_up_to_PageSize = mem->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx WritePage:%d, Offset:%d ,Writes %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToWrite_up_to_PageSize);
	W25qxx_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	W25qxx_WaitForWriteEnd();
	W25qxx_WriteEnable();
	MEM_SELECT(mem->cs_port, mem->cs_pin);
	Page_Address = (Page_Address * mem->PageSize) + OffsetInByte;
	if (mem->ID >= W25Q256)
	{
		W25qxx_Spi(0x12);
		W25qxx_Spi((Page_Address & 0xFF000000) >> 24);
	}
	else
	{
		W25qxx_Spi(0x02);
	}
	W25qxx_Spi((Page_Address & 0xFF0000) >> 16);
	W25qxx_Spi((Page_Address & 0xFF00) >> 8);
	W25qxx_Spi(Page_Address & 0xFF);
	HAL_SPI_Transmit(&_W25QXX_SPI, pBuffer, NumByteToWrite_up_to_PageSize, 100);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
	W25qxx_WaitForWriteEnd();
#if (_W25QXX_DEBUG == 1)
	StartTime = HAL_GetTick() - StartTime;
	for (uint32_t i = 0; i < NumByteToWrite_up_to_PageSize; i++)
	{
		if ((i % 8 == 0) && (i > 2))
		{
			W25QXX_DBG_LOG("\r\n");
			W25qxx_Delay(10);
		}
		W25QXX_DBG_LOG("0x%02X,", pBuffer[i]);
	}
	W25QXX_DBG_LOG("\r\n");
	W25QXX_DBG_LOG("w25qxx WritePage done after %d ms\r\n", StartTime);
	W25qxx_Delay(100);
#endif
	W25qxx_Delay(1);
	mem->Lock = 0;
}
//###################################################################################################################
void W25qxx_WriteSector(w25qxx_t *mem, uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize)
{
	if ((NumByteToWrite_up_to_SectorSize > mem->SectorSize) || (NumByteToWrite_up_to_SectorSize == 0))
		NumByteToWrite_up_to_SectorSize = mem->SectorSize;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("+++w25qxx WriteSector:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToWrite_up_to_SectorSize);
	W25qxx_Delay(100);
#endif
	if (OffsetInByte >= mem->SectorSize)
	{
#if (_W25QXX_DEBUG == 1)
		W25QXX_DBG_LOG("---w25qxx WriteSector Faild!\r\n");
		W25qxx_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToWrite;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToWrite_up_to_SectorSize) > mem->SectorSize)
		BytesToWrite = mem->SectorSize - OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_SectorSize;
	StartPage = W25qxx_SectorToPage(Sector_Address) + (OffsetInByte / mem->PageSize);
	LocalOffset = OffsetInByte % mem->PageSize;
	do
	{
		W25qxx_WritePage(pBuffer, StartPage, LocalOffset, BytesToWrite);
		StartPage++;
		BytesToWrite -= mem->PageSize - LocalOffset;
		pBuffer += mem->PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToWrite > 0);
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("---w25qxx WriteSector Done\r\n");
	W25qxx_Delay(100);
#endif
}
//###################################################################################################################
void W25qxx_WriteBlock(w25qxx_t *mem, uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize)
{
	if ((NumByteToWrite_up_to_BlockSize > mem->BlockSize) || (NumByteToWrite_up_to_BlockSize == 0))
		NumByteToWrite_up_to_BlockSize = mem->BlockSize;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("+++w25qxx WriteBlock:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToWrite_up_to_BlockSize);
	W25qxx_Delay(100);
#endif
	if (OffsetInByte >= mem->BlockSize)
	{
#if (_W25QXX_DEBUG == 1)
		W25QXX_DBG_LOG("---w25qxx WriteBlock Faild!\r\n");
		W25qxx_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToWrite;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToWrite_up_to_BlockSize) > mem->BlockSize)
		BytesToWrite = mem->BlockSize - OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_BlockSize;
	StartPage = W25qxx_BlockToPage(Block_Address) + (OffsetInByte / mem->PageSize);
	LocalOffset = OffsetInByte % mem->PageSize;
	do
	{
		W25qxx_WritePage(pBuffer, StartPage, LocalOffset, BytesToWrite);
		StartPage++;
		BytesToWrite -= mem->PageSize - LocalOffset;
		pBuffer += mem->PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToWrite > 0);
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("---w25qxx WriteBlock Done\r\n");
	W25qxx_Delay(100);
#endif
}
//###################################################################################################################
void W25qxx_ReadByte(w25qxx_t *mem, uint8_t *pBuffer, uint32_t Bytes_Address)
{
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
#if (_W25QXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	W25QXX_DBG_LOG("w25qxx ReadByte at address %d begin...\r\n", Bytes_Address);
#endif
	MEM_SELECT(mem->cs_port, mem->cs_pin);

	if (mem->ID >= W25Q256)
	{
		W25qxx_Spi(0x0C);
		W25qxx_Spi((Bytes_Address & 0xFF000000) >> 24);
	}
	else
	{
		W25qxx_Spi(0x0B);
	}
	W25qxx_Spi((Bytes_Address & 0xFF0000) >> 16);
	W25qxx_Spi((Bytes_Address & 0xFF00) >> 8);
	W25qxx_Spi(Bytes_Address & 0xFF);
	W25qxx_Spi(0);
	*pBuffer = W25qxx_Spi(W25QXX_DUMMY_BYTE);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx ReadByte 0x%02X done after %d ms\r\n", *pBuffer, HAL_GetTick() - StartTime);
#endif
	mem->Lock = 0;
}
//###################################################################################################################
void W25qxx_ReadBytes(w25qxx_t *mem, uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
#if (_W25QXX_DEBUG == 1)
	uint32_t StartTime = HAL_GetTick();
	W25QXX_DBG_LOG("w25qxx ReadBytes at Address:%d, %d Bytes  begin...\r\n", ReadAddr, NumByteToRead);
#endif
	MEM_SELECT(mem->cs_port, mem->cs_pin);

	if (mem->ID >= W25Q256)
	{
		W25qxx_Spi(0x0C);
		W25qxx_Spi((ReadAddr & 0xFF000000) >> 24);
	}
	else
	{
		W25qxx_Spi(0x0B);
	}
	W25qxx_Spi((ReadAddr & 0xFF0000) >> 16);
	W25qxx_Spi((ReadAddr & 0xFF00) >> 8);
	W25qxx_Spi(ReadAddr & 0xFF);
	W25qxx_Spi(0);
	HAL_SPI_Receive(&_W25QXX_SPI, pBuffer, NumByteToRead, 2000);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
#if (_W25QXX_DEBUG == 1)
	StartTime = HAL_GetTick() - StartTime;
	for (uint32_t i = 0; i < NumByteToRead; i++)
	{
		if ((i % 8 == 0) && (i > 2))
		{
			W25QXX_DBG_LOG("\r\n");
			W25qxx_Delay(10);
		}
		W25QXX_DBG_LOG("0x%02X,", pBuffer[i]);
	}
	W25QXX_DBG_LOG("\r\n");
	W25QXX_DBG_LOG("w25qxx ReadBytes done after %d ms\r\n", StartTime);
	W25qxx_Delay(100);
#endif
	W25qxx_Delay(1);
	mem->Lock = 0;
}
//###################################################################################################################
void W25qxx_ReadPage(w25qxx_t *mem, uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize)
{
	W25qxx_WaitLock(mem);
	mem->Lock = 1;
	if ((NumByteToRead_up_to_PageSize > mem->PageSize) || (NumByteToRead_up_to_PageSize == 0))
		NumByteToRead_up_to_PageSize = mem->PageSize;
	if ((OffsetInByte + NumByteToRead_up_to_PageSize) > mem->PageSize)
		NumByteToRead_up_to_PageSize = mem->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("w25qxx ReadPage:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToRead_up_to_PageSize);
	W25qxx_Delay(100);
	uint32_t StartTime = HAL_GetTick();
#endif
	Page_Address = Page_Address * mem->PageSize + OffsetInByte;
	MEM_SELECT(mem->cs_port, mem->cs_pin);
	if (mem->ID >= W25Q256)
	{
		W25qxx_Spi(0x0C);
		W25qxx_Spi((Page_Address & 0xFF000000) >> 24);
	}
	else
	{
		W25qxx_Spi(0x0B);
	}
	W25qxx_Spi((Page_Address & 0xFF0000) >> 16);
	W25qxx_Spi((Page_Address & 0xFF00) >> 8);
	W25qxx_Spi(Page_Address & 0xFF);
	W25qxx_Spi(0);
	HAL_SPI_Receive(&_W25QXX_SPI, pBuffer, NumByteToRead_up_to_PageSize, 100);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
#if (_W25QXX_DEBUG == 1)
	StartTime = HAL_GetTick() - StartTime;
	for (uint32_t i = 0; i < NumByteToRead_up_to_PageSize; i++)
	{
		if ((i % 8 == 0) && (i > 2))
		{
			W25QXX_DBG_LOG("\r\n");
			W25qxx_Delay(10);
		}
		W25QXX_DBG_LOG("0x%02X,", pBuffer[i]);
	}
	W25QXX_DBG_LOG("\r\n");
	W25QXX_DBG_LOG("w25qxx ReadPage done after %d ms\r\n", StartTime);
	W25qxx_Delay(100);
#endif
	W25qxx_Delay(1);
	mem->Lock = 0;
}
//###################################################################################################################
void W25qxx_ReadSector(w25qxx_t *mem, uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize)
{
	if ((NumByteToRead_up_to_SectorSize > mem->SectorSize) || (NumByteToRead_up_to_SectorSize == 0))
		NumByteToRead_up_to_SectorSize = mem->SectorSize;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("+++w25qxx ReadSector:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToRead_up_to_SectorSize);
	W25qxx_Delay(100);
#endif
	if (OffsetInByte >= mem->SectorSize)
	{
#if (_W25QXX_DEBUG == 1)
		W25QXX_DBG_LOG("---w25qxx ReadSector Faild!\r\n");
		W25qxx_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToRead;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToRead_up_to_SectorSize) > mem->SectorSize)
		BytesToRead = mem->SectorSize - OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_SectorSize;
	StartPage = W25qxx_SectorToPage(Sector_Address) + (OffsetInByte / mem->PageSize);
	LocalOffset = OffsetInByte % mem->PageSize;
	do
	{
		W25qxx_ReadPage(pBuffer, StartPage, LocalOffset, BytesToRead);
		StartPage++;
		BytesToRead -= mem->PageSize - LocalOffset;
		pBuffer += mem->PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToRead > 0);
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("---w25qxx ReadSector Done\r\n");
	W25qxx_Delay(100);
#endif
}
//###################################################################################################################
void W25qxx_ReadBlock(w25qxx_t *mem, uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize)
{
	if ((NumByteToRead_up_to_BlockSize > mem->BlockSize) || (NumByteToRead_up_to_BlockSize == 0))
		NumByteToRead_up_to_BlockSize = mem->BlockSize;
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("+++w25qxx ReadBlock:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToRead_up_to_BlockSize);
	W25qxx_Delay(100);
#endif
	if (OffsetInByte >= mem->BlockSize)
	{
#if (_W25QXX_DEBUG == 1)
		W25QXX_DBG_LOG("w25qxx ReadBlock Faild!\r\n");
		W25qxx_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToRead;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToRead_up_to_BlockSize) > mem->BlockSize)
		BytesToRead = mem->BlockSize - OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_BlockSize;
	StartPage = W25qxx_BlockToPage(Block_Address) + (OffsetInByte / mem->PageSize);
	LocalOffset = OffsetInByte % mem->PageSize;
	do
	{
		W25qxx_ReadPage(pBuffer, StartPage, LocalOffset, BytesToRead);
		StartPage++;
		BytesToRead -= mem->PageSize - LocalOffset;
		pBuffer += mem->PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToRead > 0);
#if (_W25QXX_DEBUG == 1)
	W25QXX_DBG_LOG("---w25qxx ReadBlock Done\r\n");
	W25qxx_Delay(100);
#endif
}

//###################################################################################################################
static uint32_t W25qxx_ReadID(w25qxx_t *mem)
{
	uint32_t id = 0;
	uint32_t buffer[4] = {0x9F ,0x00, 0x00, 0x00};
	MEM_SELECT(mem->cs_port, mem->cs_pin);
	spi_write_then_read(mem->spi, &buffer[0], 1, &buffer[1], 3);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
	id = (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
	return id;
}
//###################################################################################################################
static void W25qxx_ReadUniqID(w25qxx_t *mem)
{
	uint8_t i;
	uint8_t id[8];
	
	id[0] = 0x4B;
	for(i = 0; i < 4; i ++){
		id[i + 1] = W25QXX_DUMMY_BYTE;
	}

	MEM_SELECT(mem->cs_port, mem->cs_pin);
	spi_write_then_read(mem->spi, id, 5, id, 8);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
	for(i = 0; i < 8; i ++){
		mem->UniqID[i] = id[i];
	}
}
//###################################################################################################################
static void W25qxx_WriteEnable(w25qxx_t *mem)
{
	uint8_t data = 0x06;
	MEM_SELECT(mem->cs_port, mem->cs_pin);
	spi_write(mem->spi, &data, 1);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
	W25qxx_Delay(1);
}
//###################################################################################################################
static void W25qxx_WriteDisable(w25qxx_t *mem)
{
	uint8_t data = 0x04;
	MEM_SELECT(mem->cs_port, mem->cs_pin);
	spi_write(mem->spi, &data, 1);
	MEM_DESELECT(mem->cs_port, mem->cs_pin);
	W25qxx_Delay(1);
}
//###################################################################################################################
static uint8_t W25qxx_ReadStatusRegister(w25qxx_t *mem, W25QXX_StatusRegisters_t register)
{
	uint8_t status = 0;
	uint8_t register_address = 0;
	switch (register) {
		case STATUS_REGISTER_1:
			register_address = 0x05;
			break;
		case STATUS_REGISTER_2:
			register_address = 0x35;
			break;
		case STATUS_REGISTER_3:
			register_address = 0x15;
			break;
		default:
			break;
	}
	if(0 != register_address){
		MEM_SELECT(mem->cs_port, mem->cs_pin);
		spi_write_then_read(mem->spi, &register_address, 1, &status, 1);
		MEM_DESELECT(mem->cs_port, mem->cs_pin);
		mem->StatusRegisters[register] = status;
	}
	return status;
}
//###################################################################################################################
static void W25qxx_WriteStatusRegister(w25qxx_t *mem, W25QXX_StatusRegisters_t register, uint8_t status)
{
	uint8_t data[2] = {0};
	uint8_t register_address = 0;
	switch (register) {
		case STATUS_REGISTER_1:
			data[0] = 0x01;
			break;
		case STATUS_REGISTER_2:
			data[0] = 0x31;
			break;
		case STATUS_REGISTER_3:
			data[0] = 0x11;
			break;
		default:
			break;
	}
	if(0 != data[0]){
		MEM_SELECT(mem->cs_port, mem->cs_pin);
		spi_write(mem->spi, data, 2);
		MEM_DESELECT(mem->cs_port, mem->cs_pin);
	}
}
//###################################################################################################################
static void W25qxx_WaitForWriteEnd(w25qxx_t *mem)
{
	W25QXX_StatusRegister1_t status;
	W25qxx_Delay(1);
	while (1){
		MEM_SELECT(mem->cs_port, mem->cs_pin);
		W25qxx_ReadStatusRegister(mem, STATUS_REGISTER_1);
		status.value = mem->StatusRegisters[STATUS_REGISTER_1];
		MEM_DESELECT(mem->cs_port, mem->cs_pin);
		if(0 == status.BUSY){
			break;
		}
		W25qxx_Delay(1);
	}
}

static void W25qxx_WaitLock(w25qxx_t *mem){
	while(1 == mem->Lock){
		W25qxx_Delay(1);
	}
}