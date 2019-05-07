set(autotype_win_SOURCES AutoTypeWindows.cpp)

add_library(keepassxc-autotype-windows MODULE ${autotype_win_SOURCES})
target_link_libraries(keepassxc-autotype-windows keepassxc_core)
install(TARGETS keepassxc-autotype-windows
        BUNDLE DESTINATION . COMPONENT Runtime
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR} COMPONENT Runtime)
