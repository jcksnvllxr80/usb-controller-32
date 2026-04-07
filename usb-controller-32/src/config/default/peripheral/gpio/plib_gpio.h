/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


/*** Macros for GREEN_LED pin ***/
#define GREEN_LED_Set()               (LATBSET = (1<<0))
#define GREEN_LED_Clear()             (LATBCLR = (1<<0))
#define GREEN_LED_Toggle()            (LATBINV= (1<<0))
#define GREEN_LED_OutputEnable()      (TRISBCLR = (1<<0))
#define GREEN_LED_InputEnable()       (TRISBSET = (1<<0))
#define GREEN_LED_Get()               ((PORTB >> 0) & 0x1)
#define GREEN_LED_GetLatch()          ((LATB >> 0) & 0x1)
#define GREEN_LED_PIN                  GPIO_PIN_RB0

/*** Macros for RED_LED pin ***/
#define RED_LED_Set()               (LATBSET = (1<<1))
#define RED_LED_Clear()             (LATBCLR = (1<<1))
#define RED_LED_Toggle()            (LATBINV= (1<<1))
#define RED_LED_OutputEnable()      (TRISBCLR = (1<<1))
#define RED_LED_InputEnable()       (TRISBSET = (1<<1))
#define RED_LED_Get()               ((PORTB >> 1) & 0x1)
#define RED_LED_GetLatch()          ((LATB >> 1) & 0x1)
#define RED_LED_PIN                  GPIO_PIN_RB1

/*** Macros for POT_1 pin ***/
#define POT_1_Get()               ((PORTB >> 2) & 0x1)
#define POT_1_GetLatch()          ((LATB >> 2) & 0x1)
#define POT_1_PIN                  GPIO_PIN_RB2

/*** Macros for POT_2 pin ***/
#define POT_2_Get()               ((PORTB >> 3) & 0x1)
#define POT_2_GetLatch()          ((LATB >> 3) & 0x1)
#define POT_2_PIN                  GPIO_PIN_RB3

/*** Macros for INTERRUPT_A pin ***/
#define INTERRUPT_A_Set()               (LATBSET = (1<<5))
#define INTERRUPT_A_Clear()             (LATBCLR = (1<<5))
#define INTERRUPT_A_Toggle()            (LATBINV= (1<<5))
#define INTERRUPT_A_OutputEnable()      (TRISBCLR = (1<<5))
#define INTERRUPT_A_InputEnable()       (TRISBSET = (1<<5))
#define INTERRUPT_A_Get()               ((PORTB >> 5) & 0x1)
#define INTERRUPT_A_GetLatch()          ((LATB >> 5) & 0x1)
#define INTERRUPT_A_PIN                  GPIO_PIN_RB5
#define INTERRUPT_A_InterruptEnable()   (CNENBSET = (1<<5))
#define INTERRUPT_A_InterruptDisable()  (CNENBCLR = (1<<5))

/*** Macros for INTERRUPT_B pin ***/
#define INTERRUPT_B_Set()               (LATBSET = (1<<7))
#define INTERRUPT_B_Clear()             (LATBCLR = (1<<7))
#define INTERRUPT_B_Toggle()            (LATBINV= (1<<7))
#define INTERRUPT_B_OutputEnable()      (TRISBCLR = (1<<7))
#define INTERRUPT_B_InputEnable()       (TRISBSET = (1<<7))
#define INTERRUPT_B_Get()               ((PORTB >> 7) & 0x1)
#define INTERRUPT_B_GetLatch()          ((LATB >> 7) & 0x1)
#define INTERRUPT_B_PIN                  GPIO_PIN_RB7
#define INTERRUPT_B_InterruptEnable()   (CNENBSET = (1<<7))
#define INTERRUPT_B_InterruptDisable()  (CNENBCLR = (1<<7))

/*** Macros for BUTTON_1 pin ***/
#define BUTTON_1_Set()               (LATBSET = (1<<14))
#define BUTTON_1_Clear()             (LATBCLR = (1<<14))
#define BUTTON_1_Toggle()            (LATBINV= (1<<14))
#define BUTTON_1_OutputEnable()      (TRISBCLR = (1<<14))
#define BUTTON_1_InputEnable()       (TRISBSET = (1<<14))
#define BUTTON_1_Get()               ((PORTB >> 14) & 0x1)
#define BUTTON_1_GetLatch()          ((LATB >> 14) & 0x1)
#define BUTTON_1_PIN                  GPIO_PIN_RB14
#define BUTTON_1_InterruptEnable()   (CNENBSET = (1<<14))
#define BUTTON_1_InterruptDisable()  (CNENBCLR = (1<<14))

