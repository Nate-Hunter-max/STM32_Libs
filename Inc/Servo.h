/*
 * Servo.h
 *
 *  Created on: Jun 23, 2024
 *      Author: Nate Hunter
 */

#pragma once
#include "main.h"

#define SERVO_MAX_ANGLE 180
#define MIN_ANGLE_T 25
#define MAX_ANGLE_T 110

void Servo_Init(TIM_HandleTypeDef *tim, uint16_t ch);
void Servo_SetAngle(float angle);
