cmake_minimum_required(VERSION 3.16)

project(polkit-kde-agent-1)
set(PROJECT_VERSION "5.23.80")

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.86")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

# we need some parts of the ECM CMake helpers
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(FeatureSummary)
include(ECMConfiguredInstall)

#
# dependencies
#

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Widgets Qml Quick QuickControls2)

# Load the frameworks we need
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
  I18n
  DBusAddons
  CoreAddons
  Crash
)

find_package(PolkitQt5-1 REQUIRED 0.103.0)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050c00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054200)

#
# the source itself
#

add_subdirectory(src)

#
# configure desktop file
#

configure_file(polkit-kde-authentication-agent-1.desktop.cmake ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop)

#
# install files
#

install(FILES ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})
install(FILES ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop DESTINATION ${KDE_INSTALL_APPDIR} RENAME org.kde.polkit-kde-authentication-agent-1.desktop)
install(FILES ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} RENAME org.kde.polkit-kde-authentication-agent-1.desktop)

ecm_install_configured_files(INPUT plasma-polkit-agent.service.in DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
install(FILES policykit1-kde.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})

#
# clang-format
#

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
