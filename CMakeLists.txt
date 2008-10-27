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

kde4_add_ui_files(policykit_SRCS authdialogui.ui)

kde4_add_executable(policykit-kde ${policykit_SRCS})
target_link_libraries(policykit-kde ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY} polkit-dbus)

install(TARGETS policykit-kde  DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
