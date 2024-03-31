/* -*- mode: c; indent-width: 4; -*- */
/*
 * libtermemu console driver
 *
 * Copyright (c) 2024, Adam Young.
 * All rights reserved.
 *
 * This file is part of the WinXSH project.
 *
 * The applications are free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#if !defined(VK_0)
#define VK_0 0x30              // 0x30 - 0 key
#define VK_1 0x31              // 0x31 - 1 key
#define VK_2 0x32              // 0x32 - 2 key
#define VK_3 0x33              // 0x33 - 3 key
#define VK_4 0x34              // 0x34 - 4 key
#define VK_5 0x35              // 0x35 - 5 key
#define VK_6 0x36              // 0x36 - 6 key
#define VK_7 0x37              // 0x37 - 7 key
#define VK_8 0x38              // 0x38 - 8 key
#define VK_9 0x39              // 0x39 - 9 key

#define VK_A 0x41              // 0x41 - A key
#define VK_B 0x42              // 0x42 - B key
#define VK_C 0x43              // 0x43 - C key
#define VK_D 0x44              // 0x44 - D key
#define VK_E 0x45              // 0x45 - E key
#define VK_F 0x46              // 0x46 - F key
#define VK_G 0x47              // 0x47 - G key
#define VK_H 0x48              // 0x48 - H key
#define VK_I 0x49              // 0x49 - I key
#define VK_J 0x4A              // 0x4A - J key
#define VK_K 0x4B              // 0x4B - K key
#define VK_L 0x4C              // 0x4C - L key
#define VK_M 0x4D              // 0x4D - M key
#define VK_N 0x4E              // 0x4E - N key
#define VK_O 0x4F              // 0x4F - O key
#define VK_P 0x50              // 0x50 - P key
#define VK_Q 0x51              // 0x51 - Q key
#define VK_R 0x52              // 0x52 - R key
#define VK_S 0x53              // 0x53 - S key
#define VK_T 0x54              // 0x54 - T key
#define VK_U 0x55              // 0x55 - U key
#define VK_V 0x56              // 0x56 - V key
#define VK_W 0x57              // 0x57 - W key
#define VK_X 0x58              // 0x58 - X key
#define VK_Y 0x59              // 0x59 - Y key
#define VK_Z 0x5A              // 0x5A - Z key
#endif

#if !defined(VK_DBE_ALPHANUMERIC)
#define VK_DBE_ALPHANUMERIC 0x0f0
#define VK_DBE_KATAKANA 0x0f1
#define VK_DBE_HIRAGANA 0x0f2
#define VK_DBE_SBCSCHAR 0x0f3
#define VK_DBE_DBCSCHAR 0x0f4
#define VK_DBE_ROMAN 0x0f5
#define VK_DBE_NOROMAN 0x0f6
#define VK_DBE_ENTERWORDREGISTERMODE 0x0f7
#define VK_DBE_ENTERIMECONFIGMODE 0x0f8
#define VK_DBE_FLUSHSTRING 0x0f9
#define VK_DBE_CODEINPUT 0x0fa
#define VK_DBE_NOCODEINPUT 0x0fb
#define VK_DBE_DETERMINESTRING 0x0fc
#define VK_DBE_ENTERDLGCONVERSIONMODE 0x0fd
#endif

//end
