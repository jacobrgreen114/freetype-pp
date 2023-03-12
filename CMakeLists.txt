
add_library(freetype-pp
        src/freetype.cpp
)

target_include_directories(freetype-pp PUBLIC inc)

target_link_libraries(freetype-pp PUBLIC freetype)

install(TARGETS freetype-pp)
