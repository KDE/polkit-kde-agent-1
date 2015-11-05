project(polkit-kde-agent-1)
set(PROJECT_VERSION "5.4.3")

# minimal requirements
cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.4.0")

# we need some parts of the ECM CMake helpers
find_package (ECM 1.2.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMSetupVersion)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Widgets)

# Load the frameworks we need
find_package(KF5 REQUIRED COMPONENTS
  I18n
  WindowSystem
  DBusAddons
  WidgetsAddons
  CoreAddons
  Crash
  Config
  IconThemes
  Notifications
)

# create config file
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# let our config.h be found first in any case
include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})

find_package(PolkitQt5-1 REQUIRED 0.103.0)

include_directories(
                    ${POLKITQT-1_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

qt5_add_dbus_adaptor(policykit_SRCS org.kde.Polkit1AuthAgent.xml policykitlistener.h PolicyKitListener)

ki18n_wrap_ui(policykit_SRCS AuthDialog.ui authdetails.ui)

set(policykit_SRCS
    ${policykit_SRCS}
    policykitkde.cpp
    policykitlistener.cpp
    main.cpp
    AuthDialog.cpp
)

add_executable(polkit-kde-authentication-agent-1 ${policykit_SRCS})

target_link_libraries(polkit-kde-authentication-agent-1
    KF5::DBusAddons
    KF5::WindowSystem
    KF5::WidgetsAddons
    KF5::CoreAddons
    KF5::I18n
    KF5::Crash
    KF5::ConfigCore
    KF5::IconThemes
    KF5::Notifications
    ${POLKITQT-1_LIBRARIES}
)

configure_file(polkit-kde-authentication-agent-1.desktop.cmake ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop)

install(TARGETS polkit-kde-authentication-agent-1 DESTINATION ${LIBEXEC_INSTALL_DIR})

install(FILES ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

install(FILES policykit1-kde.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
