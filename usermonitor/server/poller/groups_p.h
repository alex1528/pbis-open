/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.  You should have received a copy of the GNU General
 * Public License along with this program.  If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        groups_p.h
 *
 * Abstract:
 *
 *        User monitor service for local users and groups
 * 
 *        Functions internal to this package for enumerating and tracking
 *        users.
 *
 * Authors: Kyle Stemen <kstemen@beyondtrust.com>
 * 
 */
#ifndef __GROUPS_P_H__
#define __GROUPS_P_H__

VOID
UmnSrvFreeGroupContents(
    PUSER_MONITOR_GROUP pGroup
    );

DWORD
UmnSrvReadGroup(
    PCSTR pParentKey,
    PSTR pName,
    PUSER_MONITOR_GROUP pResult
    );

DWORD
UmnSrvFindDeletedGroups(
    PLW_EVENTLOG_CONNECTION pEventlog,
    HANDLE hReg,
    PCSTR pGroupKeyName,
    HKEY hGroups,
    long long Now
    );

DWORD
UmnSrvUpdateGroups(
    HANDLE hLsass,
    PLW_EVENTLOG_CONNECTION pEventlog,
    HANDLE hReg,
    HKEY hParameters,
    long long PreviousRun,
    long long Now
    );

DWORD
UmnSrvWriteGroupMemberEvent(
    PLW_EVENTLOG_CONNECTION pEventlog,
    long long Now,
    PCSTR pGroupKeyName,
    long long PreviousRun,
    BOOLEAN AddMember,
    BOOLEAN OnlyGidChange,
    PCSTR pUserName,
    DWORD Gid,
    PCSTR pGroupName
    );

DWORD
UmnSrvFindDeletedGroupMembers(
    PLW_EVENTLOG_CONNECTION pEventlog,
    HANDLE hReg,
    PCSTR pGroupKeyName,
    HKEY hMembers,
    long long Now,
    BOOLEAN GidOnlyChange,
    DWORD Gid,
    PCSTR pGroupName
    );

#endif /* __GROUPS_P_H__ */
