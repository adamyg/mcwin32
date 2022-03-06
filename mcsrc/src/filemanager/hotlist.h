/** \file hotlist.h
 *  \brief Header: directory hotlist
 */

#ifndef MC__HOTLIST_H
#define MC__HOTLIST_H

/*** typedefs(not structures) and defined constants **********************************************/

#include "panel.h"

/*** enums ***************************************************************************************/

typedef enum
{
    LIST_VFSLIST = 0x01,
    LIST_HOTLIST = 0x02,
    LIST_MOVELIST = 0x04
} hotlist_t;

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void add2hotlist_cmd (WPanel * panel);
char *hotlist_show (hotlist_t list_type, WPanel * panel);
gboolean save_hotlist (void);
void done_hotlist (void);

/*** inline functions ****************************************************************************/
#endif /* MC__HOTLIST_H */
