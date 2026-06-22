cmake_minimum_required(VERSION 3.16)

project(Dispatcher VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поддержка как Qt6, так и Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Charts  Quick QuickWidgets QuickControls2 Location Positioning)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Charts  Quick QuickWidgets QuickControls2 Location Positioning)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    uiconnect.h
    uiconnect.cpp
    uiconnect.ui
    database.cpp
    database.h
    simulatorstart.h
    simulatorstart.cpp
    qmlMap.qrc
)

if(WIN32)
    set(WIN_ICON Images/app_icon.rc)
endif()
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Dispatcher
        MANUAL_FINALIZATION
        ${WIN_ICON}
        ${PROJECT_SOURCES}
        markermanager.h markermanager.cpp
        norounddelegate.h norounddelegate.cpp
        datastorage.h datastorage.cpp
        dispatcherdate.h dispatcherdate.cpp dispatcherdate.ui

    )
else()
    add_executable(Dispatcher
        ${PROJECT_SOURCES}
    )
endif()
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/PG_Libs)
target_link_libraries(Dispatcher PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Location Qt${QT_VERSION_MAJOR}::QuickWidgets Qt${QT_VERSION_MAJOR}::Quick  Qt${QT_VERSION_MAJOR}::QuickControls2 Qt${QT_VERSION_MAJOR}::Positioning
    libcrypto-3-x64
    libiconv-2
    libintl-9
    libpq
    libssl-3-x64)

set_target_properties(Dispatcher PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Dispatcher
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_finalize_executable(Dispatcher)
endif()
