/** \file quick.h
 *  \brief Header: quick dialog engine
 */

#ifndef MC__QUICK_H
#define MC__QUICK_H

#include "lib/tty/mouse.h"

/*** typedefs(not structures) and defined constants **********************************************/

#define QUICK_CHECKBOX(txt, st, id_)                                            \
{                                                                               \
    .widget_type = quick_checkbox,                                              \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = id_,                                                                  \
    .u = {                                                                      \
        .checkbox = {                                                           \
            .text = txt,                                                        \
            .state = st                                                         \
        }                                                                       \
    }                                                                           \
}

#define QUICK_BUTTON(txt, act, cb, id_)                                         \
{                                                                               \
    .widget_type = quick_button,                                                \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = id_,                                                                  \
    .u = {                                                                      \
        .button = {                                                             \
            .text = txt,                                                        \
            .action = act,                                                      \
            .callback = cb                                                      \
        }                                                                       \
    }                                                                           \
}

#define QUICK_INPUT(txt, hname, res, id_, is_passwd_, strip_passwd_, completion_flags_) \
{                                                                               \
    .widget_type = quick_input,                                                 \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = id_,                                                                  \
    .u = {                                                                      \
        .input = {                                                              \
            .label_text = NULL,                                                 \
            .label_location = input_label_none,                                 \
            .label = NULL,                                                      \
            .text = txt,                                                        \
            .completion_flags = completion_flags_,                              \
            .is_passwd = is_passwd_,                                            \
            .strip_passwd = strip_passwd_,                                      \
            .histname = hname,                                                  \
            .result = res                                                       \
        }                                                                       \
    }                                                                           \
}

#define QUICK_LABELED_INPUT(label_, label_loc, txt, hname, res, id_, is_passwd_, strip_passwd_, completion_flags_) \
{                                                                               \
    .widget_type = quick_input,                                                 \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = id_,                                                                  \
    .u = {                                                                      \
        .input = {                                                              \
            .label_text = label_,                                               \
            .label_location = label_loc,                                        \
            .label = NULL,                                                      \
            .text = txt,                                                        \
            .completion_flags = completion_flags_,                              \
            .is_passwd = is_passwd_,                                            \
            .strip_passwd = strip_passwd_,                                      \
            .histname = hname,                                                  \
            .result = res                                                       \
        }                                                                       \
    }                                                                           \
}

#define QUICK_LABEL(txt, id_)                                                   \
{                                                                               \
    .widget_type = quick_label,                                                 \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = id_,                                                                  \
    .u = {                                                                      \
        .label = {                                                              \
            .text = txt,                                                        \
            .input = NULL                                                       \
        }                                                                       \
    }                                                                           \
}

#define QUICK_RADIO(cnt, items_, val, id_)                                      \
{                                                                               \
    .widget_type = quick_radio,                                                 \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = id_,                                                                  \
    .u = {                                                                      \
        .radio = {                                                              \
            .count = cnt,                                                       \
            .items = items_,                                                    \
            .value = val                                                        \
        }                                                                       \
    }                                                                           \
}

#define QUICK_START_GROUPBOX(t)                                                 \
{                                                                               \
    .widget_type = quick_start_groupbox,                                        \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = NULL,                                                                 \
    .u = {                                                                      \
        .groupbox = {                                                           \
            .title = t                                                          \
        }                                                                       \
    }                                                                           \
}

#define QUICK_STOP_GROUPBOX                                                     \
{                                                                               \
    .widget_type = quick_stop_groupbox,                                         \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = NULL,                                                                 \
    .u = {                                                                      \
        .input = {                                                              \
            .text = NULL,                                                       \
            .histname = NULL,                                                   \
            .result = NULL                                                      \
        }                                                                       \
    }                                                                           \
}

#define QUICK_SEPARATOR(line_)                                                  \
{                                                                               \
    .widget_type = quick_separator,                                             \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = NULL,                                                                 \
    .u = {                                                                      \
        .separator = {                                                          \
            .space = TRUE,                                                      \
            .line = line_                                                       \
        }                                                                       \
    }                                                                           \
}

#define QUICK_START_COLUMNS                                                     \
{                                                                               \
    .widget_type = quick_start_columns,                                         \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = NULL,                                                                 \
    .u = {                                                                      \
        .input = {                                                              \
            .text = NULL,                                                       \
            .histname = NULL,                                                   \
            .result = NULL                                                      \
        }                                                                       \
    }                                                                           \
}

#define QUICK_NEXT_COLUMN                                                       \
{                                                                               \
    .widget_type = quick_next_column,                                           \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = NULL,                                                                 \
    .u = {                                                                      \
        .input = {                                                              \
            .text = NULL,                                                       \
            .histname = NULL,                                                   \
            .result = NULL                                                      \
        }                                                                       \
    }                                                                           \
}

