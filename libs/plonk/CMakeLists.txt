project(plonk)

add_library(${PROJECT_NAME}
	context.cpp
	window.cpp
	renderer.cpp
	frame.cpp
	camera.cpp
)

if (APPLE)
	if (NOT DEFINED ENV{VULKAN_SDK})
		message(FATAL_ERROR "You need to set the VULKAN_SDK environment variable")
	endif()

	#set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version" FORCE)
	#find_library(METAL_FRAMEWORK Metal)

	#link_directories(
	#	$ENV{VULKAN_SDK}/macOS/lib/
	#	$ENV{VULKAN_SDK}/MoltenVK/dylib/macOS
	#)

	target_link_libraries(${PROJECT_NAME}
		glfw 
		#$ENV{VULKAN_SDK}/macOS/lib/libvulkan.dylib
		$ENV{VULKAN_SDK}/macOS/lib/libMoltenVK.dylib
		#$ENV{VULKAN_SDK}/MoltenVK/dylib/macOS/libMoltenVK.dylib
		${METAL_FRAMEWORK}
	)
else()
	target_link_libraries(${PROJECT_NAME}
		glfw
		vulkan
		X11
	)
endif()


add_library(libs::${PROJECT_NAME} ALIAS ${PROJECT_NAME})


target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		if (APPLE)
			$ENV{VULKAN_SDK}/macOS/include/
		endif()
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
