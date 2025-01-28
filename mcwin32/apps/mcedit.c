/*
 * mcedit shim application.
 *
 * Copyright (c) 2024 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of WIN32 Midnight Commander.
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

#include "shim.h"

int
main(void)
{
    ApplicationShim(L"mcedit", L"mc.exe");
    return EXIT_FAILURE;
}

//end
