# -----------------------------------------------------------------------------
# WAM gamejects makefile
# -----------------------------------------------------------------------------
# (c) Team WAM 2001
# -----------------------------------------------------------------------------

headers=cron.hh \
  script_notifier.hh \
  mouse_pointer.hh \
  mouse_scroller.hh \
  pause_monitor.hh \
  parallax.hh \
  graphic_console.hh \
  status.hh \
  menu.hh

modules=cron.cc \
  script_notifier.cc \
  mouse_pointer.cc \
  mouse_scroller.cc \
  pause_monitor.cc \
  parallax.cc \
  graphic_console.cc \
  status.cc \
  menu.cc


# Targets ---------------------------------------------------------------------
CXXFLAGS=@CXXFLAGS@ @SDL_CFLAGS@ @IXLIB_CFLAGS@

lib_LTLIBRARIES=libwam-gamejects.la
libwam_gamejects_la_SOURCES=$(headers) $(modules)