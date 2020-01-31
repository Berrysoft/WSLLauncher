//
//    Copyright (C) Microsoft.  All rights reserved.
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#pragma once
#include <wslapi.h>

// This error definition is present in the Spring Creators Update SDK.
#ifndef ERROR_LINUX_SUBSYSTEM_NOT_PRESENT
#define ERROR_LINUX_SUBSYSTEM_NOT_PRESENT 414L
#endif // !ERROR_LINUX_SUBSYSTEM_NOT_PRESENT

class WslApiLoader
{
public:
    WslApiLoader(std::wstring_view distributionName);
    ~WslApiLoader();

    bool WslIsOptionalComponentInstalled() noexcept;

    bool WslIsDistributionRegistered() noexcept;

    void WslRegisterDistribution();

    void WslConfigureDistribution(ULONG defaultUID, WSL_DISTRIBUTION_FLAGS wslDistributionFlags);

    DWORD WslLaunchInteractive(PCWSTR command, BOOL useCurrentWorkingDirectory);

    HANDLE WslLaunch(PCWSTR command, BOOL useCurrentWorkingDirectory, HANDLE stdIn, HANDLE stdOut, HANDLE stdErr);

private:
    std::wstring _distributionName;
    bool _isInstalled;
};

extern WslApiLoader g_wslApi;
