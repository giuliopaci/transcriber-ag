SUBDIRS = src scripts demo doc

EXTRA_DIST = LICENSE
dist-hook:
	tar cf - projects/win32 --exclude=CVS --exclude=*.gif | (cd $(distdir); tar xf -)

