#ifndef RCC_PE_H
#define RCC_PE_H

#include <stdint.h>
#include <Windows.h>
#include "common.h"
/**
 * NOTE: structs in this file are in general NOT padded.
 */
// typedef enum e_machinetype
// {
//     IMAGE_FILE_MACHINE_UNKNOWN,
//     IMAGE_FILE_MACHINE_ALPHA,       // Alpha XP, 32-bit address space
//     IMAGE_FILE_MACHINE_ALPHA64,     // Alpha XP, 64-bit address space
//     IMAGE_FILE_MACHINE_AM33,        // Matsushita AM33
//     IMAGE_FILE_MACHINE_AMD64,       // x64
//     IMAGE_FILE_MACHINE_ARM,         // ARM little endian
//     IMAGE_FILE_MACHINE_ARM64,       // ARM64 little endian
//     IMAGE_FILE_MACHINE_ARM64EC,     // ABI b/w ARM64 and emulated AMD64
//     IMAGE_FILE_MACHINE_ARM64X,      // Binary format that allows both ARM64 and ARM64EC in the same file
//     IMAGE_FILE_MACHINE_ARMNT,       // ARM Thumb-2 little endian
//     IMAGE_FILE_MACHINE_AXP64,       // Alpha 64
//     IMAGE_FILE_MACHINE_EBC,         // EFI byte code
//     IMAGE_FILE_MACHINE_I386,        // Intel i386 or later (or compatible)
//     IMAGE_FILE_MACHINE_IA64,        // Intel Itanium processors
//     IMAGE_FILE_MACHINE_LOONGARCH32, // Loongarch 32-bit family
//     IMAGE_FILE_MACHINE_LOONGARCH64, // Loongarch 64-bit family
//     IMAGE_FILE_MACHINE_M32R,        // Mitsubishi M32R little endian
//     IMAGE_FILE_MACHINE_MIPS16,      // MIPS16
//     IMAGE_FILE_MACHINE_MIPSFPU,     // MIPS with FPU
//     IMAGE_FILE_MACHINE_MIPSFPU16,   // MIPS16 with FPU
//     IMAGE_FILE_MACHINE_POWERPC,     // Power PC little endian
//     IMAGE_FILE_MACHINE_POWERPCFP,   // Power PC with float support
//     IMAGE_FILE_MACHINE_R3000BE,     // MIPS I compatible 32-bit big endian
//     IMAGE_FILE_MACHINE_R3K,         // MIPS I 32-bit little endian
//     IMAGE_FILE_MACHINE_R4K,         // MIPS III compatible 64-bit endian
//     IMAGE_FILE_MACHINE_R10K,        // MIPS IV compatible 64-bit little endian
//     IMAGE_FILE_MACHINE_RISCV32,     // RISC-V 32-bit
//     IMAGE_FILE_MACHINE_RISCV64,     // RISC-V 64-bit
//     IMAGE_FILE_MACHINE_RISCV128,    // RISC-V 128-bit
//     IMAGE_FILE_MACHINE_SH3,         // Hitachi SH3
//     IMAGE_FILE_MACHINE_SH3DSP,      // Hitachi SH3 DSP
//     IMAGE_FILE_MACHINE_SH4,         // Hitachi SH4
//     IMAGE_FILE_MACHINE_SH5,         // Hitachi SH5
//     IMAGE_FILE_MACHINE_THUMB,       // Thumb
//     IMAGE_FILE_MACHINE_WCEMIPSV2,   // MIPS little endian WCE v2
// } e_machinetype_t; 

/**
 * Common-Object File Format (COFF).
 * This header is at the beginning of PE/object files
 * on Windows containing metadata pertinent to the file.
 */
#pragma pack(push, 1)
typedef struct coff_header
{
    u16 machine_type;
    u16 n_sections;
    u32 timestamp;
    // may want 32-bit ptr
    u32 ptr_to_sym_table;
    u32 n_symbols;
    u16 opt_header_sz;          // zero for object files, required for image files
    u16 characteristics_flags;  // In general, IMAGE_FILE_EXECUTABLE_IMAGE should be set
} coff_header_t;

typedef union _opt_header {
    IMAGE_OPTIONAL_HEADER32 opt_header_32;
    IMAGE_OPTIONAL_HEADER64 opt_header_64;
} opt_header_t;

typedef IMAGE_DATA_DIRECTORY idd;
typedef IMAGE_SECTION_HEADER section_header_t;

typedef struct _win_pe_obj
{
    coff_header_t coff_header;  // image and object files
    // this (may) be in the header, unsure right now
    // idd clr_runtime_header;         // .cormeta
} win_pe_obj_t;

typedef struct _win_pe_img
{
    u8 ms_dos_stub[0x3c];       // old ms-dos compatibility
    u32 signature;              // "PE\0\0"
    coff_header_t coff_header;  
    opt_header_t opt_header;    // image only

    // data directories (variable, check opt_header.NumberOfRvaAndSizes)
    idd export_table;               // .edata
    idd import_table;               // .idata
    idd resource_table;             // .rsrc
    idd exception_table;            // .pdata
    idd cert_table;
    idd base_reloc_table;           // .reloc
    idd debug_table;                // .debug
    idd arch_table;                 // must be NULL
    idd global_ptr_register;        // size = 0
    idd tls_table;                  // .tls
    idd load_cfg_table;             // load configuration structure
    idd bound_import;               // bound import table
    idd iat;                        // import address table
    idd delay_import_descriptor;
    idd reserved;                   // NULL
} win_pe_img_t;

/**
 * The Windows PE file format header.
 * This struct is not padded, so you may want
 * to extract its fields in performance-minded
 * applications.
 */
typedef union {
    win_pe_obj_t win_pe_obj;
    win_pe_img_t win_pe_img;
} win_pe_t;
#pragma pack(pop)


win_pe_t load_pe_from_file(const char* fpath, b8* OUT_is_image);

#endif