include(FindDoxygen)
include(DoxygenTarget)

option(BUILD_DOCS "Build doxygen documentation when building all" OFF)
mark_as_advanced(BUILD_DOCS)

if(NOT DOXYGEN_DOT_FOUND) 
  message(WARNING "Graphviz doesn't seem to be installed. Doxygen will not be able to generate graphs. Consider installing this package.")
endif(NOT DOXYGEN_DOT_FOUND) 

if(DOXYGEN_FOUND)
  PrepareDocTarget()
endif(DOXYGEN_FOUND)
