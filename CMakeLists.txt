cmake_minimum_required(VERSION 3.20)

# This template attempts to be "fetch_content"-able
# so that it works well with tools like CPM or other
# manual dependency management

# Set the project name and language
project(
  Alive
  VERSION 0.0.1
  DESCRIPTION ""
  HOMEPAGE_URL "https://github.com/sanjuchopracool/alive.git"
  LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick QuickControls2)

add_subdirectory(alive_core)
add_subdirectory(alive_editor)
add_subdirectory(src)
