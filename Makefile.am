# -----------------------------------------------------------------------------
# WAM root makefile
# -----------------------------------------------------------------------------
# (c) Team WAM 1999
# -----------------------------------------------------------------------------

SUBDIRS=utility output engine gamejects game main 

data:
	tar cvfz wam-data-@VERSION@.tar.gz --exclude=CVS \
	  base/fonts \
	  base/images \
	  base/music \
	  base/shapes \
	  base/sounds
	tar cvfz wam-creation-@VERSION@.tar.gz --exclude=CVS base/creation

rundata:
	tar cvfz wam-rundata-@VERSION@.tar.gz --exclude=CVS \
	  base/fonts \
	  base/images \
	  base/music \
	  base/shapes \
	  base/sounds \
	  base/scripts \
	  base/data

publish:
	rsync -e ssh --verbose --progress wam-data-@VERSION@.tar.gz wam-creation-@VERSION@.tar.gz inducer@shell.sf.net:/home/groups/w/w-/w-a-m/htdocs/

tags:
	ctags -R --c++-types=+d+p
