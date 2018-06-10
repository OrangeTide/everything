/* jdm_embed.h : header file to help embed large binaries - public domain */
/*
 * To the extent possible under law, I have waived all copyright and related or
 * neighboring rights to this source code.
 *
 * For full legal text see
 *         https://creativecommons.org/publicdomain/zero/1.0/legalcode.txt
 */
/* USAGE:
 *
 *   JDM_EMBED_FILE(yourVarName, "yourfile.ext");
 *
 * this embed the data from the file and create two constants:
 *
 *   const char yourVarName[];
 *   const size_t yourVarName_len;
 *
 * To only get the definitions, appropriate for placing in header files:
 *
 *   JDM_EMBED_FILE_DEFS(yourVarName);
 *
 * BUGS:
 *
 * + using -ggdb has errors if there are no other constants/literals defined.
 *   (a single string literal is good enough to work around the issue)
 *
 */
#ifndef JDM_EMBED_H
#define JDM_EMBED_H

#include <stdint.h>

/* assembler directive for length constant (size_t type) */
#define JDM_EMBED_SIZE_TYPE size_t

#if (__SIZEOF_SIZE_T__ == 4)
# define JDM_EMBED_SIZE_TYPE_ASM ".long"
#elif (__SIZEOF_SIZE_T__ == 8)
# define JDM_EMBED_SIZE_TYPE_ASM ".quad"
#else
# warning unable to detect size of size_t type
#endif

/* assembler section */
#if defined(__APPLE__)
# define JDM_EMBED_ASM_SECTION ".const_data"
# error TODO: MacOS support is not finished yet
#elif defined (__WIN32__)
# define JDM_EMBED_ASM_SECTION ".section\t.rdata,\"dr\""
#else
# define JDM_EMBED_ASM_SECTION ".section\t.rodata"
#endif

/* assembler incbin */
#define JDM_EMBED_ASM_INCBIN(filename) ".incbin\t" #filename

/* assembler alignment */
#if defined(__arm__)
# define JDM_EMBED_ASM_ALIGN ".align 6" /* 64 bytes */
#elif defined(__GNUC__)
# define JDM_EMBED_ASM_ALIGN ".balign 64"
#else
# define JDM_EMBED_ASM_ALIGN ".align 64"
#endif

/* assembler symbol names */
#define JDM_EMBED_ASM_SYM(identifier,suffix) #identifier #suffix
#define JDM_EMBED_ASM_SYMLOCAL(identifier,suffix) "." #identifier #suffix

/* assembler type information */
#if defined(__WIN32__)
# define JDM_EMBED_ASM_TYPE(identifier) /* do nothing */
# define JDM_EMBED_ASM_SIZE(identifier) /* do nothing */
#else
# define JDM_EMBED_ASM_TYPE(identifier) \
	".type\t" JDM_EMBED_ASM_SYM(identifier,) ", @object"
# define JDM_EMBED_ASM_SIZE(identifier) \
	".size\t" JDM_EMBED_ASM_SYM(identifier,) ", . - " JDM_EMBED_ASM_SYM(identifier,)
#endif

#if defined(_MSC_VER)
# error Visual Studio is not supported.
/* we'd have to use a very different process for including binaries because the
 * inline assembler does not support any of the features we need. */
#else
# define JDM_EMBED_ASM(identifier, filename) \
	".globl\t" JDM_EMBED_ASM_SYM(identifier,) "\n" \
	"\t" JDM_EMBED_ASM_SECTION "\n" \
	"\t" JDM_EMBED_ASM_TYPE(identifier) "\n" \
	"\t" JDM_EMBED_ASM_ALIGN "\n" \
	JDM_EMBED_ASM_SYM(identifier,) ":\n" \
	JDM_EMBED_ASM_INCBIN(filename) "\n" \
	"\t.equ\t" JDM_EMBED_ASM_SYMLOCAL(identifier,_length) ", . - " JDM_EMBED_ASM_SYM(identifier,) "\n" \
	"\t.byte\t0\n" \
	"\t" JDM_EMBED_ASM_SIZE(identifier) "\n" \
	"\t.align 4\n" \
	"\t.globl\t" JDM_EMBED_ASM_SYM(identifier,) "_len\n" \
	JDM_EMBED_ASM_SYM(identifier,_len) ":\n" \
	"\t" JDM_EMBED_SIZE_TYPE_ASM "\t" JDM_EMBED_ASM_SYMLOCAL(identifier,) "_length" "\n" \
	"\t" ".text"

# define JDM_EMBED_FILE_DEFS(base_identifier) \
	extern const unsigned char base_identifier[]; \
	extern const JDM_EMBED_SIZE_TYPE base_identifier##_len
# define JDM_EMBED_FILE(base_identifier, filename) \
	__asm__( JDM_EMBED_ASM(base_identifier, filename) "\n" ); \
	JDM_EMBED_FILE_DEFS(base_identifier)
#endif
#endif
