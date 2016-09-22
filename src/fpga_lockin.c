/**
 * @brief Red Pitaya LOCKIN FPGA controller.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "fpga_lockin.h"

/** 
 * GENERAL DESCRIPTION:
 *
 * This module initializes and provides for other SW modules the access to the 
 * FPGA LOCKIN module.
 *
 * This module maps physical address of the LOCKIN core to the logical address,
 * which can be used in the GNU/Linux user-space. To achieve this LOCKIN_BASE_ADDR
 * from CPU memory space is translated automatically to logical address with the
 * function mmap().
 * Before this module is used external SW module must call fpga_lockin_init().
 * When this module is no longer needed fpga_lockin_exit() should be called.
 */

/** The FPGA register structure (defined in fpga_lockin.h) */
lockin_reg_t *g_lockin_reg     = NULL;

/** The memory file descriptor used to mmap() the FPGA space */
int g_lockin_fd = -1;


/*----------------------------------------------------------------------------*/
/**
 * @brief Internal function used to clean up memory.
 *
 * This function un-maps FPGA registers, closes memory file
 * descriptor and cleans all memory allocated by this module.
 *
 * @retval 0 Success
 * @retval -1 Failure, error is printed to standard error output.
 */
int __lockin_cleanup_mem(void)
{
    /* If registry structure is NULL we do not need to un-map and clean up */
    if(g_lockin_reg) {
        if(munmap(g_lockin_reg, LOCKIN_BASE_SIZE) < 0) {
            fprintf(stderr, "munmap() failed: %s\n", strerror(errno));
            return -1;
        }
        g_lockin_reg = NULL;
    }

    if(g_lockin_fd >= 0) {
        close(g_lockin_fd);
        g_lockin_fd = -1;
    }
    return 0;
}

/** Reset all LOCKINs */
void reset_lockins(void)
{
    if (g_lockin_reg) {

        int i;
        for (i = 0; i < NUM_OF_LOCKINS; i++) {
            g_lockin_reg->lockin.setpoint = 0;
            g_lockin_reg->lockin.k1p = 0;
            g_lockin_reg->lockin.k1i = 0;
            g_lockin_reg->lockin.k1d = 0;
            g_lockin_reg->lockin.k2p = 0;
            g_lockin_reg->lockin.k2i = 0;
            g_lockin_reg->lockin.k2d = 0;
        }

        g_lockin_reg->configuration = 0;
    }
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Maps FPGA memory space and prepares register variables.
 * 
 * This function opens memory device (/dev/mem) and maps physical memory address
 * LOCKIN_BASE_ADDR (of length LOCKIN_BASE_SIZE) to logical addresses. It initializes
 * the pointer g_lockin_reg to point to FPGA LOCKIN.
 * If function fails FPGA variables must not be used.
 *
 * @retval 0  Success
 * @retval -1 Failure, error is printed to standard error output.
 */
int fpga_lockin_init(void)
{
    /* Page variables used to calculate correct mapping addresses */
    void *page_ptr;
    long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

    /* If module was already initialized, clean all internals */
    if(__lockin_cleanup_mem() < 0)
        return -1;

    /* Open /dev/mem to access directly system memory */
    g_lockin_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(g_lockin_fd < 0) {
        fprintf(stderr, "open(/dev/mem) failed: %s\n", strerror(errno));
        return -1;
    }

    /* Calculate correct page address and offset from LOCKIN_BASE_ADDR and
     * LOCKIN_BASE_SIZE
     */
    page_addr = LOCKIN_BASE_ADDR & (~(page_size-1));
    page_off  = LOCKIN_BASE_ADDR - page_addr;

    /* Map FPGA memory space to page_ptr. */
    page_ptr = mmap(NULL, LOCKIN_BASE_SIZE, PROT_READ | PROT_WRITE,
                          MAP_SHARED, g_lockin_fd, page_addr);
    if((void *)page_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap() failed: %s\n", strerror(errno));
         __lockin_cleanup_mem();
        return -1;
    }

    /* Set FPGA LOCKIN module pointers to correct values. */
    g_lockin_reg = page_ptr + page_off;

    /* Reset all controllers */
    reset_lockins();

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Cleans up FPGA LOCKIN module internals.
 * 
 * This function closes the memory file descriptor, unmaps the FPGA memory space
 * and cleans also all other internal things from FPGA LOCKIN module.
 * @retval 0 Success
 * @retval -1 Failure
 */
int fpga_lockin_exit(void)
{
    /* Reset all controllers */
    reset_lockins();

    return __lockin_cleanup_mem();
}
