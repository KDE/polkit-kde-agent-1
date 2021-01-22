set(polkit-kde-agent-1_SRCS
    listener.cpp
    main.cpp
    identitymodel.cpp
    context.cpp

    data/data.qrc
)

add_executable(polkit-kde-authentication-agent-1 ${polkit-kde-agent-1_SRCS})

target_link_libraries(polkit-kde-authentication-agent-1
    PolkitQt5-1::Core
    PolkitQt5-1::Agent

    Qt5::Qml
    Qt5::Quick
    Qt5::Widgets

    KF5::CoreAddons
    KF5::Crash
    KF5::DBusAddons
    KF5::I18n
)

install(TARGETS polkit-kde-authentication-agent-1 DESTINATION ${KDE_INSTALL_LIBEXECDIR})
