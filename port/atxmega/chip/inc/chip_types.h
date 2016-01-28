/*
 * chip_types.h
 *
 * Created: 2015-10-09 09:12:41
 *  Author: pmiedzik
 */ 


#ifndef CHIP_TYPES_H_
#define CHIP_TYPES_H_


#include <stdint.h>
#include <stdbool.h>

#include "compiler.h"

//typedef enum {FALSE = 0, TRUE = !FALSE} Bool;
typedef enum {RESET = 0, SET = !RESET} FlagStatus, IntStatus, SetState;
//typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;


#define INLINE inline
/* Static data/function define */
#define STATIC static
/* External data/function define */
#define EXTERN extern

#ifdef __cplusplus
#define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
#define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */


#endif /* CHIP_TYPES_H_ */