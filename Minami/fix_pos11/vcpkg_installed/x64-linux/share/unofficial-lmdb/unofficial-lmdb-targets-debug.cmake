#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::lmdb::lmdb" for configuration "Debug"
set_property(TARGET unofficial::lmdb::lmdb APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::lmdb::lmdb PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/liblmdb.a"
  )

list(APPEND _cmake_import_check_targets unofficial::lmdb::lmdb )
list(APPEND _cmake_import_check_files_for_unofficial::lmdb::lmdb "${_IMPORT_PREFIX}/debug/lib/liblmdb.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
