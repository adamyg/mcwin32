/* -*- mode: c; indent-width: 4; -*- */
/*
   win32 Change-drive command

        +------- Change Drive -------+
        |
        |   [C] [D] [E] [F] ....     |
        |
        +----------------------------+

   Adam Young 2015 - 2025

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <config.h>
#include "libw32.h"

#include <sys/types.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <glib.h>

#include "lib/tty/tty.h"
#include "lib/skin.h"
#include "lib/vfs/vfs.h"
#include "lib/strutil.h"
#include "lib/util.h"
#include "lib/widget.h"
#include "lib/keybind.h"                        /* CK_Cancel etc */
#include "src/filemanager/panel.h"
#include "src/filemanager/cmd.h"                /* reread_cmd() */
#include "src/filemanager/filemanager.h"        /* left/right panel */

#include "drive.h"

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Advapi32.lib")
#include <lm.h>

static void             drive_sel (WPanel *panel);
static cb_ret_t         drive_dlg_callback (Widget * h, Widget * sender, widget_msg_t msg, int parm, void *data);

#undef DO_NETWORK_DRIVES
#if defined(DO_NETWORK_DRIVES)

typedef DWORD (__stdcall *WNetOpenEnumA_t)(DWORD, DWORD, DWORD, NETRESOURCE *, HANDLE*);
typedef DWORD (__stdcall *WNetEnumResourceA_t)(HANDLE, DWORD *, void *, DWORD *);
typedef DWORD (__stdcall *WNetCloseEnum_t)(HANDLE);

struct WNetFunctions {
    int state;
    HANDLE mpr;
    WNetOpenEnumA_t fnWNetOpenEnumA;
    WNetEnumResourceA_t fnWNetEnumResourceA;
    WNetCloseEnum_t fnWNetCloseEnum;
};

struct NetworkDrive {
    const char *local;
    const char *remote;
        // [loca/remote storage ]
};

static int              drive_network_names (int global, GQueue *result);
static int              enumerate_disks (int global, struct WNetFunctions *fns, LPNETRESOURCE lpnr, GQueue *result);

#endif  //DO_NETWORK_DRIVES

#if (XXX)
static int
IsRemovable(char drive)
{
    int RemovableMedia = 0;
    char path[] = "\\\\?\\X:";
    HANDLE handle;

    assert(drive >= 'A' && drive <= 'Z');
    
    path[4] = drive;
    handle = CreateFileA(path, 0 /*no-access*/, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (INVALID_HANDLE_VALUE != handle) {
        STORAGE_PROPERTY_QUERY spq = { StorageDeviceProperty, PropertyStandardQuery };
        STORAGE_DEVICE_DESCRIPTOR sdd = { 0 };
        ULONG rcb;

        if (DeviceIoControl(handle, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq), &sdd, sizeof(sdd), &rcb, 0)) {
            RemovableMedia = sdd.RemovableMedia;
                // Indicates when TRUE that the device's media (if any) is removable.
        }
        CloseHandle(handle);
    }
    return RemovableMedia;
}
#endif


void
drive_cmd(void)
{
    WPanel *panel;

    if (NULL != (panel = current_panel)) {
        drive_sel(panel);
    }
}


void
drive_cmd_a(void)
{
    WPanel *panel;

    if (NULL != (panel = left_panel)) {
        drive_sel(panel);
    }
}


void
drive_cmd_b(void)
{
    WPanel *panel;

    if (NULL != (panel = right_panel)) {
        drive_sel(panel);
    }
}


