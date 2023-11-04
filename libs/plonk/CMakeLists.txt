project(plonk)


add_library(${PROJECT_NAME}
	context.cpp
	window.cpp
	renderer.cpp
)

target_link_libraries(${PROJECT_NAME} glfw vulkan X11)

add_library(libs::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
)

target_compile_features(${PROJECT_NAME}
	PRIVATE
		cxx_std_11
)


if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
