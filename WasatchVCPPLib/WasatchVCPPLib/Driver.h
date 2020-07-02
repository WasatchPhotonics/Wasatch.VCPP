#pragma once

#ifdef WASATCHVCPPLIB_EXPORTS
#define WASATCHVCPPLIB_API __declspec(dllexport)
#else
#define WASATCHVCPPLIB_API __declspec(dllimport)
#endif

namespace WasatchVCPP
{
    class Driver
    {
    public:
        Driver();

        int add();
    };

    WASATCHVCPPLIB_API int add();
}

class Driver
{
};

