#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)

    // On Intel, defer to the upstream submodule header by including it
    // through its absolute path so we don't recursively re-include ourselves.
    #include_next <lib/platform_common/qintrin.h>

#elif defined(__aarch64__) || defined(_M_ARM64)

    #include "qintrin_neon_shim.h"   // your NEON emulation of the AVX2 subset

#else
    #error "Unsupported target architecture"
#endif