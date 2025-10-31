cmake_minimum_required(VERSION 3.12)

project(fatmanager)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)

set(PROJECT_HEADERS
        include/basepanel.h
        include/fatcache.h
        include/fatimage.h
        include/fatmodel.h
        include/fatparser.h
        include/fatreader.h
        include/fatstruct.h
        include/fileoperations.h
        include/keyboardhandler.h
        include/leftpanel.h
        include/mainwindow.h
        include/rightpanel.h
)

set(PROJECT_SOURCES
        src/basepanel.cpp
        src/fatcache.cpp
        src/fatimage.cpp
        src/fatmodel.cpp
        src/fatparser.cpp
        src/fatreader.cpp
        src/fileoperations.cpp
        src/keyboardhandler.cpp
        src/leftpanel.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/rightpanel.cpp
)

add_executable(fatmanager
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
)

target_include_directories(fatmanager PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(fatmanager PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
)