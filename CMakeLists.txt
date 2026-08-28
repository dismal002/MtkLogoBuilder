cmake_minimum_required(VERSION 3.10)
project(LogoBuilderQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER "clang++")

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(LogoBuilderQt
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/logoproject.cpp
    src/logoproject.h
    src/sizedialog.cpp
    src/sizedialog.h
)

target_link_libraries(LogoBuilderQt PRIVATE Qt5::Widgets ZLIB::ZLIB)

configure_file(style.qss ${CMAKE_CURRENT_BINARY_DIR}/style.qss COPYONLY)
configure_file(resources.rcc ${CMAKE_CURRENT_BINARY_DIR}/resources.rcc COPYONLY)

# --- Compile Qt translations (.ts -> .qm) ---
find_program(LRELEASE_EXECUTABLE lrelease REQUIRED HINTS /usr/bin /usr/lib/qt5/bin)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lang)

set(TS_FILES
    lang/logobuilder_zh.ts
    lang/logobuilder_zh_tw.ts
    lang/logobuilder_ru.ts
    lang/logobuilder_ja.ts
)

foreach(TS_FILE ${TS_FILES})
    get_filename_component(QM_BASE ${TS_FILE} NAME_WE)
    set(QM_FILE ${CMAKE_CURRENT_BINARY_DIR}/lang/${QM_BASE}.qm)
    add_custom_command(
        OUTPUT ${QM_FILE}
        COMMAND ${LRELEASE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${TS_FILE} -qm ${QM_FILE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${TS_FILE}
        COMMENT "Compiling translation: ${TS_FILE}"
    )
    list(APPEND QM_FILES ${QM_FILE})
endforeach()

add_custom_target(translations ALL DEPENDS ${QM_FILES})
add_dependencies(LogoBuilderQt translations)

