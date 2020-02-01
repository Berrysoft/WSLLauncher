//
//    Copyright (C) Microsoft.  All rights reserved.
// Licensed under the terms described in the LICENSE file in the root of this project.
//

#pragma once

void PrintErrorMessage(HRESULT hr) noexcept;
void PrintMessage(DWORD messageId, ...) noexcept;
void PromptForInput() noexcept;
