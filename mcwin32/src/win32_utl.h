/* -*- mode: c; indent-width: 4; -*- */
/*
   Adam Young 2021 - 2023

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

extern char *my_unquote (const char *cmd, int quotews);
extern const char *mc_isscript (const char *cmd);
extern const char **mc_busybox_cmds (unsigned *count);

/*end*/
