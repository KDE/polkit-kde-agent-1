find_package(KDE4 REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${POLKIT_INCLUDE_DIRS}
    ${POLKITDBUS_INCLUDE_DIRS}
)

set(policykit_SRCS
    AuthDialog.cpp
    policykitkde.cpp
    processwatcher.cpp
    main.cpp
)

qt4_add_dbus_adaptor(policykit_SRCS org.freedesktop.PolicyKit.AuthenticationAgent.xml
   policykitkde.h PolicyKitKDE)

kde4_add_ui_files(policykit_SRCS AuthDialog.ui authdetails.ui)

kde4_add_executable(polkit-kde-manager ${policykit_SRCS})

target_link_libraries(polkit-kde-manager ${KDE4_KDEUI_LIBS}
${POLKITDBUS_LIBRARIES} ${POLKITGRANT_LIBRARIES} ${POLKIT_LIBRARIES})

install(TARGETS polkit-kde-manager DESTINATION ${LIBEXEC_INSTALL_DIR})
