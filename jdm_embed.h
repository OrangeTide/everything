/* jdm_embed.h : header file to help embed large binaries - public domain */

/* USAGE:
 *
 * JDM_EMBED_FILE(yourVarName, "yourfile.ext");
 *
 * BUGS:
 * + using -ggdb has errors if there are no other constants/literals defined. (a single string literal is good enough)
 *
 */
#ifndef JDM_EMBED_H
#define JDM_EMBED_H

#include <stdint.h>

#define JDM_EMBED_SIZE_TYPE size_t
#if (__SIZEOF_SIZE_T__ == 4)
#define JDM_EMBED_SIZE_TYPE_ASM ".4byte"
#elif (__SIZEOF_SIZE_T__ == 8)
#define JDM_EMBED_SIZE_TYPE_ASM ".8byte"
#else
#warning unable to detect size of size_t type
#endif

/* section */
#if defined(__APPLE__)
# define JDM_EMBED_ASM_SECTION ".const_data"
# error TODO: MacOS support is not finished yet
#else
# define JDM_EMBED_ASM_SECTION ".section\t.rodata"
#endif

/* incbin */
#if defined(__ghs__)
# define JDM_EMBED_ASM_INCBIN "INCBIN "
#else
# define JDM_EMBED_ASM_INCBIN ".incbin\t"
#endif

/* alignment */
#if defined(__arm__)
# define JDM_EMBED_ASM_ALIGN ".align 6" /* 64 bytes */
#elif defined(__GNUC__)
# define JDM_EMBED_ASM_ALIGN ".balign 64"
#else
# define JDM_EMBED_ASM_ALIGN ".align 64"
#endif

/* preferred assembler symbol */
#if defined(__WIN32__)
#define JDM_EMBED_ASM_SYM(identifier,suffix) " " #identifier #suffix
# error TODO: windows support is not finished yet
#else
# define JDM_EMBED_ASM_SYM(identifier,suffix) #identifier #suffix
#endif

#if defined(_MSC_VER)
# error TODO: visual studio is not supported
#else
# define JDM_EMBED_ASM(identifier, filename) \
	".globl\t" JDM_EMBED_ASM_SYM(identifier,) "\n" \
	"\t" JDM_EMBED_ASM_SECTION "\n" \
	"\t.type\t" JDM_EMBED_ASM_SYM(identifier,) ", @object" "\n" \
	"\t" JDM_EMBED_ASM_ALIGN "\n" \
	JDM_EMBED_ASM_SYM(identifier,) ":\n" \
	JDM_EMBED_ASM_INCBIN #filename "\n" \
	"\t.equ\t" "." JDM_EMBED_ASM_SYM(identifier,_length) ", . - " JDM_EMBED_ASM_SYM(identifier,) "\n" \
	"\t.byte\t0\n" \
	"\t.size\t" JDM_EMBED_ASM_SYM(identifier,) ", . - " JDM_EMBED_ASM_SYM(identifier,) "\n" \
	"\t.align 4\n" \
	"\t.globl\t" JDM_EMBED_ASM_SYM(identifier,) "_len\n" \
	#identifier "_len:\n" \
	"\t" JDM_EMBED_SIZE_TYPE_ASM "\t" "." JDM_EMBED_ASM_SYM(identifier,) "_length"

# define JDM_EMBED_FILE(base_identifier, filename) \
	__asm__( JDM_EMBED_ASM(base_identifier, filename) "\n" ); \
	extern const unsigned char base_identifier[]; \
	extern const JDM_EMBED_SIZE_TYPE base_identifier##_len
#endif
#endif
