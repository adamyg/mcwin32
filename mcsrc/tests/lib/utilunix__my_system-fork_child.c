/*
   lib - tests lib/utilinux:my_system() function

   Copyright (C) 2013-2025
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2013

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

#define TEST_SUITE_NAME "/lib/utilunix"

#include "tests/mctest.h"

#include "lib/util.h"

#include "utilunix__my_system-common.c"

/* --------------------------------------------------------------------------------------------- */

/* *INDENT-OFF* */
START_TEST (fork_child)
/* *INDENT-ON* */
{
    int actual_value;
    /* given */
    fork__return_value = 0;

    /* when */
    actual_value = my_system (0, "/bin/some-command", "some parameter");

    /* then */
    ck_assert_int_eq (actual_value, 0);

    VERIFY_SIGACTION_CALLS ();
    VERIFY_SIGNAL_CALLS ();

    mctest_assert_str_eq (execvp__file__captured, "/bin/some-command");
    ck_assert_int_eq (execvp__args__captured->len, 2);

    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 0), "/bin/some-command");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 1), "some parameter");

    /* All exec* calls is mocked, so call to _exit() function with 127 status code it's a normal situation */
    ck_assert_int_eq (my_exit__status__captured, 127);
}
/* *INDENT-OFF* */
END_TEST
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */

/* *INDENT-OFF* */
START_TEST (fork_child_tokens)
/* *INDENT-ON* */
{
    int actual_value;
    /* given */
    fork__return_value = 0;

    /* when */
    actual_value = my_system (0, "vi +", "foo.txt");

    /* then */
    ck_assert_int_eq (actual_value, 0);

    VERIFY_SIGACTION_CALLS ();
    VERIFY_SIGNAL_CALLS ();

    mctest_assert_str_eq (execvp__file__captured, "vi");
    ck_assert_int_eq (execvp__args__captured->len, 3);

    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 0), "vi");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 1), "+");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 2), "foo.txt");

    /* All exec* calls is mocked, so call to _exit() function with 127 status code it's a normal situation */
    ck_assert_int_eq (my_exit__status__captured, 127);
}
/* *INDENT-OFF* */
END_TEST
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */

/* *INDENT-OFF* */
START_TEST (fork_child_tokens2)
/* *INDENT-ON* */
{
    int actual_value;
    /* given */
    fork__return_value = 0;

    /* when */
    actual_value = my_system (0, "qwe -a 'aa bb' -b -c cc -d \"dd ee\" -f ff\\ gg", "foo.txt");

    /* then */
    ck_assert_int_eq (actual_value, 0);

    VERIFY_SIGACTION_CALLS ();
    VERIFY_SIGNAL_CALLS ();

    mctest_assert_str_eq (execvp__file__captured, "qwe");
    ck_assert_int_eq (execvp__args__captured->len, 11);

    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 0), "qwe");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 1), "-a");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 2), "'aa bb'");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 3), "-b");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 4), "-c");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 5), "cc");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 6), "-d");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 7), "\"dd ee\"");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 8), "-f");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 9), "ff\\ gg");
    mctest_assert_str_eq (g_ptr_array_index (execvp__args__captured, 10), "foo.txt");

    /* All exec* calls is mocked, so call to _exit() function with 127 status code it's a normal situation */
    ck_assert_int_eq (my_exit__status__captured, 127);
}
/* *INDENT-OFF* */
END_TEST
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    TCase *tc_core;

    tc_core = tcase_create ("Core");

    tcase_add_checked_fixture (tc_core, setup, teardown);

    /* Add new tests here: *************** */
    tcase_add_test (tc_core, fork_child);
    tcase_add_test (tc_core, fork_child_tokens);
    tcase_add_test (tc_core, fork_child_tokens2);
    /* *********************************** */

    return mctest_run_all (tc_core);
}

/* --------------------------------------------------------------------------------------------- */
