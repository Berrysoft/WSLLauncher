//
//    Copyright (C) 2020 Berrysoft
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#include "stdafx.h"

#include "Helpers.h"
#include "WslDistribution.h"

using namespace std;

bool WslDistribution::IsRegistered() noexcept
{
    return WslIsDistributionRegistered(m_name.c_str());
}

DWORD WslDistribution::LaunchInteractive(wstring_view command, bool useCurrentWorkingDirectory)
{
    DWORD exitCode{ 0 };
    try
    {
        THROW_IF_FAILED(WslLaunchInteractive(m_name.c_str(), command.data(), useCurrentWorkingDirectory, &exitCode));
    }
    catch (wil::ResultException const& e)
    {
        PrintMessage(MSG_WSL_LAUNCH_INTERACTIVE_FAILED, command, e.GetErrorCode());
        throw;
    }
    return exitCode;
}

HANDLE WslDistribution::Launch(wstring_view command, bool useCurrentWorkingDirectory, HANDLE stdIn, HANDLE stdOut, HANDLE stdErr)
{
    HANDLE process{ nullptr };
    try
    {
        THROW_IF_FAILED(WslLaunch(m_name.c_str(), command.data(), useCurrentWorkingDirectory, stdIn, stdOut, stdErr, &process));
    }
    catch (wil::ResultException const& e)
    {
        PrintMessage(MSG_WSL_LAUNCH_FAILED, command.data(), e.GetErrorCode());
        throw;
    }
    return process;
}

bool WslDistribution::LaunchIgnoreReturn(wstring_view command, bool useCurrentWorkingDirectory)
{
    try
    {
        DWORD exitCode = LaunchInteractive(command, useCurrentWorkingDirectory);
        if (exitCode) return false;
    }
    catch (wil::ResultException const&)
    {
        return false;
    }
    return true;
}

void WslDistribution::Register()
{
    try
    {
        THROW_IF_FAILED(WslRegisterDistribution(m_name.c_str(), L"rootfs.tar.gz"));
    }
    catch (wil::ResultException const& e)
    {
        PrintMessage(MSG_WSL_REGISTER_DISTRIBUTION_FAILED, e.GetErrorCode());
        throw;
    }
}

void WslDistribution::Configure(ULONG defaultUID, WSL_DISTRIBUTION_FLAGS wslDistributionFlags)
{
    try
    {
        THROW_IF_FAILED(WslConfigureDistribution(m_name.c_str(), defaultUID, wslDistributionFlags));
    }
    catch (wil::ResultException const& e)
    {
        PrintMessage(MSG_WSL_CONFIGURE_DISTRIBUTION_FAILED, e.GetErrorCode());
        throw;
    }
}

bool WslDistribution::CreateUser(wstring_view userName)
{
    // Create the user account.
    wstring commandLine = L"/usr/sbin/useradd -m ";
    commandLine += userName;
    if (!LaunchIgnoreReturn(commandLine, true))
    {
        return false;
    }

    // Add the user account to any relevant groups.
    commandLine = L"/usr/sbin/usermod -aG adm,cdrom,sudo,dip,plugdev ";
    commandLine += userName;
    if (!LaunchIgnoreReturn(commandLine, true))
    {
        // Delete the user if the group add command failed.
        commandLine = L"/usr/sbin/userdel -r ";
        commandLine += userName;
        LaunchIgnoreReturn(commandLine, true);
        return false;
    }

    return true;
}

optional<ULONG> WslDistribution::QueryUid(wstring_view userName)
{
    // Create a pipe to read the output of the launched process.
    wil::unique_handle readPipe;
    wil::unique_handle writePipe;
    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, true };
    if (CreatePipe(&readPipe, &writePipe, &sa, 0))
    {
        // Query the UID of the supplied username.
        wstring command = L"/usr/bin/id -u ";
        command += userName;
        wil::unique_handle child{ Launch(command, true, GetStdHandle(STD_INPUT_HANDLE), writePipe.get(), GetStdHandle(STD_ERROR_HANDLE)) };
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
        return strtoul(buffer, nullptr, 10);
    }

    return nullopt;
}

void WslDistribution::SetDefaultUser(std::wstring_view userName)
{
    auto uid = QueryUid(userName);
    if (!uid)
    {
        throw wil::ResultException(E_INVALIDARG);
    }
    Configure(*uid, WSL_DISTRIBUTION_FLAGS_DEFAULT);
}

template <size_t N>
static wstring GetUserInput(DWORD promptMsg)
{
    PrintMessage(promptMsg);
    wchar_t inputBuffer[N + 1] = {};
    auto count = wcin.readsome(inputBuffer, N);
    inputBuffer[count] = L'\0';

    // Throw away any additional chracters that did not fit in the buffer.
    wchar_t wch;
    do
    {
        wch = wcin.get();
    } while ((wch != L'\n') && (wch != WEOF));

    return inputBuffer;
}

void WslDistribution::Install(bool createUser)
{
    // Register the distribution.
    PrintMessage(MSG_STATUS_INSTALLING);
    Register();

    // Delete /etc/resolv.conf to allow WSL to generate a version based on Windows networking information.
    LaunchInteractive(L"/bin/rm /etc/resolv.conf", true);

    // Create a user account.
    if (createUser)
    {
        PrintMessage(MSG_CREATE_USER_PROMPT);
        wstring userName;
        do
        {
            userName = GetUserInput<32>(MSG_ENTER_USERNAME);
        } while (!CreateUser(userName));

        // Set this user account as the default.
        SetDefaultUser(userName);
    }
}

void WslDistribution::Uninstall()
{
    try
    {
        THROW_IF_FAILED(WslUnregisterDistribution(m_name.c_str()));
    }
    catch (wil::ResultException const& e)
    {
        PrintMessage(MSG_WSL_UNREGISTER_DISTRIBUTION_FAILED, e.GetErrorCode());
        throw;
    }
}
