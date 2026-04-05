cmake_minimum_required(VERSION 3.5)
project(KnowledgeCardGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt5 configuration
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Svg)

# Check for LaTeX rendering tools (optional, for formula support)
find_program(LATEX_EXECUTABLE latex)
find_program(DVISVGM_EXECUTABLE dvisvgm)
find_program(DVIPNG_EXECUTABLE dvipng)
find_program(PDFLATEX_EXECUTABLE pdflatex)
find_program(CONVERT_EXECUTABLE convert)

if(LATEX_EXECUTABLE AND DVISVGM_EXECUTABLE)
    message(STATUS "LaTeX rendering: latex + dvisvgm (SVG) pipeline available")
    add_definitions(-DLATEX_AVAILABLE)
elseif(LATEX_EXECUTABLE AND DVIPNG_EXECUTABLE)
    message(STATUS "LaTeX rendering: latex + dvipng pipeline available")
    add_definitions(-DLATEX_AVAILABLE)
elseif(PDFLATEX_EXECUTABLE AND CONVERT_EXECUTABLE)
    message(STATUS "LaTeX rendering: pdflatex + convert pipeline available")
    add_definitions(-DLATEX_AVAILABLE)
else()
    message(WARNING "LaTeX rendering tools not found. Install with:\n"
                    "  sudo apt-get install texlive-latex-recommended texlive-latex-extra dvipng\n"
                    "LaTeX formula rendering will be disabled (app will still build and run).")
endif()

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
    src/LatexRenderer.cpp
    src/LatexParser.cpp
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
    src/LatexRenderer.h
    src/LatexParser.h
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
    Qt5::Svg
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
