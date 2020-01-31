//
//    Copyright (C) Microsoft.  All rights reserved.
//    Copyright (C) 2020 Berrysoft
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#include "stdafx.h"

#include "Helpers.h"
#include "WslApiLoader.h"

WslApiLoader::WslApiLoader(std::wstring_view distributionName) : _distributionName(distributionName), _isInstalled(false)
{
    wil::unique_hmodule wslApiDll{ LoadLibraryEx(L"wslapi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32) };
    if (wslApiDll)
    {
        _isInstalled = true;
    }
}

WslApiLoader::~WslApiLoader()
{
}

bool WslApiLoader::WslIsOptionalComponentInstalled() noexcept
{
    return _isInstalled;
}

bool WslApiLoader::WslIsDistributionRegistered() noexcept
{
    return ::WslIsDistributionRegistered(_distributionName.c_str());
}

void WslApiLoader::WslRegisterDistribution()
{
    try
    {
        THROW_IF_FAILED(::WslRegisterDistribution(_distributionName.c_str(), L"rootfs.tar.gz"));
    }
    catch (wil::ResultException const& e)
    {
        Helpers::PrintMessage(MSG_WSL_REGISTER_DISTRIBUTION_FAILED, e.GetErrorCode());
        throw;
    }
}

void WslApiLoader::WslConfigureDistribution(ULONG defaultUID, WSL_DISTRIBUTION_FLAGS wslDistributionFlags)
{
    try
    {
        THROW_IF_FAILED(::WslConfigureDistribution(_distributionName.c_str(), defaultUID, wslDistributionFlags));
    }
    catch (wil::ResultException const& e)
    {
        Helpers::PrintMessage(MSG_WSL_CONFIGURE_DISTRIBUTION_FAILED, e.GetErrorCode());
        throw;
    }
}

DWORD WslApiLoader::WslLaunchInteractive(PCWSTR command, BOOL useCurrentWorkingDirectory)
{
    DWORD exitCode{ 0 };
    try
    {
        THROW_IF_FAILED(::WslLaunchInteractive(_distributionName.c_str(), command, useCurrentWorkingDirectory, &exitCode));
    }
    catch (wil::ResultException const& e)
    {
        Helpers::PrintMessage(MSG_WSL_LAUNCH_INTERACTIVE_FAILED, command, e.GetErrorCode());
        throw;
    }
    return exitCode;
}

HANDLE WslApiLoader::WslLaunch(PCWSTR command, BOOL useCurrentWorkingDirectory, HANDLE stdIn, HANDLE stdOut, HANDLE stdErr)
{
    HANDLE process{ nullptr };
    try
    {
        THROW_IF_FAILED(::WslLaunch(_distributionName.c_str(), command, useCurrentWorkingDirectory, stdIn, stdOut, stdErr, &process));
    }
    catch (wil::ResultException const& e)
    {
        Helpers::PrintMessage(MSG_WSL_LAUNCH_FAILED, command, e.GetErrorCode());
        throw;
    }
    return process;
}
