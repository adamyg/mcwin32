#ifndef MC__CMDVIEW_H
#define MC__CMDVIEW_H

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct WCmdView
{
    Widget widget;
    WButtonBar *bar;
    gboolean view_quit;

    /*previous state*/
    int original_output_lines;
    gboolean original_command_prompt;
    WGroup *ogroups[2];
} WCmdView;

#define CMDVIEW(x) ((WCmdView *)(x))

/*** global variables defined in .c file *********************************************************/

extern const global_keymap_t *cmdview_map;

/*** declarations of public functions ************************************************************/

gboolean cmdview_cmd (void);

#endif /* MC__CMDVIEW_H */
