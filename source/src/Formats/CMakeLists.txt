# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Subdirectories ---
add_subdirectory	(CHAT)
add_subdirectory	(CTM)
add_subdirectory	(LBL)
add_subdirectory	(STM)
add_subdirectory	(MDTM)
add_subdirectory	(HTML)
add_subdirectory	(TransAG)
add_subdirectory	(TransAG_compat)
add_subdirectory	(TRS)
add_subdirectory	(VRXML)
add_subdirectory	(TXT)
add_subdirectory	(SGML)
#if (NOT LIBTAG)
#   add_subdirectory	(SGML)
#    add_subdirectory	(VSFT)
#endif(NOT LIBTAG)