static void
drive_sel(WPanel *panel)
{
    char t_drives[27 * 8] = {0}, *cursor,       /* temporary drives and cursor */
            drives[27 * 8] = {0};               /* resulting */
    int x_pos, y_pos, y_height, x_width;
    char root[4] = "?:\\";
    WDialog *drive_dlg;
    int totaldrives = 0, groupidx = 0, d;
    char currentdrive = 0;
    const char *path;
    WGroup *g;
    GQueue *buttons;

    /* Active local drives */
    if (NULL != (path = vfs_get_current_dir ())) {
        if (path[0] && ':' == path[1]) {        /* X:.... */
            currentdrive = toupper (*path);
        }
    }

    GetLogicalDriveStringsA(sizeof(t_drives) - 1, t_drives);
    for (path = t_drives, cursor = drives; *path;) {
        const int length = strlen(path);
        const char drive = toupper(*path);
        int type;

        assert(length < 8);
        assert(drive >= 'A' && drive <= 'Z');

        root[0] = drive;
        if ((type = GetDriveTypeA(root)) != DRIVE_UNKNOWN && type != DRIVE_NO_ROOT_DIR) {
            if (DRIVE_REMOVABLE == type ||      /* assume floppies; FIXME */
                    GetDiskFreeSpaceExA(root, NULL, NULL, NULL)) {
                (void) memcpy(cursor, path, length);
                if (drive == currentdrive) {
                    groupidx = totaldrives;     /* initial selection index */
                }
                cursor[0] = drive;
                cursor += length + 1;
                ++totaldrives;
            }
        }
        path += length + 1 /*nul*/;
    }
    groupidx = totaldrives - groupidx;          /* flip */

    /* Network resources */
#if defined(DO_NETWORK_DRIVES)
    {                                           /* test only */
        GQueue *network_drives = g_queue_new();
        int elements = drive_network_names (TRUE, network_drives); // XXX: cache results           
        g_queue_free_full (network_drives, free);
    }
#endif  //DO_NETWORK_DRIVES

    /* Create Dialog */
#define D_PERLINE       12                      /* dynamic, based on screen width?? */
#define D_BUTSTART      3
#define D_BUTWIDTH      4

    if (totaldrives > D_PERLINE) {
        x_width  = 5 + (D_BUTWIDTH * D_PERLINE);
        y_height = 7 + ((totaldrives / D_PERLINE) * 2);
    } else {
        x_width  = 5 + (D_BUTWIDTH * totaldrives);
        if (x_width < 20) x_width = 20;         /* min width, inclusive of title/border */
        y_height = 7;
    }

    y_pos = ((LINES - 6) / 2) - 3;
    x_pos = panel->widget.rect.x +              /* center relative to panel */
                ((panel->widget.rect.cols -
                    ((totaldrives > D_PERLINE ? D_PERLINE : totaldrives) * D_BUTWIDTH)) / 2) + 2;

    do_refresh ();
    drive_dlg =
        dlg_create(TRUE, y_pos, x_pos, y_height, x_width, WPOS_CENTER | WPOS_TRYUP, FALSE,
                dialog_colors, drive_dlg_callback, NULL /*TODO-MOUSE*/, "[Chdrive]", _("Change Drive"));
    g = GROUP (drive_dlg);

    /*
     *  Drive buttons
     *      build and then push in reverse so button navigation is correct
     */
    buttons = g_queue_new();

    if (totaldrives > D_PERLINE) {
        int y = 1, x = D_BUTSTART;
        char buffer[7];
                                                /* multiple lines */
        for (path = drives, d = 0; d < totaldrives; ++d) {
            sprintf(buffer, "&%c", *path);
            if (0 == (d % D_PERLINE)) {
                y += 2, x = D_BUTSTART;         /* new line */
            }
            g_queue_push_tail(buttons, button_new(y, x, B_USER + d, NARROW_BUTTON, buffer, 0));
            path += strlen(path) + 1;
            x += D_BUTWIDTH;
        }

    } else {
        int x = ((x_width - (totaldrives * D_BUTWIDTH)) / 2) + 1;
        char buffer[7];
                                                /* single line */
        for (path = drives, d = 0; d < totaldrives; ++d) {
            sprintf(buffer, "&%c", *path);
            g_queue_push_tail(buttons, button_new(3, x, B_USER + d, NARROW_BUTTON, buffer, 0));
            path += strlen(path) + 1;
            x += D_BUTWIDTH;
        }
    }

    while (! g_queue_is_empty (buttons)) {      /* drive widgets */
        group_add_widget (g, g_queue_pop_tail(buttons));
    }

    while (groupidx-- > 0) {                    /* select current working drive */
        group_select_next_widget (g);
    }

    g_queue_free (buttons);

    /* do action */
    dlg_run (drive_dlg);

    if (drive_dlg->ret_value != B_CANCEL) {
        const int is_right = (panel == right_panel ? 1 : 0);
        const int drive_idx = (drive_dlg->ret_value - B_USER);

        for (path = drives, d = 0; d < totaldrives; ++d) {
            if (d == drive_idx) {               /* drive selection */
                const char *curr_dir = vfs_get_current_dir ();
                const char drive = *path;

                if (!*curr_dir || toupper(*curr_dir) != drive) {
                    char t_path[MAX_PATH], t_cwd[MAX_PATH];

                    if (! w32_getcwdd(drive, t_path, sizeof(t_path))) {
                        t_path[0] = drive;
                        t_path[1] = ':';
                        t_path[2] = 0;
                    }

                    if (0 == w32_chdir (t_path)) {
                        vfs_path_t *cwd_vdir;

                        w32_getcwd (t_cwd, sizeof(t_cwd));
                        cwd_vdir = vfs_path_from_str (t_cwd);
                        if (get_panel_type (is_right) != view_listing) {
                            create_panel (is_right, view_listing);
                        }
                        panel_do_cd (panel, cwd_vdir, cd_exact);
                        vfs_path_free (cwd_vdir, TRUE);

                    } else {
                        message (D_ERROR, MSG_ERROR, _("Cannot change drive to \"%s\"\n%s"), t_path,
                                    unix_error_string (errno));
                    }
                }
                break;  //done
            }
            path += strlen(path) + 1;
        }
    }

    widget_destroy (WIDGET (drive_dlg));
    repaint_screen ();
}


