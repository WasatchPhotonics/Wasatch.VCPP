# Memory Profiling

At writing, there are no known memory leaks in Wasatch.VCPP.  The library is 
normally profiled using Valgrind with "--leak-check=full" on Ubuntu.  Sample
profile output on Ubuntu 20.04.2 LTS:

    $ valgrind --leak-check=full ./demo

    ==49221== Memcheck, a memory error detector
    ==49221== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
    ==49221== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
    ==49221== Command: ./demo
    ==49221==
    Wasatch.VCPP Demo (1.0.12)
    1 spectrometers found
    Found 785-OEM-L WP-00289 with 1024 pixels (772.14, 951.13)

    ...snip...

    Press [return] to disable laser...
    successfully disabled laser
    detector temperature 9.74 degC
    ==49221==
    ==49221== HEAP SUMMARY:
    ==49221==     in use at exit: 0 bytes in 0 blocks
    ==49221==   total heap usage: 4,291 allocs, 4,291 frees, 1,235,418 bytes allocated
    ==49221==
    ==49221== All heap blocks were freed -- no leaks are possible
    ==49221==
    ==49221== For lists of detected and suppressed errors, rerun with: -s
    ==49221== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

Note that if you don't call wp\_destroy\_driver at the end of execution, the 
application should still run and shutdown properly; however, memory-profiling
tools may report the WasatchVCPP::Driver singleton as an un-freed instance.
WasatchVCPP::Driver::destroy() was added to allow every last bit of allocated
memory to be released at shutdown such that memory-profilers can issue a clean
bill of health.

