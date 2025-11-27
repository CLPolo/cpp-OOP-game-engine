add_library(Engine SHARED GameEngine.cpp GameEngine.h GameObject.cpp GameObject.h DrawContext.cpp DrawContext.h CollisionObject.cpp CollisionObject.h EngineView.h MathUtil.h NotificationManager.cpp NotificationManager.h SpatialGrid.cpp SpatialGrid.h )

# TODO: Set your required components here... (also see below at target_link_libraries)
set(SFML_COMPONENTS Graphics)

# Clear any existing CMAKE_PREFIX_PATH to avoid system SFML
set(CMAKE_PREFIX_PATH "")

# Set CMAKE_PREFIX_PATH to only include extern SFML
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern/SFML-3.0.0")

message(STATUS "Using extern SFML from: ${CMAKE_SOURCE_DIR}/extern/SFML-3.0.0")
find_package(SFML 3 REQUIRED COMPONENTS ${SFML_COMPONENTS})

target_include_directories(Engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Engine PUBLIC /opt/homebrew/include/)
target_compile_features(Engine PRIVATE cxx_std_20)
# TODO: Also update here
target_link_libraries(Engine PUBLIC SFML::Graphics)