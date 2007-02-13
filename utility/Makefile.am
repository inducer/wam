# -----------------------------------------------------------------------------
# WAM utility makefile
# -----------------------------------------------------------------------------
# (c) Team *W*A*M* 1999
# -----------------------------------------------------------------------------

headers=\
  hash_map.hh \
  config.hh base.hh debug.hh exgame.hh basic_types.hh \
  string_pack.hh \
  stream.hh stream_types.hh  \
  codemanager.hh codemanager_impl.hh \
  manager.hh manager_impl.hh  \
  instancemanager.hh instancemanager_impl.hh \
  resourcemanager.hh resourcemanager_impl.hh \
  shapebitmap.hh \
  filemanager.hh \
  object.hh \
  png_info.hh

modules=exgame.cc basic_types.cc stream.cc \
  string_pack.cc \
  shapebitmap.cc filemanager.cc \
  object.cc

# Targets ---------------------------------------------------------------------
CXXFLAGS=@CXXFLAGS@ @IXLIB_CFLAGS@

lib_LTLIBRARIES=libwam-utility.la
libwam_utility_la_SOURCES=$(headers) $(modules)
