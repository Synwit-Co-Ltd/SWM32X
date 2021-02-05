/**
 * @file indev.h
 * 
 */

#ifndef INDEV_H
#define INDEV_H

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void touchpad_init(void);
void touchpad_setstate( int16_t xp, int16_t yp, uint16_t state);
void touchpad_process(void);
/**********************
 *      MACROS
 **********************/

#endif
