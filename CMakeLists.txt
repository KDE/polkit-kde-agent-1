find_package(KDE4 REQUIRED)

include_directories(
    ${KDE4_INCLUDES}
    ${POLKIT_INCLUDE_DIRS}
)

set(policykit_SRCS
    authdialog.cpp
    policykitkde.cpp
    processwatcher.cpp
    main.cpp
)

qt4_add_dbus_adaptor(policykit_SRCS org.freedesktop.PolicyKit.AuthenticationAgent.xml
   policykitkde.h PolicyKitKDE)

kde4_add_ui_files(policykit_SRCS authdialogui.ui authdetails.ui)

kde4_add_executable(polkit-kde-manager ${policykit_SRCS})
target_link_libraries(polkit-kde-manager ${KDE4_KDEUI_LIBS} polkit-dbus polkit-grant)

install(TARGETS polkit-kde-manager DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
