# -----------------------------------------------------------------------------
# iXiON library extended compiler checks
# -----------------------------------------------------------------------------
# (c) iXiONmedia 1999
# -----------------------------------------------------------------------------




AC_DEFUN(IX_CXX_SUPPORTS_BOOL,[
  AC_MSG_CHECKING([whether c++ compiler ($CXX) supports bool])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE(,[ bool a = true; ],
    ix_cxx_supports_bool=yes,
    ix_cxx_supports_bool=no)
  AC_LANG_RESTORE
  if test "$ix_cxx_supports_bool" = "yes" ; then
    AC_MSG_RESULT([yup])
  else
    AC_MSG_RESULT([no])
    AC_DEFINE(bool,unsigned char,[Substitute 'bool' by 'unsigned char' if necessary])
    AC_DEFINE(true,1,[Substitute 'true' by '1' if necessary])
    AC_DEFINE(false,0,[Substitute 'false' by '0' if necessary])
  fi
])
