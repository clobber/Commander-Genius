
OPTION(DBFUSION "Dosbox Fusion" No)

include_directories(${SDL_INCLUDE_DIR})

add_subdirectory(core)
add_subdirectory(keen)


if(DOWNLOADER)
    add_definitions(-DDOWNLOADER)
endif()


if(DBFUSION)
    add_subdirectory(dbfusion)
    add_definitions(-DDBFUSION)
endif()

add_subdirectory(refkeen)

set(EngineSources CGameLauncher.cpp CGameLauncher.h
                   downloadgui.cpp
                   gamedownloader.cpp gamedownloader.h
                   unzip/miniunz.c unzip/ioapi.c
                   unzip/unzip.c)

IF(WIN32)
set(EngineSources ${EngineSources} unzip/iowin32.c)
ENDIF(WIN32)

add_library(engine OBJECT ${EngineSources})

set_property(GLOBAL APPEND PROPERTY CG_OBJ_LIBS $<TARGET_OBJECTS:engine>)
