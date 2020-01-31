//
//    Copyright (C) Microsoft.  All rights reserved.
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#include "stdafx.h"

#include "Helpers.h"
#include <iostream>
#include <wil/resource.h>

namespace
{
    std::wstring FormatMessageHelperVa(DWORD messageId, va_list vaList);
    void PrintMessageVa(DWORD messageId, va_list vaList);
} // namespace

std::wstring Helpers::GetUserInput(DWORD promptMsg, DWORD maxCharacters)
{
    Helpers::PrintMessage(promptMsg);
    size_t bufferSize = maxCharacters + 1;
    std::unique_ptr<wchar_t[]> inputBuffer = std::make_unique<wchar_t[]>(bufferSize);
    std::wstring input;
    if (std::wcin.read(inputBuffer.get(), bufferSize))
    {
        input = inputBuffer.get();
    }

    // Throw away any additional chracters that did not fit in the buffer.
    wchar_t wch;
    do
    {
        wch = getwchar();
    } while ((wch != L'\n') && (wch != WEOF));

    return input;
}

void Helpers::PrintErrorMessage(HRESULT error) noexcept
{
    wil::unique_hlocal_string buffer;
    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, error, 0, (PWSTR)&buffer, 0, nullptr);
    Helpers::PrintMessage(MSG_ERROR_CODE, error, buffer.get());
}

void Helpers::PrintMessage(DWORD messageId, ...) noexcept
{
    va_list argList;
    va_start(argList, messageId);
    try
    {
        PrintMessageVa(messageId, argList);
    }
    catch (wil::ResultException const& e)
    {
        OutputDebugStringA(e.what());
    }
    va_end(argList);
}

void Helpers::PromptForInput() noexcept
{
    Helpers::PrintMessage(MSG_PRESS_A_KEY);
    _getwch();
}

namespace
{
    std::wstring FormatMessageHelperVa(DWORD messageId, va_list vaList)
    {
        wil::unique_hlocal_string buffer;
        DWORD written = ::FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, messageId, 0, (PWSTR)&buffer, 10, &vaList);
        if (written <= 0)
        {
            throw wil::ResultException(HRESULT_FROM_WIN32(GetLastError()));
        }
        return buffer.get();
    }

    void PrintMessageVa(DWORD messageId, va_list vaList)
    {
        std::wstring message = FormatMessageHelperVa(messageId, vaList);
        std::wcout << message;
    }
} // namespace
