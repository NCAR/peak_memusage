
AC_DEFUN([AX_MALLOC_FEATURES], [
  AC_MSG_NOTICE([Checking for platform-specific malloc features])

  AC_MSG_CHECKING([for mallinfo2()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [struct mallinfo mi;
                                   mi = mallinfo2();
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_MALLINFO2], [1], [Have malloc.h mallinfo2() interface.])],
                 [AC_MSG_RESULT([no])])
  AC_MSG_CHECKING([for mallinfo()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [struct mallinfo mi;
                                   mi = mallinfo();
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_MALLINFO], [1], [Have malloc.h mallinfo() interface.])],
                 [AC_MSG_RESULT([no])])

  AC_MSG_CHECKING([for malloc_usable_size()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [int *p, s;
                                   s = malloc_usable_size(p);
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_MALLOC_USABLE_SIZE], [1], [Have malloc.h malloc_usable_size() interface.])],
                 [AC_MSG_RESULT([no])])

  AC_MSG_CHECKING([for malloc_size()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc/malloc.h>
                                  ],
                                  [int *p=0, s=0;
                                   s = malloc_size(p);
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_MALLOC_SIZE], [1], [Have malloc.h malloc_size() interface.])],
                 [AC_MSG_RESULT([no])])

  AC_MSG_CHECKING([for __libc_malloc()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [int *p=0;
                                   p = __libc_malloc(10);
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_LIBC_MALLOC], [1], [Have malloc.h __libc_malloc() interface.])],
                 [AC_MSG_RESULT([no])])

  AC_MSG_CHECKING([for __libc_calloc()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [int *p=0;
                                   p = __libc_calloc(2, 3);
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_LIBC_CALLOC], [1], [Have malloc.h __libc_calloc() interface.])],
                 [AC_MSG_RESULT([no])])

  AC_MSG_CHECKING([for __libc_realloc()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [int *p=0;
                                   p = __libc_realloc(p,10);
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_LIBC_REALLOC], [1], [Have malloc.h __libc_realloc() interface.])],
                 [AC_MSG_RESULT([no])])

  AC_MSG_CHECKING([for __libc_reallocarray()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [int *p=0;
                                   p = __libc_reallocarray(p,10,10);
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_LIBC_REALLOCARRAY], [1], [Have malloc.h __libc_reallocarray() interface.])],
                 [AC_MSG_RESULT([no])])

  AC_MSG_CHECKING([for __libc_free()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [int *p=0;
                                   __libc_free(p);
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_LIBC_MALLOC], [1], [Have malloc.h __libc_malloc() interface.])],
                 [AC_MSG_RESULT([no])])

  AC_MSG_CHECKING([for reallocarray()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <stdlib.h>
                                  ],
                                  [int *p=0;
                                   p = reallocarray(p,10,sizeof(int));
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_REALLOCARRAY], [1], [Have stdlib.h reallocarray() interface.])],
                 [AC_MSG_RESULT([no])])


  # see if our linker supports the GNU/Linux "--wrap" option
  saveLDFLAGS="${LDFLAGS}"
  tryLDFLAGS="-Wl,--wrap,malloc -Wl,--wrap,free -Wl,--wrap,realloc -Wl,--wrap,reallocarray -Wl,--wrap,calloc"
  LDFLAGS_WRAP_MALLOC=""
  AC_MSG_CHECKING([if the linker supports ${tryLDFLAGS}])
  LDFLAGS=" ${tryLDFLAGS} ${LDFLAGS}"
  AC_LINK_IFELSE([AC_LANG_PROGRAM(
  [
#include <malloc.h>
#include <stdlib.h>

  void * __real_calloc       (size_t count, size_t size);
  void   __real_free         (void *ptr);
  void * __real_malloc       (size_t size);
  void * __real_realloc      (void *ptr, size_t size);
  void * __real_reallocarray (void *ptr, size_t count, size_t size);

  void * __wrap_malloc(size_t size)
  { return __real_malloc(size); }

  void * __wrap_calloc(size_t count, size_t size)
  { return __real_calloc(count,size); }

  void * __wrap_realloc(void *p, size_t size)
  { return __real_realloc(p,size); }

  void * __wrap_reallocarray(void *p, size_t count, size_t size)
  { return __real_reallocarray(p,count,size); }

  void __wrap_free(void *p)
  { __real_free(p); }
  ],
  [int *p=0;
   p = malloc(10);
   p = realloc(p,20);
   free(p);
   p = calloc(5,5);
   p = reallocarray(p,10,10);
   free(p);
  ])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([HAVE_WRAPPABLE_MALLOC], [1], [Linker supports e.g. -Wl,--wrap,malloc.])
   ld_wrap="yes"
   LDFLAGS_WRAP_MALLOC="${tryLDFLAGS}"
  ],
  [AC_MSG_RESULT([no])
   ld_wrap="no"
  ])

  AC_SUBST([LDFLAGS_WRAP_MALLOC])

  LDFLAGS="${saveLDFLAGS}"
dnl  AS_IF([test "$ac_cv_tls" != "none"],
dnl    [AC_DEFINE_UNQUOTED([TLS],[$ac_cv_tls],[If the compiler supports a TLS storage class, define it to that here])
dnl     m4_ifnblank([$1],[$1],[[:]])],
dnl    [m4_ifnblank([$2],[$2],[[:]])])
])
