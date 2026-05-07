/*
 * magneto.h
 *
 *  Created on: Mar 28, 2026
 *      Author: Thanh Le
 */

#ifndef MAGNETO_H_
#define MAGNETO_H_

#include <stdint.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup MAGNETO
  * @{
  */

/** @defgroup MAGNETO_Exported_Types
  * @{
  */

/** @defgroup MAGNETO_Config_structure  Magnetometer Configuration structure
  * @{
  */
typedef struct
{
  uint8_t Register1;
  uint8_t Register2;
  uint8_t Register3;
  uint8_t Register4;
  uint8_t Register5;
}MAGNETO_InitTypeDef;
/**
  * @}
  */

/** @defgroup MAGNETO_Driver_structure  Magnetometer Driver structure
  * @{
  */
typedef struct
{
  void      (*Init)(MAGNETO_InitTypeDef);
  void      (*DeInit)(void);
  uint8_t   (*ReadID)(void);
  void      (*Reset)(void);
  void      (*LowPower)(void);
  void      (*ConfigIT)(void);
  void      (*EnableIT)(uint8_t);
  void      (*DisableIT)(uint8_t);
  uint8_t   (*ITStatus)(uint16_t);
  void      (*ClearIT)(void);
  void      (*FilterConfig)(uint8_t);
  void      (*FilterCmd)(uint8_t);
  void      (*GetXYZ)(int16_t *);
}MAGNETO_DrvTypeDef;

#endif /* MAGNETO_H_ */
