#ifndef container_of
# if defined __GNUC__ && !defined __STRICT_ANSI__
#  define container_of(ptr, type, member) ({ \
	const typeof(((type*)0)->member)*__mptr = (ptr); \
	(type*)((char*)__mptr - offsetof(type,member));})
# else
#  define container_of(ptr, type, member) \
	((type*)(char*)(ptr) - offsetof(type, member))
# endif
#endif