/*** Macros for BUTTON_2 pin ***/
#define BUTTON_2_Set()               (LATBSET = (1<<15))
#define BUTTON_2_Clear()             (LATBCLR = (1<<15))
#define BUTTON_2_Toggle()            (LATBINV= (1<<15))
#define BUTTON_2_OutputEnable()      (TRISBCLR = (1<<15))
#define BUTTON_2_InputEnable()       (TRISBSET = (1<<15))
#define BUTTON_2_Get()               ((PORTB >> 15) & 0x1)
#define BUTTON_2_GetLatch()          ((LATB >> 15) & 0x1)
#define BUTTON_2_PIN                  GPIO_PIN_RB15
#define BUTTON_2_InterruptEnable()   (CNENBSET = (1<<15))
#define BUTTON_2_InterruptDisable()  (CNENBCLR = (1<<15))


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/
#define    GPIO_PORT_A  (0U)
#define    GPIO_PORT_B  (1U)
typedef uint32_t GPIO_PORT;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/
#define    GPIO_PIN_RA0  (0U)
#define    GPIO_PIN_RA1  (1U)
#define    GPIO_PIN_RA2  (2U)
#define    GPIO_PIN_RA3  (3U)
#define    GPIO_PIN_RA4  (4U)
#define    GPIO_PIN_RB0  (16U)
#define    GPIO_PIN_RB1  (17U)
#define    GPIO_PIN_RB2  (18U)
#define    GPIO_PIN_RB3  (19U)
#define    GPIO_PIN_RB4  (20U)
#define    GPIO_PIN_RB5  (21U)
#define    GPIO_PIN_RB7  (23U)
#define    GPIO_PIN_RB8  (24U)
#define    GPIO_PIN_RB9  (25U)
#define    GPIO_PIN_RB10  (26U)
#define    GPIO_PIN_RB11  (27U)
#define    GPIO_PIN_RB13  (29U)
#define    GPIO_PIN_RB14  (30U)
#define    GPIO_PIN_RB15  (31U)

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
#define    GPIO_PIN_NONE    (-1)

typedef uint32_t GPIO_PIN;

typedef  void (*GPIO_PIN_CALLBACK) ( GPIO_PIN pin, uintptr_t context);

void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortInterruptEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortInterruptDisable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: Local Data types and Prototypes
// *****************************************************************************
// *****************************************************************************

typedef struct {

    /* target pin */
    GPIO_PIN                 pin;

    /* Callback for event on target pin*/
    GPIO_PIN_CALLBACK        callback;

    /* Callback Context */
    uintptr_t               context;

} GPIO_PIN_CALLBACK_OBJ;

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
    GPIO_PortWrite((GPIO_PORT)(pin>>4), (uint32_t)(0x1) << (pin & 0xFU), (uint32_t)(value) << (pin & 0xFU));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return (bool)(((GPIO_PortRead((GPIO_PORT)(pin>>4))) >> (pin & 0xFU)) & 0x1U);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (bool)((GPIO_PortLatchRead((GPIO_PORT)(pin>>4)) >> (pin & 0xFU)) & 0x1U);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle((GPIO_PORT)(pin>>4), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet((GPIO_PORT)(pin>>4), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear((GPIO_PORT)(pin>>4), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable((GPIO_PORT)(pin>>4), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable((GPIO_PORT)(pin>>4), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinInterruptEnable(GPIO_PIN pin)
{
    GPIO_PortInterruptEnable((GPIO_PORT)(pin>>4), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinInterruptDisable(GPIO_PIN pin)
{
    GPIO_PortInterruptDisable((GPIO_PORT)(pin>>4), 0x1UL << (pin & 0xFU));
}

bool GPIO_PinInterruptCallbackRegister(
    GPIO_PIN pin,
    const   GPIO_PIN_CALLBACK callback,
    uintptr_t context
);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
