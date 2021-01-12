#ifndef __AP_SysClock_H
#define __AP_SysClock_H

#include <stdio.h>
#include <stm32f427xx.h>
#include <AP_HAL/AP_HAL.h>
#include "hal.h"

void timer_init();
void timer_start();

#endif