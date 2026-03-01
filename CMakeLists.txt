cmake_minimum_required(VERSION 3.5)
project(KnowledgeCardGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt5 configuration
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/Card.cpp
    src/CardWidget.cpp
    src/CardTable.cpp
    src/CardManager.cpp
    src/CardEditDialog.cpp
    src/Deck.cpp
    src/DeckRepository.cpp
    src/DeckManager.cpp
    src/DeckSelectionDialog.cpp
)

# Header files
set(HEADERS
    src/MainWindow.h
    src/Card.h
    src/CardWidget.h
    src/CardTable.h
    src/CardManager.h
    src/CardEditDialog.h
    src/Deck.h
    src/DeckRepository.h
    src/DeckManager.h
    src/DeckSelectionDialog.h
)

# Executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Link Qt5 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
