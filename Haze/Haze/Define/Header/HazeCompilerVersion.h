#pragma once

// C++版本检测
#if __cplusplus >= 202002L
    #define HAZE_CPP_VERSION 20
    #define HAZE_CPP20_OR_LATER 1
    #define HAZE_CPP17_OR_LATER 1
    #define HAZE_CPP14_OR_LATER 1
    #define HAZE_CPP11_OR_LATER 1
#elif __cplusplus >= 201703L
    #define HAZE_CPP_VERSION 17
    #define HAZE_CPP20_OR_LATER 0
    #define HAZE_CPP17_OR_LATER 1
    #define HAZE_CPP14_OR_LATER 1
    #define HAZE_CPP11_OR_LATER 1
#elif __cplusplus >= 201402L
    #define HAZE_CPP_VERSION 14
    #define HAZE_CPP20_OR_LATER 0
    #define HAZE_CPP17_OR_LATER 0
    #define HAZE_CPP14_OR_LATER 1
    #define HAZE_CPP11_OR_LATER 1
#elif __cplusplus >= 201103L
    #define HAZE_CPP_VERSION 11
    #define HAZE_CPP20_OR_LATER 0
    #define HAZE_CPP17_OR_LATER 0
    #define HAZE_CPP14_OR_LATER 0
    #define HAZE_CPP11_OR_LATER 1
#else
    #define HAZE_CPP_VERSION 98
    #define HAZE_CPP20_OR_LATER 0
    #define HAZE_CPP17_OR_LATER 0
    #define HAZE_CPP14_OR_LATER 0
    #define HAZE_CPP11_OR_LATER 0
#endif

// 编译器检测
#if defined(_MSC_VER)
    #define HAZE_COMPILER_MSVC 1
    #define HAZE_COMPILER_GCC 0
    #define HAZE_COMPILER_CLANG 0
    
    // MSVC版本检测
    #if _MSC_VER >= 1930
        #define HAZE_MSVC_VERSION 2022
    #elif _MSC_VER >= 1920
        #define HAZE_MSVC_VERSION 2019
    #elif _MSC_VER >= 1910
        #define HAZE_MSVC_VERSION 2017
    #elif _MSC_VER >= 1900
        #define HAZE_MSVC_VERSION 2015
    #else
        #define HAZE_MSVC_VERSION 0
    #endif
    
#elif defined(__GNUC__)
    #define HAZE_COMPILER_MSVC 0
    #define HAZE_COMPILER_GCC 1
    #define HAZE_COMPILER_CLANG 0
    #define HAZE_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
    
#elif defined(__clang__)
    #define HAZE_COMPILER_MSVC 0
    #define HAZE_COMPILER_GCC 0
    #define HAZE_COMPILER_CLANG 1
    #define HAZE_CLANG_VERSION (__clang_major__ * 100 + __clang_minor__)
    
#else
    #define HAZE_COMPILER_MSVC 0
    #define HAZE_COMPILER_GCC 0
    #define HAZE_COMPILER_CLANG 0
#endif

// 特性检测
// codecvt支持检测
#if defined(__cpp_lib_codecvt) || (HAZE_COMPILER_MSVC && HAZE_CPP11_OR_LATER)
    #define HAZE_HAS_CODECVT 1
#else
    #define HAZE_HAS_CODECVT 0
#endif

// filesystem支持检测
#if defined(__cpp_lib_filesystem) || (HAZE_COMPILER_MSVC && _MSC_VER >= 1914) || (HAZE_COMPILER_GCC && HAZE_GCC_VERSION >= 800)
    #define HAZE_HAS_FILESYSTEM 1
#else
    #define HAZE_HAS_FILESYSTEM 0
#endif

// string_view支持检测
#if defined(__cpp_lib_string_view) || HAZE_CPP17_OR_LATER
    #define HAZE_HAS_STRING_VIEW 1
#else
    #define HAZE_HAS_STRING_VIEW 0
#endif

// Windows UTF-8 locale支持检测（Windows 10 1903+）
#if HAZE_COMPILER_MSVC && defined(_WIN32)
    #define HAZE_HAS_WIN32_UTF8_LOCALE 1
#else
    #define HAZE_HAS_WIN32_UTF8_LOCALE 0
#endif

// 平台检测
#if defined(_WIN32) || defined(_WIN64)
    #define HAZE_PLATFORM_WINDOWS 1
    #define HAZE_PLATFORM_LINUX 0
    #define HAZE_PLATFORM_MAC 0
#elif defined(__linux__)
    #define HAZE_PLATFORM_WINDOWS 0
    #define HAZE_PLATFORM_LINUX 1
    #define HAZE_PLATFORM_MAC 0
#elif defined(__APPLE__)
    #define HAZE_PLATFORM_WINDOWS 0
    #define HAZE_PLATFORM_LINUX 0
    #define HAZE_PLATFORM_MAC 1
#else
    #define HAZE_PLATFORM_WINDOWS 0
    #define HAZE_PLATFORM_LINUX 0
    #define HAZE_PLATFORM_MAC 0
#endif

// 调试信息宏
// 字符串化宏（两级展开确保正确转换）
#define HAZE_STRINGIFY(x) #x
#define HAZE_TOSTRING(x) HAZE_STRINGIFY(x)

#define HAZE_CPP_VERSION_STRING "C++" HAZE_TOSTRING(HAZE_CPP_VERSION)

#if HAZE_COMPILER_MSVC
    #define HAZE_COMPILER_STRING "MSVC " HAZE_TOSTRING(HAZE_MSVC_VERSION)
#elif HAZE_COMPILER_GCC
    #define HAZE_COMPILER_STRING "GCC " __VERSION__
#elif HAZE_COMPILER_CLANG
    #define HAZE_COMPILER_STRING "Clang " __clang_version__
#else
    #define HAZE_COMPILER_STRING "Unknown"
#endif 