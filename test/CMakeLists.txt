cmake_minimum_required(VERSION 3.5)
project(CardGameTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Test 1: Simple Card test
add_executable(test_simple
    test_simple.cpp
    ../src/Card.cpp
)
target_link_libraries(test_simple Qt5::Core)

# Test 2: CardWidget test
add_executable(test_widget
    test_widget.cpp
    ../src/Card.cpp
    ../src/CardWidget.cpp
)
target_link_libraries(test_widget Qt5::Core Qt5::Widgets)

# Test 3: Card vector test
add_executable(test_card_vector
    test_card_vector.cpp
    ../src/Card.cpp
)
target_link_libraries(test_card_vector Qt5::Core)

# Test 4: Deck System Test Suite (New)
add_executable(test_deck_system
    test_deck_system.cpp
    ../src/Deck.cpp
    ../src/DeckRepository.cpp
    ../src/DeckManager.cpp
    ../src/Card.cpp
    ../src/CardManager.cpp
)
target_link_libraries(test_deck_system Qt5::Core Qt5::Widgets Qt5::Test)
