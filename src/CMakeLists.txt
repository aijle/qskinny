add_library(qskinny STATIC)

target_link_libraries(qskinny Qt5::Widgets Qt5::Quick)

set (WILDCARD_SOURCE *.cpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${WILDCARD_SOURCE})

target_sources(qskinny PUBLIC  
    ${SOURCES}
)

target_include_directories(qskinny PUBLIC common controls dialogs graphic inputpanel layouts nodes )
