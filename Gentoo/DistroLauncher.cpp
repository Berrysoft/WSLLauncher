//
//    Copyright (C) Microsoft.  All rights reserved.
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#include "stdafx.h"

#include "DistributionInfo.h"
#include "Helpers.h"
#include "WslApiLoader.h"
#include <iostream>
#include <locale>

// Commandline arguments:
constexpr std::wstring_view ARG_CONFIG = L"config";
constexpr std::wstring_view ARG_CONFIG_DEFAULT_USER = L"--default-user";
constexpr std::wstring_view ARG_INSTALL = L"install";
constexpr std::wstring_view ARG_INSTALL_ROOT = L"--root";
constexpr std::wstring_view ARG_RUN = L"run";
constexpr std::wstring_view ARG_RUN_C = L"-c";

// Helper class for calling WSL Functions:
// https://msdn.microsoft.com/en-us/library/windows/desktop/mt826874(v=vs.85).aspx
WslApiLoader g_wslApi(DistributionInfo::Name);

static void InstallDistribution(bool createUser);
static void SetDefaultUser(std::wstring_view userName);

void InstallDistribution(bool createUser)
{
    // Register the distribution.
    Helpers::PrintMessage(MSG_STATUS_INSTALLING);
    g_wslApi.WslRegisterDistribution();

    // Delete /etc/resolv.conf to allow WSL to generate a version based on Windows networking information.
    g_wslApi.WslLaunchInteractive(L"/bin/rm /etc/resolv.conf", true);

    // Create a user account.
    if (createUser)
    {
        Helpers::PrintMessage(MSG_CREATE_USER_PROMPT);
        std::wstring userName;
        do
        {
            userName = Helpers::GetUserInput(MSG_ENTER_USERNAME, 32);

        } while (!DistributionInfo::CreateUser(userName));

        // Set this user account as the default.
        SetDefaultUser(userName);
    }
}

void SetDefaultUser(std::wstring_view userName)
{
    // Query the UID of the given user name and configure the distribution
    // to use this UID as the default.
    ULONG uid = DistributionInfo::QueryUid(userName);
    if (uid == UID_INVALID)
    {
        throw wil::ResultException(E_INVALIDARG);
    }

    g_wslApi.WslConfigureDistribution(uid, WSL_DISTRIBUTION_FLAGS_DEFAULT);
}

int wmain(int argc, wchar_t const* argv[])
{
    // Update the title bar of the console window.
    SetConsoleTitle(DistributionInfo::WindowTitle.data());

    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale(""));

    // Initialize a vector of arguments.
    std::vector<std::wstring_view> arguments(argv + 1, argv + argc);

    // Ensure that the Windows Subsystem for Linux optional component is installed.
    if (!g_wslApi.WslIsOptionalComponentInstalled())
    {
        Helpers::PrintMessage(MSG_MISSING_OPTIONAL_COMPONENT);
        if (arguments.empty())
        {
            Helpers::PromptForInput();
        }
        return 1;
    }

    try
    {
        // Install the distribution if it is not already.
        bool installOnly = ((arguments.size() > 0) && (arguments[0] == ARG_INSTALL));
        if (!g_wslApi.WslIsDistributionRegistered())
        {

            // If the "--root" option is specified, do not create a user account.
            bool useRoot = ((installOnly) && (arguments.size() > 1) && (arguments[1] == ARG_INSTALL_ROOT));
            InstallDistribution(!useRoot);
            Helpers::PrintMessage(MSG_INSTALL_SUCCESS);
        }

        // Parse the command line arguments.
        DWORD exitCode{ 0 };
        if (arguments.empty())
        {
            exitCode = g_wslApi.WslLaunchInteractive(L"", false);
        }
        else if ((arguments[0] == ARG_RUN) || (arguments[0] == ARG_RUN_C))
        {
            std::wstring command;
            for (size_t index = 1; index < arguments.size(); index += 1)
            {
                command += L" ";
                command += arguments[index];
            }

            exitCode = g_wslApi.WslLaunchInteractive(command.c_str(), true);
        }
        else if (arguments[0] == ARG_CONFIG)
        {
            if (arguments.size() == 3 && arguments[1] == ARG_CONFIG_DEFAULT_USER)
            {
                SetDefaultUser(arguments[2]);
            }
            else
            {
                throw wil::ResultException(E_INVALIDARG);
            }
        }
        else
        {
            Helpers::PrintMessage(MSG_USAGE);
            return exitCode;
        }
    }
    catch (wil::ResultException const& e)
    {
        HRESULT hr = e.GetErrorCode();
        if (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
        {
            Helpers::PrintMessage(MSG_INSTALL_ALREADY_EXISTS);
        }
        else if (hr == HRESULT_FROM_WIN32(ERROR_LINUX_SUBSYSTEM_NOT_PRESENT))
        {
            Helpers::PrintMessage(MSG_MISSING_OPTIONAL_COMPONENT);
        }
        else
        {
            Helpers::PrintErrorMessage(hr);
        }

        if (arguments.empty())
        {
            Helpers::PromptForInput();
        }
        return 1;
    }

    return 0;
}