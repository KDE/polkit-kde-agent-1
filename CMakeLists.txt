project(polkit-kde-agent-1)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package(KDE4 REQUIRED)

set(POLKITQT-1_MIN_VERSION "0.99.0")
find_package(PolkitQt-1 REQUIRED)

include_directories(${KDE4_INCLUDES}
                    ${POLKITQT-1_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

set(policykit_SRCS
    policykitkde.cpp
    policykitlistener.cpp
    main.cpp
    AuthDialog.cpp
)

qt4_add_dbus_adaptor(policykit_SRCS org.kde.Polkit1AuthAgent.xml policykitlistener.h PolicyKitListener)

kde4_add_ui_files(policykit_SRCS AuthDialog.ui authdetails.ui)

kde4_add_executable(polkit-kde-authentication-agent-1 ${policykit_SRCS})

target_link_libraries(polkit-kde-authentication-agent-1
                                         ${KDE4_KDEUI_LIBS}
                                         ${POLKITQT-1_LIBRARIES}
)

configure_file(polkit-kde-authentication-agent-1.desktop.in ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop)

install(TARGETS polkit-kde-authentication-agent-1 DESTINATION ${LIBEXEC_INSTALL_DIR})

install(FILES ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop DESTINATION ${KDE4_AUTOSTART_INSTALL_DIR})

install(FILES policykit1-kde.notifyrc DESTINATION ${DATA_INSTALL_DIR}/policykit1-kde)
