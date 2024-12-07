/*
 * Servo.c
 *
 *  Created on: Jun 23, 2024
 *      Author: Nate Hunter
 */

#include "Servo.h"
static TIM_HandleTypeDef *_tim;

void Servo_Init(TIM_HandleTypeDef *tim, uint16_t ch){
	_tim = tim;
	HAL_TIM_PWM_Start(_tim, ch);
}

void Servo_SetAngle(float angle){
	if(angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;
	_tim->Instance->CCR1 = MIN_ANGLE_T + (angle / SERVO_MAX_ANGLE * (MAX_ANGLE_T - MIN_ANGLE_T));
}
