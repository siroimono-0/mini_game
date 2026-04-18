cmake_minimum_required(VERSION 3.16)

project(Stream VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appStream
    main.cpp
)

qt_add_qml_module(appStream
    URI Stream
    VERSION 1.0
    QML_FILES
        Main.qml

    SOURCES
        Cpp_Module/Map_md.h Cpp_Module/Map_md.cpp
        Cpp_Module/Cpp_Module.h Cpp_Module/Cpp_Module.cpp
        Cpp_Module/WK_loop.h Cpp_Module/WK_loop.cpp

    RESOURCES
        ./img/block_orange.png
        ./img/box.png
        ./img/forest.png
        ./img/home.png
        ./img/tree.png
        ./img/ground_green.png
        ./img/ground_gray.png
        ./img/transparent.png
        ./img/ch_1.png
        ./img/ch_2.png
        ./img/boom_1.png
        ./img/boom_2.png
        ./img/needle.png
        ./img/car.png
        ./img/power.png
        ./img/boom_plus.png
        ./img/ch_1.png
        ./img/ch_1_0.png
        ./img/ch_2.png
        ./img/ch_2_0.png
        ./img/RIP.png
        ./img/ch_1_0_0.png
        ./img/ch_2_0_0.png
        ./img/gost.png
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appStream PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appStream
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appStream
    PRIVATE Qt6::Quick
)
target_link_libraries(appStream PRIVATE Qt6::Core)
target_link_libraries(appStream PRIVATE Qt6::Core)
target_link_libraries(appStream PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(TARGETS appStream
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
