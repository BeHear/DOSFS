#ifndef DANYA_NE2000_H
#define DANYA_NE2000_H

#include "../include/types.h"

#define NE2000_IO_BASE 0x300
#define NE2000_IRQ     3
#define NE2000_MAC     {0x52, 0x54, 0x00, 0x12, 0x34, 0x56}

#define NE2000_BUF_PAGE_START 0x4C
#define NE2000_BUF_PAGE_STOP  0x80
#define NE2000_RX_BUF_PAGES   (NE2000_BUF_PAGE_STOP - NE2000_BUF_PAGE_START)
#define NE2000_TX_PAGE        0x40
#define NE2000_RAM_SIZE       8192
#define NE2000_PAGE_SIZE      256

#define NE2000_REG_CR           0x00
#define NE2000_REG_PSTART       0x01
#define NE2000_REG_PSTOP        0x02
#define NE2000_REG_BNRY         0x03
#define NE2000_REG_TSR          0x04
#define NE2000_REG_TPSR         0x05
#define NE2000_REG_TBCR0        0x06
#define NE2000_REG_TBCR1        0x07
#define NE2000_REG_ISR          0x08
#define NE2000_REG_CRDA0        0x09
#define NE2000_REG_CRDA1        0x0A
#define NE2000_REG_ISR2         0x0B
#define NE2000_REG_RSR          0x0C
#define NE2000_REG_FIFO         0x0D
#define NE2000_REG_CRDA2        0x0E
#define NE2000_REG_CRDA3        0x0F

#define NE2000_REG_DMA          0x10
#define NE2000_REG_DMA2         0x18

#define NE2000_REG_PAGE1_MAC0   0x01
#define NE2000_REG_PAGE1_MAC1   0x02
#define NE2000_REG_PAGE1_MAC2   0x03
#define NE2000_REG_PAGE1_MAC3   0x04
#define NE2000_REG_PAGE1_MAC4   0x05
#define NE2000_REG_PAGE1_MAC5   0x06
#define NE2000_REG_CURR         0x07
#define NE2000_REG_MAR0         0x08
#define NE2000_REG_MAR1         0x09
#define NE2000_REG_MAR2         0x0A
#define NE2000_REG_MAR3         0x0B
#define NE2000_REG_MAR4         0x0C
#define NE2000_REG_MAR5         0x0D
#define NE2000_REG_MAR6         0x0E
#define NE2000_REG_MAR7         0x0F

#define NE2000_REG_RCR          0x0C
#define NE2000_REG_TCR          0x0D
#define NE2000_REG_DCR          0x0E
#define NE2000_REG_IMR          0x0F

#define NE2000_CR_STP          0x01
#define NE2000_CR_STA          0x02
#define NE2000_CR_TXP          0x04
#define NE2000_CR_RD0          0x08
#define NE2000_CR_RD1          0x10
#define NE2000_CR_RD2          0x20
#define NE2000_CR_PAGE0        0x00
#define NE2000_CR_PAGE1        0x40
#define NE2000_CR_PAGE2        0x80

#define NE2000_ISR_PRX         0x01
#define NE2000_ISR_PTX         0x02
#define NE2000_ISR_RXE         0x04
#define NE2000_ISR_TXE         0x08
#define NE2000_ISR_OVW         0x10
#define NE2000_ISR_CNT         0x20
#define NE2000_ISR_RDC         0x40
#define NE2000_ISR_RST         0x80

#define NE2000_RCR_AB         0x04
#define NE2000_RCR_AM         0x08
#define NE2000_RCR_PRO        0x10
#define NE2000_RCR_MON        0x20

#define NE2000_TCR_LOOP0      0x02
#define NE2000_TCR_LOOP1      0x04
#define NE2000_TCR_CRC        0x01

#define NE2000_DCR_FT0        0x01
#define NE2000_DCR_FT1        0x02
#define NE2000_DCR_WTS        0x04

#define NE2000_MTU            1500
#define NE2000_RX_QUEUE_SIZE  16

typedef struct {
    uint8_t status;
    uint8_t next_page;
    uint16_t length;
} __attribute__((packed)) ne2000_rx_header_t;

typedef struct {
    uint8_t data[NE2000_MTU];
    uint16_t len;
} ne2000_rx_packet_t;

void ne2000_init(void);
void ne2000_send(const uint8_t* buf, uint16_t len);
int  ne2000_poll(ne2000_rx_packet_t* pkt);
void ne2000_set_mac(const uint8_t* mac);
void ne2000_get_mac(uint8_t* mac);

#endif
