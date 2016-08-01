project(kit)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
#set(QT_MIN_VERSION "5.3.0")
#set(KF5_MIN_VERSION "5.2.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)


# Find Qt modules
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
)

# Find KDE modules
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    WidgetsAddons
    XmlGui
    KIO
    Parts
)

# run moc automatically
set(CMAKE_AUTOMOC ON)

# enable C++11
add_definitions("-std=c++11")

# find qt headers in same build dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# show all warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
  cmake_policy(SET CMP0063 NEW)
endif()

add_executable(main_EXE
  src/main.cpp
  src/mainwindow.cpp
  src/widget.cpp
  src/terminal.cpp)

# find qt libraries for linking
target_link_libraries(main_EXE
    #Qt5::Core
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    KF5::XmlGui
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KIOFileWidgets
    KF5::Parts
)

install(TARGETS main_EXE ${INSTALL_TARGETS_DEFAULT_ARGS})
