#include "ata.h"
#include "io.h"

static void ata_wait_busy() {
    while (inb(0x1F7) & 0x80); /* BSY */
}

static int ata_wait_drq() {
    uint8_t status;
    while (1) {
        status = inb(0x1F7);
        if (!(status & 0x80) && (status & 0x08))
            return 0; /* OK */
        if (status & 0x01)
            return -1; /* ERR */
    }
}

int ata_read_sector(uint32_t lba, uint8_t *buf) {
    ata_wait_busy();

    outb(0x1F2, 1); /* 1 sector */
    outb(0x1F3, (uint8_t)(lba));
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); /* LBA mode */

    outb(0x1F7, 0x20); /* READ SECTORS */

    if (ata_wait_drq() < 0)
        return -1;

    /* 读取 512 bytes = 256 * 16bit */
    insw(0x1F0, buf, 256);

    return 0;
}
