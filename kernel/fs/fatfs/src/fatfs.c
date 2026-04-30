#include "ff.h"
#include <stdio.h>
#include <string.h>
#include "diskio.h"
#include "fatfs.h"

#define DEV_MMC 1
/* FatFs 对象 */
static FATFS fs;

/* 简单错误打印 */
static void print_fresult(FRESULT res) {
    printf("FRESULT = %d\n", res);
}

/* 核心测试函数 */
void fatfs_test(void) {
    FRESULT res;
    FIL file;
    UINT br;
    char buf[128];

    fs.pdrv = DEV_MMC;  /* 关联物理驱动号 */

    printf("=== FatFs test start ===\n");

    /* 1️⃣ 初始化底层磁盘（你的实现） */
    if (disk_initialize(DEV_MMC) != 0) {
        printf("disk init failed\n");
        return;
    }

    /* 2️⃣ 挂载文件系统（必须！） */
    res = f_mount(&fs, "0:", 1);   // "0:" = 默认盘，1 = 立即挂载
    if (res != FR_OK) {
        printf("f_mount failed: ");
        print_fresult(res);
        return;
    }

    printf("mount ok\n");

    /* 3️⃣ 打开文件 */
    res = f_open(&file, "test.txt", FA_READ);
    if (res != FR_OK) {
        printf("f_open failed: ");
        print_fresult(res);
        return;
    }

    printf("file opened\n");

    /* 4️⃣ 读取文件 */
    memset(buf, 0, sizeof(buf));

    res = f_read(&file, buf, sizeof(buf) - 1, &br);
    if (res != FR_OK) {
        printf("f_read failed: ");
        print_fresult(res);
        f_close(&file);
        return;
    }

    printf("read %u bytes\n", br);
    printf("content: %s\n", buf);

    /* 5️⃣ 关闭文件 */
    f_close(&file);

    printf("=== FatFs test done ===\n");
}
