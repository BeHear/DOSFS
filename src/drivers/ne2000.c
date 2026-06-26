#include "ne2000.h"
#include "../include/io.h"
#include "../kernel/idt.h"
#include "../drivers/vga.h"
#include "../drivers/timer.h"
#include "../libc/string.h"

static uint16_t io_base = NE2000_IO_BASE;
static uint8_t  irq_num = NE2000_IRQ;
static uint8_t  mac_addr[6];

static inline uint8_t ne2000_read_reg(uint8_t reg) {
    return inb(io_base + reg);
}

static inline void ne2000_write_reg(uint8_t reg, uint8_t val) {
    outb(io_base + reg, val);
}

static inline void ne2000_set_page(uint8_t page) {
    ne2000_write_reg(NE2000_REG_CR, page);
}

static void ne2000_reset(void) {
    // Software reset: write any value to the reset port (base+0x1F for NE2000, but base+0xF ISR write triggers reset)
    // Actually, the standard reset is to write to the "reset register" which is at base+0x1F for some,
    // but for NE2000 compatible it's typically: outb(base + 0x1F, 0) then read ISR.
    // Simpler: issue STOP command then START.
    ne2000_write_reg(NE2000_REG_CR, NE2000_CR_STP | NE2000_CR_PAGE0);
    for (volatile int i = 0; i < 100; i++);
    ne2000_write_reg(NE2000_REG_CR, NE2000_CR_STA | NE2000_CR_PAGE0);
    for (volatile int i = 0; i < 100; i++);
}

static void ne2000_dma_memcpy(uint8_t* dst, uint16_t src_offset, uint16_t len) {
    // Remote DMA read: set DMA address, then read from DMA data register
    ne2000_write_reg(NE2000_REG_CR, NE2000_CR_RD0 | NE2000_CR_PAGE0);
    ne2000_write_reg(NE2000_REG_DMA,  (uint8_t)(src_offset & 0xFF));
    ne2000_write_reg(NE2000_REG_DMA2, (uint8_t)((src_offset >> 8) & 0xFF));
    ne2000_write_reg(NE2000_REG_TBCR0, (uint8_t)(len & 0xFF));
    ne2000_write_reg(NE2000_REG_TBCR1, (uint8_t)((len >> 8) & 0xFF));
    ne2000_write_reg(NE2000_REG_CR,   NE2000_CR_RD0 | NE2000_CR_PAGE0);

    uint16_t port = io_base + NE2000_REG_DMA;
    for (uint16_t i = 0; i < len; i++) {
        dst[i] = inb(port);
    }
}

static void ne2000_irq_handler(stack_state_t* state) {
    UNUSED(state);
    uint8_t isr = ne2000_read_reg(NE2000_REG_ISR);
    // Packets are handled via ne2000_poll(), just clear interrupt
    ne2000_write_reg(NE2000_REG_CR, NE2000_CR_PAGE0);
    ne2000_write_reg(NE2000_REG_ISR, isr);
}

void ne2000_init(void) {

    ne2000_reset();
    ne2000_set_page(NE2000_CR_PAGE0);

    // Set MAC address
    mac_addr[0] = 0x52; mac_addr[1] = 0x54; mac_addr[2] = 0x00;
    mac_addr[3] = 0x12; mac_addr[4] = 0x34; mac_addr[5] = 0x56;

    ne2000_set_page(NE2000_CR_PAGE1);
    ne2000_write_reg(NE2000_REG_PAGE1_MAC0, mac_addr[0]);
    ne2000_write_reg(NE2000_REG_PAGE1_MAC1, mac_addr[1]);
    ne2000_write_reg(NE2000_REG_PAGE1_MAC2, mac_addr[2]);
    ne2000_write_reg(NE2000_REG_PAGE1_MAC3, mac_addr[3]);
    ne2000_write_reg(NE2000_REG_PAGE1_MAC4, mac_addr[4]);
    ne2000_write_reg(NE2000_REG_PAGE1_MAC5, mac_addr[5]);

    // Clear multicast filter
    for (int i = 0; i < 8; i++) {
        ne2000_write_reg(NE2000_REG_MAR0 + i, 0x00);
    }
    ne2000_write_reg(NE2000_REG_CURR, NE2000_BUF_PAGE_START + 1);

    ne2000_set_page(NE2000_CR_PAGE0);

    // Configure receive buffer
    ne2000_write_reg(NE2000_REG_PSTART, NE2000_BUF_PAGE_START);
    ne2000_write_reg(NE2000_REG_PSTOP,  NE2000_BUF_PAGE_STOP);
    ne2000_write_reg(NE2000_REG_BNRY,    NE2000_BUF_PAGE_START);

    // Receiver: accept broadcast, all physical (no promiscuous)
    ne2000_write_reg(NE2000_REG_RCR, NE2000_RCR_AB | NE2000_RCR_AM);

    // Transmitter: internal xmit, normal loopback off (0x00)
    ne2000_write_reg(NE2000_REG_TCR, 0x00);

    // Data config: 8-byte FIFO threshold, word-wide, 16-bit DMA
    ne2000_write_reg(NE2000_REG_DCR, NE2000_DCR_WTS);

    // Clear all interrupts
    ne2000_write_reg(NE2000_REG_ISR, 0xFF);

    // Enable interrupts: receive, transmit, overwrite
    ne2000_write_reg(NE2000_REG_IMR, NE2000_ISR_PRX | NE2000_ISR_PTX | NE2000_ISR_OVW);

    // Start the NIC
    ne2000_write_reg(NE2000_REG_CR, NE2000_CR_STA | NE2000_CR_PAGE0);

    // Register IRQ handler
    idt_register_handler(irq_num + 32, ne2000_irq_handler);
    uint8_t mask = inb(0x21);
    mask &= ~(1 << irq_num);
    outb(0x21, mask);

    vga_printf("  [ OK ] NE2000: MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
               mac_addr[0], mac_addr[1], mac_addr[2],
               mac_addr[3], mac_addr[4], mac_addr[5]);
}

