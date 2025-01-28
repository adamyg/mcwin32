/*test application*/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

static char *
argument0(char *arg0)
{
    const wchar_t *wcmdline = GetCommandLineW();
    char *cmd = arg0;

    if (wcmdline[0]) { // import and derive argv0
        const unsigned wcmdsz = wcslen(wcmdline),
            cmdsz = WideCharToMultiByte(CP_UTF8, 0, wcmdline, (int)wcmdsz, NULL, 0, NULL, NULL);
        char *cursor;

        if (NULL != (cursor = malloc(cmdsz))) {

            cmd = cursor;
            WideCharToMultiByte(CP_UTF8, 0, wcmdline, (int)wcmdsz, cmd, cmdsz, NULL, NULL);

            if (cmd[0] == '"') {
                if (NULL != (cursor = strchr(cmd + 1, '"'))) { // quoted
                    *cursor = 0;
                    ++cmd;
                }
            } else {
                if (NULL != (cursor = strchr(cmd, ' '))) { // unquoted
                    *cursor = 0;
                }
            }
        }
    }
    return cmd;
}


void
main(int argc, char **argv)
{
    int i;

    printf("cmdline:  >%s<\n", GetCommandLineA());
    printf("arg0:     >%s<\n", argument0(argv[0]));
    printf("argcount: %d\n", argc);
    for (i = 0; i < argc; ++i)
    {
        printf("%d]: %s\n", i, argv[i]);
    }
}

//end


