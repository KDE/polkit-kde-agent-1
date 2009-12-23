set(policykit_SRCS
    policykitkde.cpp
    policykitlistener.cpp
    main.cpp
    AuthDialog.cpp
)

kde4_add_ui_files(policykit_SRCS AuthDialog.ui authdetails.ui)

kde4_add_executable(polkit-kde-authentication-agent-1 ${policykit_SRCS})

target_link_libraries(polkit-kde-authentication-agent-1
                                         ${KDE4_KDEUI_LIBS}
                                         ${POLKITQT-1_AGENT_LIBRARY}
)

configure_file(polkit-kde-authentication-agent-1.desktop.in ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop)

install(TARGETS polkit-kde-authentication-agent-1 DESTINATION ${LIBEXEC_INSTALL_DIR})

# Install .desktop file to the XDG_CONFIG_DIRS if exists, to the /etc/xdg/autostart otherwise
set (DESKTOP_INSTALL_DIR $ENV{XDG_CONFIG_DIRS})
if (DESKTOP_INSTALL_DIR)
else (DESKTOP_INSTALL_DIR)
  set (DESKTOP_INSTALL_DIR /etc/xdg/autostart)
endif (DESKTOP_INSTALL_DIR)

install(FILES ${CMAKE_BINARY_DIR}/polkit-kde-authentication-agent-1.desktop DESTINATION ${DESKTOP_INSTALL_DIR})