void ne2000_send(const uint8_t* buf, uint16_t len) {
    if (len > NE2000_MTU) len = NE2000_MTU;

    // Wait for previous TX to complete
    for (int i = 0; i < 100000; i++) {
        if (!(ne2000_read_reg(NE2000_REG_CR) & NE2000_CR_TXP)) break;
    }

    // Write packet to NIC RAM at TX page
    uint16_t offset = NE2000_TX_PAGE * NE2000_PAGE_SIZE;
    uint16_t page_off = offset & 0xFF;
    uint16_t page_no = (offset >> 8) & 0xFF;

    ne2000_write_reg(NE2000_REG_CR, NE2000_CR_PAGE0);
    ne2000_write_reg(NE2000_REG_DMA,  (uint8_t)page_off);
    ne2000_write_reg(NE2000_REG_DMA2, (uint8_t)page_no);
    ne2000_write_reg(NE2000_REG_TBCR0, (uint8_t)(len & 0xFF));
    ne2000_write_reg(NE2000_REG_TBCR1, (uint8_t)((len >> 8) & 0xFF));
    ne2000_write_reg(NE2000_REG_TPSR,  NE2000_TX_PAGE);
    ne2000_write_reg(NE2000_REG_CR,   NE2000_CR_RD1 | NE2000_CR_PAGE0);

    uint16_t port = io_base + NE2000_REG_DMA;
    for (uint16_t i = 0; i < len; i++) {
        outb(port, buf[i]);
    }

    // Issue transmit
    ne2000_write_reg(NE2000_REG_CR, NE2000_CR_TXP | NE2000_CR_PAGE0);

    // Wait for TX complete
    for (int i = 0; i < 100000; i++) {
        uint8_t isr = ne2000_read_reg(NE2000_REG_ISR);
        if (isr & NE2000_ISR_PTX) {
            ne2000_write_reg(NE2000_REG_ISR, NE2000_ISR_PTX);
            break;
        }
    }
}

int ne2000_poll(ne2000_rx_packet_t* pkt) {
    ne2000_set_page(NE2000_CR_PAGE0);

    uint8_t bndry = ne2000_read_reg(NE2000_REG_BNRY);
    uint8_t curr;
    // Read current page register (on page 1 then back)
    ne2000_set_page(NE2000_CR_PAGE1);
    curr = ne2000_read_reg(NE2000_REG_CURR);
    ne2000_set_page(NE2000_CR_PAGE0);

    if (curr == bndry) return 0;

    uint8_t page = bndry + 1;
    if (page >= NE2000_BUF_PAGE_STOP) page = NE2000_BUF_PAGE_START;

    // Read packet header (4 bytes)
    uint8_t hdr[4];
    ne2000_dma_memcpy(hdr, 0, 4);

    uint16_t pkt_len = hdr[2] | ((uint16_t)hdr[3] << 8);
    if (pkt_len > NE2000_MTU + 4) pkt_len = NE2000_MTU + 4;

    uint8_t next_page = hdr[1];

    // Read packet data (skip 4-byte header)
    uint16_t data_len = pkt_len - 4;
    if (data_len > NE2000_MTU) data_len = NE2000_MTU;

    if (data_len > 0) {
        ne2000_dma_memcpy(pkt->data, 4, data_len);
    }
    pkt->len = data_len;

    // Update boundary
    if (next_page >= NE2000_BUF_PAGE_START && next_page < NE2000_BUF_PAGE_STOP) {
        ne2000_write_reg(NE2000_REG_BNRY, next_page - 1);
    } else {
        ne2000_write_reg(NE2000_REG_BNRY, curr - 1);
    }

    return 1;
}

void ne2000_set_mac(const uint8_t* mac) {
    for (int i = 0; i < 6; i++) mac_addr[i] = mac[i];
    ne2000_set_page(NE2000_CR_PAGE1);
    for (int i = 0; i < 6; i++) {
        ne2000_write_reg(NE2000_REG_PAGE1_MAC0 + i, mac[i]);
    }
    ne2000_set_page(NE2000_CR_PAGE0);
}

void ne2000_get_mac(uint8_t* mac) {
    for (int i = 0; i < 6; i++) mac[i] = mac_addr[i];
}
