/*
 *  ITRON Message Buffer Manager
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id$
 */

#include <itron.h>

#include <rtems/itron/msgbuffer.h>
#include <rtems/itron/task.h>

/*
 *  rcv_mbf - Receive Message from MessageBuffer
 */

ER rcv_mbf(
  VP   msg,
  INT *p_msgsz,
  ID   mbfid
)
{
  return trcv_mbf(msg, p_msgsz, mbfid, TMO_FEVR);
}
