# -----------------------------------------------------------------------------
# WAM output makefile
# -----------------------------------------------------------------------------
# (c) TEAM_WAM 2001
# -----------------------------------------------------------------------------

headers=image.hh
modules=image.cc

# Targets ---------------------------------------------------------------------
CXXFLAGS=@CXXFLAGS@ @IXLIB_CFLAGS@ @SDLUCID_CFLAGS@

lib_LTLIBRARIES=libwam-output.la
libwam_output_la_SOURCES=$(headers) $(modules)

bin_PROGRAMS=markpng

markpng_SOURCES=markpng.cc
markpng_LDADD=@SDLUCID_LIBS@
