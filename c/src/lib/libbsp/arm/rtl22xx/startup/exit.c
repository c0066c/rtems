/*
 * Philips LPC22XX/LPC21xx BSP Shutdown  code
 * Copyright (c) 2007 by Ray Xu <rayx.cn@gmail.com>
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *
 *  http://www.rtems.com/license/LICENSE.
 *
 *
 *  $Id$
*/

#include <stdio.h>
#include <bsp.h>
#include <rtems/bspIo.h>
#include <rtems/libio.h>

int uart_poll_read(int);

void rtemsReboot (void)
{
  asm volatile ("b _start");
}

void bsp_cleanup(void)
{
  static   char line[]="\nEXECUTIVE SHUTDOWN! Any key to reboot...";
  /*
   * AT this point, the console driver is disconnected => we must
   * use polled output/input. This is exactly what printk
   * does.
   */
  debug_printk("\n");
  debug_printk(line);
  while (uart_poll_read(0) < 0) continue;

  /* rtemsReboot(); */
}
