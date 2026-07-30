#include "includes.h"
#include <thread>
#include <chrono>
#include <tchar.h>
#include <algorithm>
#include <iterator>

int __stdcall DllMain(HMODULE self, ulong_t reason, void* reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        // note; the unloader needs this to free us later on.
        g_module = self;

        HANDLE thread = CreateThread(nullptr, 0, Client::init, nullptr, 0, nullptr);
        if (!thread)
            return 0;

        CloseHandle(thread);
        return 1;
    }

    // note; every other reason has to return TRUE as well, this used to fall off the
    //       end of the function and hand the loader whatever was in eax.
    return 1;
}
