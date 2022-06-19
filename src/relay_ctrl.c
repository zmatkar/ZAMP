/*
 * relay_ctrl.c
 *
 *  Created on: 16. 4. 2022
 *      Author: zmatkar
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "relay_ctrl.h"

void RelayCtrl_Switch(uint16_t index)
{
    switch (index) {
        case 0:
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0);
            break;
        case 1:
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, 0);
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_INT_PIN_2);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0);
            break;
        case 2:
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, 0);
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0);
            break;
        case 3:
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, 0);
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0);
            break;
        default:
            GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, 0);
            GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0);
            GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0);
            break;
    }
}

void RealyCtrl_Init(void)
{

}



