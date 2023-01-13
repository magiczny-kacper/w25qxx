#ifndef W25QXX_H_
#define W25QXX_H_

/*
  Author:     Nima Askari, Kacper Brzostowski
  WebSite:    http://www.github.com/NimaLTD
  
  Version:    1.1.3
  
  
  Reversion History:
  
  (1.1.3)
  Fix Erase and write sector in w25q256 and w25q512.

  (1.1.2)
  Fix read ID.
  
  (1.1.1)
  Fix some errors.
  
  (1.1.0)
  Fix some errors.
  
  (1.0.0)
  First release.
*/

#include <stdint.h>
#include <stdbool.h>
#include "w25qxxConf.h"

typedef enum W25QXX_ID
{
	W25Q10 = 1,
	W25Q20,
	W25Q40,
	W25Q80,
	W25Q16,
	W25Q32,
	W25Q64,
	W25Q128,
	W25Q256,
	W25Q512,
} W25QXX_ID_t;

typedef struct w25qxx
{
	W25QXX_ID_t ID;
	uint8_t UniqID[8];
	uint16_t PageSize;
	uint32_t PageCount;
	uint32_t SectorSize;
	uint32_t SectorCount;
	uint32_t BlockSize;
	uint32_t BlockCount;
	uint32_t CapacityInKiloByte;
	uint8_t StatusRegisters[3];
	uint8_t Lock;
	port_t cs_port;
	pin_t cs_pin;
	spi_t spi;
} w25qxx_t;

extern w25qxx_t w25qxx;
//############################################################################
// in Page,Sector and block read/write functions, can put 0 to read maximum bytes
//############################################################################

/**
 * @brief Initialize memory - set handle values, read memory info
 * 
 * @param mem Memory handle
 * @return true Initialization successfull
 * @return false Initialization failed
 */
bool W25qxx_Init(w25qxx_t *mem);

/**
 * @brief Erases whole memory
 * 
 * @param mem Memory handle
 */
void W25qxx_EraseChip (w25qxx_t *mem);

/**
 * @brief Erases given memory sector
 * 
 * @param mem Memory handle
 * @param SectorAddr 
 */
void W25qxx_EraseSector (w25qxx_t *mem, uint32_t SectorAddr);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param BlockAddr 
 */
void W25qxx_EraseBlock (w25qxx_t *mem, uint32_t BlockAddr);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param PageAddress 
 * @return uint32_t 
 */
uint32_t W25qxx_PageToSector (w25qxx_t *mem, uint32_t PageAddress);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param PageAddress 
 * @return uint32_t 
 */
uint32_t W25qxx_PageToBlock (w25qxx_t *mem, uint32_t PageAddress);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param SectorAddress 
 * @return uint32_t 
 */
uint32_t W25qxx_SectorToBlock (w25qxx_t *mem, uint32_t SectorAddress);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param SectorAddress 
 * @return uint32_t 
 */
uint32_t W25qxx_SectorToPage (w25qxx_t *mem, uint32_t SectorAddress);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param BlockAddress 
 * @return uint32_t 
 */
uint32_t W25qxx_BlockToPage (w25qxx_t *mem, uint32_t BlockAddress);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param Page_Address 
 * @param OffsetInByte 
 * @param NumByteToCheck_up_to_PageSize 
 * @return true 
 * @return false 
 */
bool W25qxx_IsEmptyPage (w25qxx_t *mem, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param Sector_Address 
 * @param OffsetInByte 
 * @param NumByteToCheck_up_to_SectorSize 
 * @return true 
 * @return false 
 */
bool W25qxx_IsEmptySector (w25qxx_t *mem, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param Block_Address 
 * @param OffsetInByte 
 * @param NumByteToCheck_up_to_BlockSize 
 * @return true 
 * @return false 
 */
bool W25qxx_IsEmptyBlock (w25qxx_t *mem, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param Bytes_Address 
 */
void W25qxx_WriteByte (w25qxx_t *mem, uint8_t pBuffer, uint32_t Bytes_Address);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param Page_Address 
 * @param OffsetInByte 
 * @param NumByteToWrite_up_to_PageSize 
 */
void W25qxx_WritePage (w25qxx_t *mem, uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param Sector_Address 
 * @param OffsetInByte 
 * @param NumByteToWrite_up_to_SectorSize 
 */
void W25qxx_WriteSector (w25qxx_t *mem, uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param Block_Address 
 * @param OffsetInByte 
 * @param NumByteToWrite_up_to_BlockSize 
 */
void W25qxx_WriteBlock (w25qxx_t *mem, uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param Bytes_Address 
 */
void W25qxx_ReadByte (w25qxx_t *mem, uint8_t *pBuffer, uint32_t Bytes_Address);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param ReadAddr 
 * @param NumByteToRead 
 */
void W25qxx_ReadBytes (w25qxx_t *mem, uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param Page_Address 
 * @param OffsetInByte 
 * @param NumByteToRead_up_to_PageSize 
 */
void W25qxx_ReadPage (w25qxx_t *mem, uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param Sector_Address 
 * @param OffsetInByte 
 * @param NumByteToRead_up_to_SectorSize 
 */
void W25qxx_ReadSector (w25qxx_t *mem, uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);

/**
 * @brief 
 * 
 * @param mem Memory handle
 * @param pBuffer 
 * @param Block_Address 
 * @param OffsetInByte 
 * @param NumByteToRead_up_to_BlockSize 
 */
void W25qxx_ReadBlock (w25qxx_t *mem, uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);

#endif /* W25QXX_H_ */
