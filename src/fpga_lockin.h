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

#ifndef _FPGA_LOCKIN_H_
#define _FPGA_LOCKIN_H_

#include <stdint.h>

/** @defgroup fpga_lockin_h LOCKIN Controller
 * @{
 */

/** Base LOCKIN FPGA address */
#define LOCKIN_BASE_ADDR 0x40600000
/** Base LOCKIN FPGA core size */
#define LOCKIN_BASE_SIZE 0x100

/** Number of LOCKIN controllers */
#define NUM_OF_LOCKINS 4


/** LOCKIN Controller parameters */
typedef struct {
    /** @brief lock-in phase
     */
    uint32_t phase;
    /** @brief lock-in phase
     */
    uint32_t gain;
    /** @brief Relative offset 0x0 - Set-point
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Set-point (signed)
     */
    uint32_t setpoint;
    /** @brief Relative offset 0x4 - Proportional gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Proportional gain (signed)
     */
    uint32_t k1p;
    /** @brief Relative offset 0x8 - Integral gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Integral gain (signed)
     */
    uint32_t k1i;
    /** @brief Relative offset 0xC - Derivative gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Derivative gain (signed)
     */
    uint32_t k1d;
    /** @brief Relative offset 0x4 - Proportional gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Proportional gain (signed)
     */
    uint32_t k2p;
    /** @brief Relative offset 0x8 - Integral gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Integral gain (signed)
     */
    uint32_t k2i;
    /** @brief Relative offset 0xC - Derivative gain
     *
     *  bits [31:14] - Reserved
     *  bit  [13: 0] - Derivative gain (signed)
     */
    uint32_t k2d;
} lockin_param_t;


/** @brief LOCKIN FPGA registry structure.
 *
 * This structure is direct image of physical FPGA memory. When accessing it all
 * reads/writes are performed directly from/to FPGA LOCKIN registers.
 */
typedef struct lockin_reg_t {
    /** @brief Offset 0x00 - Configuration
     *
     *  bits [31:2]  - Reserved
     *  bit  [   1] -  LOCKIN PID1 ena
     *  bit  [   0] -  LOCKIN PID2 ena
     */
    uint32_t configuration;
    /** @brief Offset 0x04 - Reserved */
    uint32_t res0;
    /** @brief Offset 0x08 - Reserved */
    uint32_t res1;
    /** @brief Offset 0x0c - Reserved */
    uint32_t res2;


    lockin_param_t lockin;

} lockin_reg_t;

/** @} */

/* Description of the following variables or function declarations is in 
 * fpga_lockin.c
 */
extern lockin_reg_t    *g_lockin_reg;

int fpga_lockin_init(void);
int fpga_lockin_exit(void);

#endif // _FPGA_LOCKIN_H_
