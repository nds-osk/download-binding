PKG_CHECK_MODULES(afb afb-daemon)
PKG_CHECK_MODULES(glib REQUIRED glib-2.0)
IF(afb_FOUND)
    MESSAGE(STATUS "Creation download-api for AFB-DAEMON")
    EXECUTE_PROCESS(
            COMMAND pkg-config --variable binding_install_dir afb-daemon
            OUTPUT_VARIABLE binding_dir OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    ADD_LIBRARY(download-api MODULE
                download-api.c
                downloader.c
                validation.c
                )
    INCLUDE_DIRECTORIES(${glib_STATIC_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(download-api "-lcurl -lpthread -lglib-2.0")
    SET_TARGET_PROPERTIES(download-api PROPERTIES
        PREFIX ""
        LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export.map"
    )
    INSTALL(TARGETS download-api LIBRARY DESTINATION ${binding_dir})
ELSE()
    MESSAGE(STATUS "Not creating the binding for AFB-DAEMON")
ENDIF()
