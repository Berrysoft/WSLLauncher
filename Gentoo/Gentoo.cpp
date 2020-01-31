//
//    Copyright (C) Microsoft.  All rights reserved.
//    Copyright (C) 2020 Berrysoft
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#include "stdafx.h"

#include "Helpers.h"
#include "WslDistribution.h"

using namespace std;

// The name of the distribution. This will be displayed to the user via
// wslconfig.exe and in other places. It must conform to the following
// regular expression: ^[a-zA-Z0-9._-]+$
//
// WARNING: This value must not change between versions of your app,
// otherwise users upgrading from older versions will see launch failures.
constexpr wstring_view DistributionName = L"Gentoo";

// The title bar for the console window while the distribution is installing.
constexpr wstring_view WindowTitle = L"Gentoo";

// Commandline arguments:
constexpr wstring_view ARG_CONFIG = L"config";
constexpr wstring_view ARG_CONFIG_DEFAULT_USER = L"--default-user";
constexpr wstring_view ARG_INSTALL = L"install";
constexpr wstring_view ARG_INSTALL_ROOT = L"--root";
constexpr wstring_view ARG_UNINSTALL = L"uninstall";
constexpr wstring_view ARG_RUN = L"run";
constexpr wstring_view ARG_RUN_C = L"-c";

WslDistribution distro(DistributionName);

int wmain(int argc, wchar_t const* argv[])
{
    // Fix localization
    locale::global(locale(""));
    wcout.imbue(locale(""));

    // Update the title bar of the console window.
    SetConsoleTitle(WindowTitle.data());

    // Initialize a vector of arguments.
    vector<wstring_view> arguments(argv + 1, argv + argc);

    // Ensure that the Windows Subsystem for Linux optional component is installed.
    if (!WslDistribution::IsOptionalComponentInstalled())
    {
        PrintMessage(MSG_MISSING_OPTIONAL_COMPONENT);
        if (arguments.empty())
        {
            PromptForInput();
        }
        return 1;
    }

    try
    {
        // Install the distribution if it is not already.
        bool installOnly = ((arguments.size() > 0) && (arguments[0] == ARG_INSTALL));
        if (!distro.IsRegistered())
        {

            // If the "--root" option is specified, do not create a user account.
            bool useRoot = ((installOnly) && (arguments.size() > 1) && (arguments[1] == ARG_INSTALL_ROOT));
            distro.Install(!useRoot);
            PrintMessage(MSG_INSTALL_SUCCESS);
            return 0;
        }

        // Parse the command line arguments.
        DWORD exitCode{ 0 };
        if (arguments.empty())
        {
            exitCode = distro.LaunchInteractive(L"", false);
        }
        else if (arguments[0] == ARG_UNINSTALL)
        {
            distro.Uninstall();
        }
        else if ((arguments[0] == ARG_RUN) || (arguments[0] == ARG_RUN_C))
        {
            wostringstream command;
            for (size_t index = 1; index < arguments.size(); index += 1)
            {
                command << L' ';
                command << arguments[index];
            }

            exitCode = distro.LaunchInteractive(command.str(), true);
        }
        else if (arguments[0] == ARG_CONFIG)
        {
            if (arguments.size() == 3 && arguments[1] == ARG_CONFIG_DEFAULT_USER)
            {
                distro.SetDefaultUser(arguments[2]);
            }
            else
            {
                throw wil::ResultException(E_INVALIDARG);
            }
        }
        else
        {
            PrintMessage(MSG_USAGE);
            return exitCode;
        }
    }
    catch (wil::ResultException const& e)
    {
        HRESULT hr = e.GetErrorCode();
        if (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
        {
            PrintMessage(MSG_INSTALL_ALREADY_EXISTS);
        }
        else if (hr == HRESULT_FROM_WIN32(ERROR_LINUX_SUBSYSTEM_NOT_PRESENT))
        {
            PrintMessage(MSG_MISSING_OPTIONAL_COMPONENT);
        }
        else
        {
            PrintErrorMessage(hr);
        }

        if (arguments.empty())
        {
            PromptForInput();
        }
        return 1;
    }

    return 0;
}