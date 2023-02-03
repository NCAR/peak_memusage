
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

  AC_MSG_CHECKING([for __libc_free()])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <malloc.h>
                                  ],
                                  [int *p=0;
                                   __libc_free(p);
                                  ])],
                 [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_LIBC_MALLOC], [1], [Have malloc.h __libc_malloc() interface.])],
                 [AC_MSG_RESULT([no])])

dnl  AS_IF([test "$ac_cv_tls" != "none"],
dnl    [AC_DEFINE_UNQUOTED([TLS],[$ac_cv_tls],[If the compiler supports a TLS storage class, define it to that here])
dnl     m4_ifnblank([$1],[$1],[[:]])],
dnl    [m4_ifnblank([$2],[$2],[[:]])])
])