#define QUICK_STOP_COLUMNS                                                      \
{                                                                               \
    .widget_type = quick_stop_columns,                                          \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = NULL,                                                                 \
    .u = {                                                                      \
        .input = {                                                              \
            .text = NULL,                                                       \
            .histname = NULL,                                                   \
            .result = NULL                                                      \
        }                                                                       \
    }                                                                           \
}

#define QUICK_START_BUTTONS(space_, line_)                                      \
{                                                                               \
    .widget_type = quick_buttons,                                               \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = NULL,                                                                 \
    .u = {                                                                      \
        .separator = {                                                          \
            .space = space_,                                                    \
            .line = line_                                                       \
        }                                                                       \
    }                                                                           \
}

#define QUICK_BUTTONS_OK_CANCEL                                                 \
    QUICK_START_BUTTONS (TRUE, TRUE),                                           \
        QUICK_BUTTON (N_("&OK"), B_ENTER, NULL, NULL),                          \
        QUICK_BUTTON (N_("&Cancel"), B_CANCEL, NULL, NULL)

#define QUICK_END                                                               \
{                                                                               \
    .widget_type = quick_end,                                                   \
    .options = WOP_DEFAULT,                                                     \
    .pos_flags = WPOS_KEEP_DEFAULT,                                             \
    .id = NULL,                                                                 \
    .u = {                                                                      \
        .input = {                                                              \
            .text = NULL,                                                       \
            .histname = NULL,                                                   \
            .result = NULL                                                      \
        }                                                                       \
    }                                                                           \
}

/*** enums ***************************************************************************************/

/* Quick Widgets */
typedef enum
{
    quick_end = 0,
    quick_checkbox = 1,
    quick_button = 2,
    quick_input = 3,
    quick_label = 4,
    quick_radio = 5,
    quick_start_groupbox = 6,
    quick_stop_groupbox = 7,
    quick_separator = 8,
    quick_start_columns = 9,
    quick_next_column = 10,
    quick_stop_columns = 11,
    quick_buttons = 12
} quick_t;

typedef enum
{
    input_label_none = 0,
    input_label_above = 1,
    input_label_left = 2,
    input_label_right = 3,
    input_label_below = 4
} quick_input_label_location_t;

/*** structures declarations (and typedefs of structures)*****************************************/

/* The widget is placed on relative_?/divisions_? of the parent widget */
typedef struct quick_widget_t quick_widget_t;

struct quick_widget_t
{
    quick_t widget_type;

    widget_options_t options;
    widget_state_t state;
    widget_pos_flags_t pos_flags;
    unsigned long *id;

    /* widget parameters */
    union
    {
        struct
        {
            const char *text;
            gboolean *state;    /* in/out */
        } checkbox;

        struct
        {
            const char *text;
            int action;
            bcback_fn callback;
        } button;

        struct
        {
            const char *label_text;
            quick_input_label_location_t label_location;
            quick_widget_t *label;
            const char *text;
            input_complete_t completion_flags;
            gboolean is_passwd; /* TRUE -- is password */
            gboolean strip_passwd;
            const char *histname;
            char **result;
        } input;

        struct
        {
            const char *text;
            quick_widget_t *input;
        } label;

        struct
        {
            int count;
            const char **items;
            int *value;         /* in/out */
        } radio;

        struct
        {
            const char *title;
        } groupbox;

        struct
        {
            gboolean space;
            gboolean line;
        } separator;
    } u;
};

typedef struct
{
    WRect rect;                 /* if rect.x == -1 or rect.y == -1, then dialog is ceneterd;
                                 * rect.lines is unused and ignored */
    const char *title;
    const char *help;
    quick_widget_t *widgets;
    widget_cb_fn callback;
    widget_mouse_cb_fn mouse_callback;
} quick_dialog_t;

/*** WIN32 ****/

#if defined(__GNUC__)
#define XQUICK_INLINE static inline
#else
#define XQUICK_INLINE inline
#endif

XQUICK_INLINE quick_dialog_t 
QUICK_DIALOG_INIT(
    const WRect *rect, const char *title, const char *help, quick_widget_t *widgets, widget_cb_fn callback, widget_mouse_cb_fn mouse_callback)
{
    quick_dialog_t qd;
    memset(&qd, 0, sizeof(qd));
    qd.rect = *rect;
    qd.title = title;
    qd.help = help;
    qd.widgets = widgets;
    qd.callback = NULL;
    qd.mouse_callback = NULL;
    qd.callback = callback;
    qd.mouse_callback = mouse_callback;
    return qd;
}

#if defined(WIN32) //WIN32, quick

