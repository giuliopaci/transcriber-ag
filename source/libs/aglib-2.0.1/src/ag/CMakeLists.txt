# -----------------------------
# --- AGLib (BT Patch) - AG ---
# -----------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
	${SRC}/ag/ag
)

# --- Libraries ---
add_library(
	ag
	SHARED

	# --- Sources ---
	AG.cc
	AGAPI.cc
	AGException.cc
	AGSet.cc
	Anchor.cc
	Annotation.cc
	AnnotationIndex.cc
	FeatureMap.cc
	Identifiers.cc
	Metadata.cc
	Paired.cc
	RE.cc
	Record.cc
	Signal.cc
	Timeline.cc
	Utilities.cc
	Validation.cc
	agfio.cc
	agfioError.cc
	agfio_plugin.cc
	tree_kernel.cc
	tree_others.cc
	#ag_dlfcn.c
	regcomp.c
	regerror.c
	regexec.c
	regfree.c
)



target_link_libraries(
	ag
	
	dl
)

install(
	TARGETS ag

	RUNTIME	DESTINATION bin
	LIBRARY	DESTINATION lib
)