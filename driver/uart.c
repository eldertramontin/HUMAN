/*
 * uart.c
 *
 *  Created on: 9 de jun de 2018
 *      Author: elder
 */

#include "uart.h"

void uart0_setup(uint32_t baudrate)
{
    UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK

    switch(baudrate)
    {
    case 9600:
        UCA0BR0 = 104;                             // 9600 baud at 16 MHz
        UCA0BR1 = 0x00;
        UCA0MCTLW |= UCOS16 | UCBRF_2 | 0xD600;
        break;
    case 115200:
        UCA0BR0 = 8;                             // 115200 baud at 16 MHz
        UCA0BR1 = 0x00;
        UCA0MCTLW |= UCOS16 | UCBRF_10 | 0xF700;
        break;
    default: //9600
        UCA0BR0 = 104;
        UCA0BR1 = 0x00;
        UCA0MCTLW |= UCOS16 | UCBRF_2 | 0xD600;
        break;
    }

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    __bis_SR_register(GIE);
}

void uart0_write(uint8_t *data, uint8_t length)
{
    while (length-- > 0)
    {
        while (UCA0STATW & UCBUSY);
        UCA0TXBUF = *data;
        data++;
    }
}

void uart0_flush()
{
    uint8_t x;
    while (UCA0IFG & UCRXIFG)
    {
        x = UCA0RXBUF;
    }
}

void uart0_read(uint8_t *data, uint8_t length)
{
    uart0_flush();
    while (length--)
    {
        while (!(UCA0IFG & UCRXIFG));
        *data =  UCA0RXBUF;
        data++;
    }
}


#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
      while(!(UCA0IFG & UCTXIFG));
//      UCA0TXBUF = UCA0RXBUF;
      __no_operation();
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}
