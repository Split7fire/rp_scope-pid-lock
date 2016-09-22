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

#ifndef __LOCKIN_H
#define __LOCKIN_H

#include "main.h"

int lockin_init(void);
int lockin_exit(void);

int lockin_update(rp_app_params_t *params);

#endif // __LOCKIN_H
