project(outpost CXX)
cmake_minimum_required(VERSION 3.5)

find_package (Qt5 COMPONENTS Core Network Qml Gui Quick REQUIRED)

include(FindPkgConfig)
pkg_search_module(SAILFISH sailfishapp REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(cpr)
add_subdirectory(json)

SET(QZXING_USE_QML ON)
set(QZXING_USE_ENCODER ON)
add_subdirectory(qzxing/src)

FILE(GLOB SRC "src/*.cpp" "src/*.h")
add_executable(outpost
    ${SRC}
    qml/resources/resources.qrc
)
target_compile_definitions(outpost PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)
target_include_directories(outpost PRIVATE
    $<BUILD_INTERFACE:
    ${SAILFISH_INCLUDE_DIRS}
>)
target_link_libraries(outpost
    PUBLIC
    Qt5::Quick
    ${SAILFISH_LDFLAGS}
    qzxing
    PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
)

install(TARGETS outpost
    RUNTIME DESTINATION bin
)
install(DIRECTORY qml
    DESTINATION share/outpost
)
install(DIRECTORY translations
    DESTINATION share/outpost
    FILES_MATCHING PATTERN "*.qm"
)
install(FILES outpost.desktop
    DESTINATION share/applications
)
install(FILES icons/86x86/outpost.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/outpost.png
    DESTINATION share/icons/hicolor/108x108/apps
)
install(FILES icons/128x128/outpost.png
    DESTINATION share/icons/hicolor/128x128/apps
)
install(FILES icons/172x172/outpost.png
    DESTINATION share/icons/hicolor/172x172/apps
)

# Get the other files reachable from the project tree in Qt Creator
FILE(GLOB TsFiles "translations/*.ts")
add_custom_target(distfiles
    SOURCES
        outpost.desktop
        qml/outpost.qml
        qml/cover/CoverPage.qml
        qml/pages/Main.qml
        qml/pages/About.qml
        qml/pages/PhoneNumberDialog.qml
        qml/pages/AuthorizationCodeDialog.qml
        rpm/outpost.spec
        ${TsFiles})

# Tell Qt Creator where the application executable(s) would be located on the
# device.
#
# It is not necessary to list other deployables than executables (runtime
# targets) here. The deployment process of Sailfish OS projects is opaque to
# Qt Creator and the information contained in QtCreatorDeployment.txt is only
# used to locate the executable associated with the active run configuration
# on the device in order to run it.
#
# Search the Qt Creator Manual to learn about the QtCreatorDeployment.txt file
# format.
file(WRITE "${CMAKE_BINARY_DIR}/QtCreatorDeployment.txt"
    "${CMAKE_INSTALL_PREFIX}
${CMAKE_BINARY_DIR}/outpost:bin
")

