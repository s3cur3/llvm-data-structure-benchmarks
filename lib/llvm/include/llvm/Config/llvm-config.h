/* Hacked by Tyler to match X-Plane's config! */

#ifndef LLVM_CONFIG_H
#define LLVM_CONFIG_H

/* Define if LLVM_ENABLE_DUMP is enabled */
#define LLVM_ENABLE_DUMP 0

/* Define if we link Polly to the tools */
#define LINK_POLLY_INTO_TOOLS 0

/* Target triple LLVM will generate code for by default */
#define LLVM_DEFAULT_TARGET_TRIPLE 0

/* Define if this is Unixish platform */
#define LLVM_ON_UNIX (APL||LIN)

/* Define if this is Win32ish platform */
#define LLVM_ON_WIN32 IBM

/* Major version of the LLVM API */
#define LLVM_VERSION_MAJOR 7

/* Minor version of the LLVM API */
#define LLVM_VERSION_MINOR 0

/* Patch version of the LLVM API */
#define LLVM_VERSION_PATCH 0

#endif
