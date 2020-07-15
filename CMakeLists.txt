cmake_minimum_required(VERSION 3.10)

project(Crescendo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# sources
file(GLOB sources
	beat_block.cpp
	bitmap_text.cpp
	game.cpp
	infiltrator.cpp
	level_state.cpp
	lever.cpp
	main.cpp
	menu_state.cpp
	orb.cpp
	state.cpp
)


#-------------------------------------------------------------------------------
# Dependencies
#-------------------------------------------------------------------------------
include(FindPkgConfig)
# SFML
PKG_SEARCH_MODULE(SFML REQUIRED sfml-all)

include_directories(${SFML_INCLUDE_DIRS})

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(Crescendo ${sources})
target_link_libraries(Crescendo ${SFML_LIBRARIES})

#-------------------------------------------------------------------------------
# Install commands
#-------------------------------------------------------------------------------
install(TARGETS   Crescendo RUNTIME DESTINATION bin)
install(DIRECTORY img/ DESTINATION share/Crescendo/img)
install(DIRECTORY snd/ DESTINATION share/Crescendo/snd)
install(DIRECTORY mus/ DESTINATION share/Crescendo/mus)