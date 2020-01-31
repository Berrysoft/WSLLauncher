//
//    Copyright (C) Microsoft.  All rights reserved.
//    Copyright (C) 2020 Berrysoft
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#include "stdafx.h"

#include "Helpers.h"

using namespace std;

wstring GetUserInput(DWORD promptMsg, DWORD maxCharacters)
{
    PrintMessage(promptMsg);
    size_t bufferSize = maxCharacters + 1;
    unique_ptr<wchar_t[]> inputBuffer = make_unique<wchar_t[]>(bufferSize);
    wstring input;
    if (wcin.read(inputBuffer.get(), bufferSize))
    {
        input = inputBuffer.get();
    }

    // Throw away any additional chracters that did not fit in the buffer.
    wchar_t wch;
    do
    {
        wch = wcin.get();
    } while ((wch != L'\n') && (wch != WEOF));

    return input;
}

static wstring FormatMessageHelperVa(DWORD messageId, va_list vaList)
{
    wil::unique_hlocal_string buffer;
    DWORD written = ::FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, messageId, 0, (PWSTR)&buffer, 10, &vaList);
    if (written <= 0)
    {
        throw wil::ResultException(HRESULT_FROM_WIN32(GetLastError()));
    }
    return buffer.get();
}

void PrintErrorMessage(HRESULT error) noexcept
{
    wil::unique_hlocal_string buffer;
    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, error, 0, (PWSTR)&buffer, 0, nullptr);
    PrintMessage(MSG_ERROR_CODE, error, buffer.get());
}

void PrintMessage(DWORD messageId, ...) noexcept
{
    va_list argList;
    va_start(argList, messageId);
    try
    {
        wstring message = FormatMessageHelperVa(messageId, argList);
        wcout << message;
    }
    catch (wil::ResultException const& e)
    {
        OutputDebugStringA(e.what());
    }
    va_end(argList);
}

void PromptForInput() noexcept
{
    PrintMessage(MSG_PRESS_A_KEY);
    wcin.get();
}
