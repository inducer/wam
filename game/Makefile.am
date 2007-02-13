# -----------------------------------------------------------------------------
# WAM game library makefile
# -----------------------------------------------------------------------------
# (c) Team WAM 1999
# -----------------------------------------------------------------------------

all-local:
	cp -df .libs/lib*.so* $(top_srcdir)/base/code &> /dev/null || true

headers=game_names.hh \
  environment.hh physics.hh \
  round.hh \
  hero.hh weapon.hh \
  background.hh credits.hh \
  fx.hh \
  main.hh \
  supervisor.hh

modules=physics.cc environment.cc \
  round.cc \
  hero.cc weapon.cc \
  background.cc credits.cc \
  fx.cc \
  main.cc \
  supervisor.cc 

# Targets ---------------------------------------------------------------------
CXXFLAGS=@CXXFLAGS@ @IXLIB_CFLAGS@ @SDL_CFLAGS@ 

noinst_LTLIBRARIES=libwam.la

libwam_la_SOURCES=$(headers) $(modules)
libwam_la_LDFLAGS=-rpath @libdir@ -module

