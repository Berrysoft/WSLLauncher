//
//    Copyright (C) Microsoft.  All rights reserved.
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#include "stdafx.h"

#include "DistributionInfo.h"
#include "Helpers.h"
#include "WslApiLoader.h"

static inline bool WslLaunchIgnoreReturn(std::wstring_view command, bool use_pwd)
{
    try
    {
        DWORD exitCode = g_wslApi.WslLaunchInteractive(command.data(), use_pwd);
        if (exitCode) return false;
    }
    catch (wil::ResultException const&)
    {
        return false;
    }
    return true;
}

bool DistributionInfo::CreateUser(std::wstring_view userName)
{
    // Create the user account.
    std::wstring commandLine = L"/usr/sbin/useradd -m ";
    commandLine += userName;
    if (!WslLaunchIgnoreReturn(commandLine.c_str(), true))
    {
        return false;
    }

    // Add the user account to any relevant groups.
    commandLine = L"/usr/sbin/usermod -aG adm,cdrom,sudo,dip,plugdev ";
    commandLine += userName;
    if (!WslLaunchIgnoreReturn(commandLine.c_str(), true))
    {
        // Delete the user if the group add command failed.
        commandLine = L"/usr/sbin/userdel -r ";
        commandLine += userName;
        WslLaunchIgnoreReturn(commandLine.c_str(), true);
        return false;
    }

    return true;
}

ULONG DistributionInfo::QueryUid(std::wstring_view userName)
{
    // Create a pipe to read the output of the launched process.
    wil::unique_handle readPipe;
    wil::unique_handle writePipe;
    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, true };
    ULONG uid = UID_INVALID;
    if (CreatePipe(&readPipe, &writePipe, &sa, 0))
    {
        // Query the UID of the supplied username.
        std::wstring command = L"/usr/bin/id -u ";
        command += userName;
        wil::unique_handle child{ g_wslApi.WslLaunch(command.c_str(), true, GetStdHandle(STD_INPUT_HANDLE), writePipe.get(), GetStdHandle(STD_ERROR_HANDLE)) };
        // Wait for the child to exit and ensure process exited successfully.
        wil::handle_wait(child.get());
        DWORD exitCode;
        THROW_IF_WIN32_BOOL_FALSE(GetExitCodeProcess(child.get(), &exitCode));
        if (exitCode)
        {
            throw wil::ResultException(E_INVALIDARG);
        }

        child = nullptr;
        char buffer[64];
        DWORD bytesRead;

        // Read the output of the command from the pipe and convert to a UID.
        THROW_IF_WIN32_BOOL_FALSE(ReadFile(readPipe.get(), buffer, (sizeof(buffer) - 1), &bytesRead, nullptr));
        buffer[bytesRead] = ANSI_NULL;
        uid = std::strtoul(buffer, nullptr, 10);
    }

    return uid;
}
