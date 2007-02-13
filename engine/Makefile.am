# -----------------------------------------------------------------------------
# WAM engine makefile
# -----------------------------------------------------------------------------
# (c) Team WAM 1999
# -----------------------------------------------------------------------------

headers=gameject.hh names.hh \
  animator.hh \
  codemanager.hh instancemanager.hh network.hh network_internal.hh \
  tick.hh collision.hh input.hh \
  message.hh drawable.hh resource.hh persistence.hh \
  console.hh commandmanager.hh \
  commands.hh audio_commands.hh \
  scrolling.hh \
  gamebase.hh game.hh \
  binding.hh

modules=gameject.cc \
  animator.cc \
  codemanager.cc instancemanager.cc network.cc \
  tick.cc collision.cc input.cc \
  message.cc drawable.cc resource.cc persistence.cc \
  console.cc commandmanager.cc \
  commands.cc audio_commands.cc \
  scrolling.cc \
  game.cc  \
  binding.cc

# Targets ---------------------------------------------------------------------
CXXFLAGS=@CXXFLAGS@ @SDL_CFLAGS@ @IXLIB_CFLAGS@

lib_LTLIBRARIES=libwam-engine.la
libwam_engine_la_SOURCES=$(headers) $(modules)
