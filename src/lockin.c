/**
 * @brief Red Pitaya LOCKIN Controller
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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lockin.h"
#include "fpga_lockin.h"

/**
 * GENERAL DESCRIPTION:
 *
 * The code below sets the LOCKIN parameters to the user-specified values.
 * A direct translation from GUI LOCKIN parameters to the FPGA registers.
 *
 * The LOCKIN Controller algorithm itself is implemented in FPGA.
 * There are 4 independent LOCKIN controllers, connecting each input (IN1, IN2)
 * to each output (OUT1, OUT2):
 *
 *                 /-------\       /-----------\
 *   IN1 -----+--> | LOCKIN11 | ------| SUM & SAT | ---> OUT1
 *            |    \-------/       \-----------/
 *            |                            ^
 *            |    /-------\               |
 *            ---> | LOCKIN21 | ----------    |
 *                 \-------/           |   |
 *                                     |   |
 *                                     |   |
 *                                     |   |
 *                 /-------\           |   |
 *            ---> | LOCKIN12 | --------------
 *            |    \-------/           |
 *            |                        ˇ
 *            |    /-------\       /-----------\
 *   IN2 -----+--> | LOCKIN22 | ------| SUM & SAT | ---> OUT2
 *                 \-------/       \-----------/
 *
 */


/*----------------------------------------------------------------------------------*/
/** @brief Initialize LOCKIN Controller module
 *
 * A function is intended to be called within application initialization. It's purpose
 * is to initialize LOCKIN Controller module and to calculate maximal voltage, which can be
 * applied on DAC device on individual channel.
 *
 * @retval     -1 failure, error message is reported on standard error
 * @retval      0 successful initialization
 */

int lockin_init(void)
{
    if(fpga_lockin_init() < 0) {
        return -1;
    }

    return 0;
}


/*----------------------------------------------------------------------------------*/
/** @brief Cleanup LOCKIN COntroller module
 *
 * A function is intended to be called on application's termination. The main purpose
 * of this function is to release allocated resources...
 *
 * @retval      0 success, never fails.
 */
int lockin_exit(void)
{
    fpga_lockin_exit();

    return 0;
}


/*----------------------------------------------------------------------------------*/
/**
 * @brief Update LOCKIN Controller module towards actual settings.
 *
 * A function is intended to be called whenever one of the following settings on each LOCKIN
 * sub-controller is modified:
 *    - Enable
 *    - Integrator reset
 *    - Set-point
 *    - Kp
 *    - Ki
 *    - Kd
 *
 * @param[in] params  Pointer to overall configuration parameters
 * @retval -1 failure, error message is repoted on standard error device
 * @retval  0 succesful update
 */
int lockin_update(rp_app_params_t *params)
{

    lockin_param_t lockin = { 0 };
    uint32_t lockin_config = 0;

        /* LOCKIN enabled? */
        if (params[LOCKIN_PID_1_ENABLE].value == 1)
        {
        	lockin_config |= 1 << 1;
            lockin.k1p = (int)params[LOCKIN_PID_1_KP].value;
            lockin.k1i = (int)params[LOCKIN_PID_1_KI].value;
            lockin.k1d = (int)params[LOCKIN_PID_1_KD].value;
        }
        else
        {
            lockin.k1p = 0;
            lockin.k1i = 0;
            lockin.k1d = 0;
        }

        if (params[LOCKIN_PID_2_ENABLE].value == 1)
        {
        	lockin_config |= 1 << 2;
            lockin.k2p = (int)params[LOCKIN_PID_2_KP].value;
            lockin.k2i = (int)params[LOCKIN_PID_2_KI].value;
            lockin.k2d = (int)params[LOCKIN_PID_2_KD].value;
        }
        else
        {
            lockin.k2p = 0;
            lockin.k2i = 0;
            lockin.k2d = 0;
        }

        g_lockin_reg->lockin.phase 	  = (int)params[LOCKIN_PHASE].value;
        g_lockin_reg->lockin.gain 	  = (int)params[LOCKIN_GAIN].value;
        g_lockin_reg->lockin.setpoint = (int)params[LOCKIN_PID_1_SP].value;
        g_lockin_reg->lockin.k1p = lockin.k1p;
        g_lockin_reg->lockin.k1i = lockin.k1i;
        g_lockin_reg->lockin.k1d = lockin.k1d;
        g_lockin_reg->lockin.k2p = lockin.k2p;
        g_lockin_reg->lockin.k2i = lockin.k2i;
        g_lockin_reg->lockin.k2d = lockin.k2d;


    g_lockin_reg->configuration = lockin_config;

    return 0;
}

