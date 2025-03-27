//*****************************************************************************
//
// pwm_interrupt_modified.c - Example demonstrating the PWM interrupt with 1% increment.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

uint32_t g_ui32SysClock;
uint32_t g_ui32PWMIncrement;
uint32_t g_ui32Counter = 0;

void PWM0Gen1IntHandler(void)
{
    MAP_PWMGenIntClear(PWM0_BASE, PWM_GEN_1, PWM_INT_CNT_LOAD);
    
    g_ui32Counter++;
    
    // Incrementa el duty cycle cada 125 interrupciones (0.5s)
    if (g_ui32Counter >= 125) {
        g_ui32Counter = 0; // Reinicia el contador
        
        if ((MAP_PWMPulseWidthGet(PWM0_BASE, PWM_OUT_2) + g_ui32PWMIncrement) <=
            (MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1)))
        {
            MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,
                                 MAP_PWMPulseWidthGet(PWM0_BASE, PWM_OUT_2) +
                                 g_ui32PWMIncrement);
        }
        else
        {
            MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, g_ui32PWMIncrement);
        }
    }
}

void ConfigureUART(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, g_ui32SysClock);
}

int main(void)
{
    uint32_t ui32PWMClockRate;
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_240), 120000000);
    
    ConfigureUART();
    
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    MAP_GPIOPinConfigure(GPIO_PF2_M0PWM2);
    MAP_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
    
    MAP_PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_8);
    ui32PWMClockRate = g_ui32SysClock / 8;
    
    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, (ui32PWMClockRate / 250));
    
    g_ui32PWMIncrement = (MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1) / 100);
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, g_ui32PWMIncrement);
    
    MAP_IntMasterEnable();
    MAP_PWMIntEnable(PWM0_BASE, PWM_INT_GEN_1);
    MAP_PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_1, PWM_INT_CNT_LOAD);
    MAP_IntEnable(INT_PWM0_1);
    MAP_PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);
    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_1);
    
    while(1)
    {
        MAP_SysCtlDelay((g_ui32SysClock / 2) / 3);
    }
}