XQUICK_INLINE quick_widget_t *
XQUICK_CHECKBOX(quick_widget_t *qc,
    const char *txt, int *st, unsigned long *id_)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_checkbox;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = id_;
    tqc.u.checkbox.text = txt;
    tqc.u.checkbox.state = st;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_BUTTON(quick_widget_t *qc,
    const char *txt, int act, bcback_fn cb, unsigned long *id_)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_button;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = id_;
    tqc.u.button.text = txt;
    tqc.u.button.action = act;
    tqc.u.button.callback = cb;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_INPUT(quick_widget_t *qc,
    const char *txt, const char *hname, char **res, unsigned long *id_,
        int is_passwd_, int strip_passwd_, int completion_flags_)
{
    quick_widget_t tqc = {0};

    tqc.widget_type = quick_input;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = id_;
    tqc.u.input.label_text = NULL;
    tqc.u.input.label_location = input_label_none;
    tqc.u.input.label = NULL;
    tqc.u.input.text = txt;
    tqc.u.input.completion_flags = completion_flags_;
    tqc.u.input.is_passwd = is_passwd_;
    tqc.u.input.strip_passwd = strip_passwd_;
    tqc.u.input.histname = hname;
    tqc.u.input.result = res;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_LABELED_INPUT(quick_widget_t *qc,
    const char *label_, quick_input_label_location_t label_loc,
    const char *txt, const char *hname, char **res, unsigned long *id_,
    int is_passwd_, int strip_passwd_, int completion_flags_)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_input;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = id_;
    tqc.u.input.label_text = label_;
    tqc.u.input.label_location = label_loc;
    tqc.u.input.label = NULL;
    tqc.u.input.text = txt;
    tqc.u.input.completion_flags = completion_flags_;
    tqc.u.input.is_passwd = is_passwd_;
    tqc.u.input.strip_passwd = strip_passwd_;
    tqc.u.input.histname = hname;
    tqc.u.input.result = res;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_LABEL(quick_widget_t *qc,
    const char *txt, unsigned long *id_)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_label;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = id_;
    tqc.u.label.text = txt;
    tqc.u.label.input = NULL;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_RADIO(quick_widget_t *qc,
    int cnt, const char **items_, int *val, unsigned long *id_)
{
    quick_widget_t tqc = {0};

    tqc.widget_type = quick_radio;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = id_;
    tqc.u.radio.count = cnt;
    tqc.u.radio.items = items_;
    tqc.u.radio.value = val;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_START_GROUPBOX(quick_widget_t *qc, const char *t)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_start_groupbox;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = NULL;
    tqc.u.groupbox.title = t;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_STOP_GROUPBOX(quick_widget_t *qc)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_stop_groupbox;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = NULL;
    tqc.u.input.text = NULL;
    tqc.u.input.completion_flags = 0;
    tqc.u.input.is_passwd = 0;
    tqc.u.input.strip_passwd = 0;
    tqc.u.input.histname = NULL;
    tqc.u.input.result = NULL;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_SEPARATOR(quick_widget_t *qc, gboolean line_)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_separator;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = NULL;
    tqc.u.separator.space = TRUE;
    tqc.u.separator.line = line_;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_START_COLUMNS(quick_widget_t *qc)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_start_columns;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = NULL;
    tqc.u.input.text = NULL;
    tqc.u.input.completion_flags = 0;
    tqc.u.input.is_passwd = 0;
    tqc.u.input.strip_passwd = 0;
    tqc.u.input.histname = NULL;
    tqc.u.input.result = NULL;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_NEXT_COLUMN(quick_widget_t *qc)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_next_column;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = NULL;
    tqc.u.input.text = NULL;
    tqc.u.input.completion_flags = 0;
    tqc.u.input.is_passwd = 0;
    tqc.u.input.strip_passwd = 0;
    tqc.u.input.histname = NULL;
    tqc.u.input.result = NULL;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_STOP_COLUMNS(quick_widget_t *qc)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_stop_columns;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = NULL;
    tqc.u.input.text = NULL;
    tqc.u.input.completion_flags = 0;
    tqc.u.input.is_passwd = 0;
    tqc.u.input.strip_passwd = 0;
    tqc.u.input.histname = NULL;
    tqc.u.input.result = NULL;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_START_BUTTONS(quick_widget_t *qc, gboolean space_, gboolean line_)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_buttons;
    tqc.options = 0;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    tqc.id = NULL;
    tqc.u.separator.space = space_;
    tqc.u.separator.line = line_;
    *qc = tqc;
    return ++qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_BUTTONS_OK_CANCEL(quick_widget_t *qc)
{
    qc = XQUICK_START_BUTTONS (qc, TRUE, TRUE);
    qc = XQUICK_BUTTON (qc, N_("&OK"), B_ENTER, NULL, NULL);
    qc = XQUICK_BUTTON (qc, N_("&Cancel"), B_CANCEL, NULL, NULL);
    return qc;
}

XQUICK_INLINE quick_widget_t *
XQUICK_END(quick_widget_t *qc)
{
    quick_widget_t tqc = {0};
    tqc.widget_type = quick_end;
    tqc.pos_flags = WPOS_KEEP_DEFAULT;
    *qc = tqc;
    return ++qc;
}
#endif  //WIN32

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

int quick_dialog_skip (quick_dialog_t * quick_dlg, int nskip);

/*** inline functions ****************************************************************************/

static inline int
quick_dialog (quick_dialog_t * quick_dlg)
{
    return quick_dialog_skip (quick_dlg, 1);
}

#endif /* MC__QUICK_H */
