/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_INC_DBGGUI_HXX
#define INCLUDED_VCL_INC_DBGGUI_HXX

#ifdef DBG_UTIL

class Window;

void DbgGUIInitSolarMutexCheck();
void DbgGUIDeInitSolarMutexCheck();
void DbgGUIStart();
void DbgDialogTest( Window* pWindow );

#define DBGGUI_INIT_SOLARMUTEXCHECK() DbgGUIInitSolarMutexCheck()
#define DBGGUI_DEINIT_SOLARMUTEXCHECK() DbgGUIDeInitSolarMutexCheck()
#define DBGGUI_START()          DbgGUIStart()

#define DBG_DIALOGTEST( pWindow )                   \
    if ( DbgIsDialog() )                            \
        DbgDialogTest( pWindow );

#else

#define DBGGUI_INIT_SOLARMUTEXCHECK()
#define DBGGUI_DEINIT_SOLARMUTEXCHECK()
#define DBGGUI_START()

#define DBG_DIALOGTEST( pWindow )

#endif

#endif // INCLUDED_VCL_INC_DBGGUI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
