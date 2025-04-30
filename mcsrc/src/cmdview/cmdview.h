#ifndef MC__CMDVIEW_H
#define MC__CMDVIEW_H

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct WCmd
{
    Widget widget;
    WButtonBar *bar;
    gboolean view_quit;
} WCmd;

/*** global variables defined in .c file *********************************************************/

extern const global_keymap_t *cmdview_map;

/*** declarations of public functions ************************************************************/

gboolean cmdview_cmd (void);

#endif /* MC__CMDVIEW_H */
