# -----------------------------------------------------------------------------
# WAM game makefile
# -----------------------------------------------------------------------------
# (c) Team WAM 1999
# -----------------------------------------------------------------------------

CXXFLAGS=@CXXFLAGS@ @SDLUCID_CFLAGS@ @SDL_CFLAGS@ @IXLIB_CFLAGS@

if HAVE_DYNAMIC_LINKING
LINKED_GAME_LIBS=
else
LINKED_GAME_LIBS=../game/libwam.la
endif

bin_PROGRAMS=wam-binary
wam_binary_SOURCES=wam.cc
wam_binary_LDFLAGS=-export-dynamic 
wam_binary_LDADD=\
  $(LINKED_GAME_LIBS) \
  ../gamejects/libwam-gamejects.la \
  ../engine/libwam-engine.la \
  ../output/libwam-output.la \
  ../utility/libwam-utility.la \
  @SDLUCID_LIBS@ @IXLIB_LIBS@ @PLATFORM_LIBS@
