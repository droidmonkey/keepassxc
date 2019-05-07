if(WITH_XC_SSHAGENT)
    register_plugin(sshagent)

    set(sshagent_SOURCES
        AgentSettingsPage.cpp
        AgentSettingsWidget.cpp
        KeeAgentSettings.cpp
        SSHAgent.cpp)

    add_library(sshagent STATIC ${sshagent_SOURCES})
    target_link_libraries(sshagent Qt5::Core Qt5::Widgets Qt5::Network ${GCRYPT_LIBRARIES} ${crypto_ssh_LIB})

    set(SSH_AGENT)
endif()
