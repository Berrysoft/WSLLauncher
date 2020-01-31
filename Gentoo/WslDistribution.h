//
//    Copyright (C) 2020 Berrysoft
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#pragma once

#include <wslapi.h>

class WslDistribution
{
private:
    std::wstring m_name;

public:
    static bool IsOptionalComponentInstalled() noexcept;

    WslDistribution(std::wstring_view name) : m_name(name) {}

    bool IsRegistered() noexcept;
    DWORD LaunchInteractive(std::wstring_view command, bool useCurrentWorkingDirectory);
    HANDLE Launch(std::wstring_view command, bool useCurrentWorkingDirectory, HANDLE stdIn, HANDLE stdOut, HANDLE stdErr);

private:
    bool LaunchIgnoreReturn(std::wstring_view command, bool useCurrentWorkingDirectory);

    void Register();
    void Configure(ULONG defaultUID, WSL_DISTRIBUTION_FLAGS wslDistributionFlags);

    bool CreateUser(std::wstring_view userName);
    std::optional<ULONG> QueryUid(std::wstring_view userName);

public:
    void SetDefaultUser(std::wstring_view userName);

    void Install(bool createUser);
    void Uninstall();
};
