SUBDIRS = dtd

EXTRA_DIST = \
	agdb.html getting.html java_wrapper.html \
	changes.html index.html python_wrapper.html \
	components.html install.html tcl_wrapper.html \
	formats.html install_bin.html \
	archi.gif layout.gif \
	cag.txt

dist-hook:
	tar cf - api --exclude=CVS | (cd $(distdir); tar xf -)