static cb_ret_t
drive_dlg_callback (Widget * h, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    WDialog *d = DIALOG (h);

    switch (msg) {
    case MSG_DRAW:
        group_default_callback (h, NULL, MSG_DRAW, 0, NULL); /* frame + buttons */
        return MSG_HANDLED;

    case MSG_KEY:
        switch (parm) {
        case KEY_LEFT:
        case KEY_UP:
            group_select_next_widget (GROUP (h));
            return MSG_HANDLED;
        case KEY_RIGHT:
        case KEY_DOWN:
            group_select_prev_widget (GROUP (h));
            return MSG_HANDLED;
        }
        /*FALLTHRU*/

    default:
        return group_default_callback(h, sender, msg, parm, data);
    }
    /*NOTREACHED*/
    return 0;
}


#if defined(DO_NETWORK_DRIVES)
static int
drive_network_names(int global, GQueue *result)
{
    static struct WNetFunctions fns = {0};

    if (0 == fns.state) {                       // initial request
        fns.state = -1;
        if (NULL != (fns.mpr = LoadLibraryA ("MPR.DLL"))) {
            fns.fnWNetOpenEnumA = (WNetOpenEnumA_t) GetProcAddress (fns.mpr, "WNetOpenEnumA");
            fns.fnWNetEnumResourceA = (WNetEnumResourceA_t) GetProcAddress (fns.mpr, "WNetEnumResourceA");
            fns.fnWNetCloseEnum = (WNetCloseEnum_t) GetProcAddress (fns.mpr, "WNetCloseEnum");
            if (fns.fnWNetOpenEnumA && fns.fnWNetEnumResourceA && fns.fnWNetCloseEnum) {
                fns.state = 1;                  // done
            } else {
                FreeLibrary (fns.mpr);
                fns.mpr = 0;
            }
        }
    }

    if (1 == fns.state) {                       // success
        return enumerate_disks(global, &fns, NULL, result);
    }
    return -1;
}
#endif  //DO_NETWORK_DRIVES


#if defined(DO_NETWORK_DRIVES)
static int
enumerate_disks(int global, struct WNetFunctions *fns, LPNETRESOURCE lpnr, GQueue *result)
{
    const unsigned scope = (global ? RESOURCE_GLOBALNET : RESOURCE_CONNECTED);
    int elements = 0;
    HANDLE hEnum = 0;
    DWORD dwResult;

    if (NO_ERROR == (dwResult = fns->fnWNetOpenEnumA(scope, RESOURCETYPE_DISK, 0, lpnr, &hEnum))) {
        DWORD dwSize = 256, dwOrgSize = dwSize;
        NETRESOURCE *pnr = (NETRESOURCE *)calloc(dwSize, 1);
        DWORD cEntries = (DWORD)-1, cIndex;

        while (pnr) {
            if (NO_ERROR == (dwResult = fns->fnWNetEnumResourceA(hEnum, &cEntries, pnr, &dwSize))) {
                //
                //  Iterate result
                for (cIndex = 0; cIndex < cEntries; ++cIndex) {
                    NETRESOURCE *nr = pnr + cIndex;

                    if (RESOURCEUSAGE_CONTAINER == (nr->dwUsage & RESOURCEUSAGE_CONTAINER)) {
                        // If the NETRESOURCE structure represents a container resource,
                        // call the EnumerateFunc function recursively.
                        int subelements;

                        if ((subelements = enumerate_disks(global, fns, nr, result)) < 0) {
                            break;              // error
                        }
                        elements += subelements;

                    } else {
                        const size_t localsz = strlen(nr->lpLocalName) + 1,
                            remotesz = strlen(nr->lpRemoteName) + 1;
                        struct NetworkDrive *pnd = NULL;

                        assert(RESOURCETYPE_DISK == nr->dwType);
                        if (NULL != (pnd = (struct NetworkDrive *)
                                calloc(sizeof(struct NetworkDrive) + localsz + remotesz, 1))) {
                            char *cursor = (char *)(pnd + 1);

                            pnd->local = cursor, memcpy(cursor, nr->lpLocalName, localsz), cursor += localsz;
                            pnd->remote = cursor, memcpy(cursor, nr->lpRemoteName, remotesz);
                            g_queue_push_tail(result, pnd);
                            ++elements;
                        }
                    }
                }
                continue;   //next

            } else if (ERROR_MORE_DATA == dwResult) {
                //
                //  Extend storage
                NETRESOURCE *org_pnr = pnr;

                assert(dwSize > dwOrgSize);
                if (NULL == (pnr = (NETRESOURCE*) realloc(pnr, dwSize))) {
                    free((void *)org_pnr);      // error, release previous
                } else {
                    memset(pnr + dwOrgSize, 0, dwSize - dwOrgSize);
                    dwOrgSize = dwSize;
                }
                continue;   //next

            } else if (ERROR_NO_MORE_ITEMS != dwResult) {
                //
                //  Completion or error
                elements = -1;
            }
            break;  //done
        }

        if (NO_ERROR != fns->fnWNetCloseEnum(hEnum) || NULL == pnr) {
            elements = -1;                      // WNetCloseEnum failure/or memory.
        }
        free((void *)pnr);
    }

    return elements;
}
#endif  //DO_NETWORK_DRIVES

/*end*/
