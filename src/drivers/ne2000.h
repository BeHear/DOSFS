#ifndef DANYA_NE2000_H
#define DANYA_NE2000_H

#include "../include/types.h"

#define NE2000_IO_BASE 0x300
#define NE2000_IRQ     3

/* Page 0 registers */
#define NE2000_REG_CR       0x00  /* Command Register (R/W) */
#define NE2000_REG_PSTART   0x01  /* Page Start (W only) */
#define NE2000_REG_PSTOP    0x02  /* Page Stop (W only) */
#define NE2000_REG_BNDRY    0x03  /* Boundary (R/W) */
#define NE2000_REG_TSR      0x04  /* Transmit Status (R only) */
#define NE2000_REG_TPSR     0x05  /* Transmit Page Start (W only) */
#define NE2000_REG_TBCR0    0x06  /* Transmit Byte Count 0 (W only) */
#define NE2000_REG_TBCR1    0x07  /* Transmit Byte Count 1 (W) / ISR (R) */
#define NE2000_REG_ISR      0x07  /* Interrupt Status (R only, same as TBCR1) */
#define NE2000_REG_RSAR0    0x08  /* Remote Start Addr 0 (W) / CRDA0 (R) */
#define NE2000_REG_RSAR1    0x09  /* Remote Start Addr 1 (W) / CRDA1 (R) */
#define NE2000_REG_RBCR0    0x0A  /* Remote Byte Count 0 (W only) */
#define NE2000_REG_RBCR1    0x0B  /* Remote Byte Count 1 (W only) */
#define NE2000_REG_RSR      0x0C  /* Receive Status (R) / RCR (W) */
#define NE2000_REG_RCR      0x0C  /* Receive Config (W only) */
#define NE2000_REG_TCR      0x0D  /* Transmit Config (W only) */
#define NE2000_REG_DCR      0x0E  /* Data Config (W only) */
#define NE2000_REG_IMR      0x0F  /* Interrupt Mask (W only) */
#define NE2000_REG_RDP      0x10  /* Remote Data Port (R/W) */

/* Page 1 registers */
#define NE2000_REG_PAR0     0x01  /* Physical Address 0 */
#define NE2000_REG_CURR     0x07  /* Current Page Register */

/* CR bits */
#define NE2000_CR_STP       0x01
#define NE2000_CR_STA       0x02
#define NE2000_CR_TXP       0x04
#define NE2000_CR_RD0       0x08  /* Remote DMA: read */
#define NE2000_CR_RD1       0x10  /* Remote DMA: write */
#define NE2000_CR_PAGE0     0x00
#define NE2000_CR_PAGE1     0x40

/* ISR bits */
#define NE2000_ISR_PRX      0x01
#define NE2000_ISR_PTX      0x02
#define NE2000_ISR_RXE      0x04
#define NE2000_ISR_TXE      0x08
#define NE2000_ISR_OVW      0x10
#define NE2000_ISR_RDC      0x40
#define NE2000_ISR_RST      0x80

/* DCR bits */
#define NE2000_DCR_WTS      0x01
#define NE2000_DCR_AR       0x10
#define NE2000_DCR_LS       0x20

/* RCR bits */
#define NE2000_RCR_AB       0x04
#define NE2000_RCR_AM       0x08

#define NE2000_BUF_PAGE_START 0x4C
#define NE2000_BUF_PAGE_STOP  0x80
#define NE2000_TX_PAGE        0x40
#define NE2000_PAGE_SIZE      256
#define NE2000_MTU            1500

typedef struct {
    uint8_t data[NE2000_MTU];
    uint16_t len;
} ne2000_rx_packet_t;

void ne2000_init(void);
void ne2000_send(const uint8_t* buf, uint16_t len);
int  ne2000_poll(ne2000_rx_packet_t* pkt);
void ne2000_get_mac(uint8_t* mac);

#endif
