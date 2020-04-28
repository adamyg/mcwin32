#ifndef MC_WIN32_DRIVE_H
#define MC_WIN32_DRIVE_H
/* -*- mode: c; indent-width: 4; -*- */
/*
   win32 Change-drive command 
 
   Copyright (C) 2012
   The Free Software Foundation, Inc.

   Written by:
   Adam Young 2012

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

void drive_cmd (void);
void drive_cmd_a (void);
void drive_cmd_b (void);

const char *drive_getcwd(int drive);

#endif
