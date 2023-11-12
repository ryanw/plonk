project(plonk_tests)

create_test_sourcelist(TestFiles
	TestSuite.cpp
	math.cpp
)

add_executable(TestSuite ${TestFiles})
target_link_libraries(TestSuite
	PRIVATE
		libs::plonk
)
set(TestsToRun ${TestFiles})
remove(TestsToRun TestSuite.cpp)

foreach (test ${TestsToRun})
	get_filename_component(TName ${test} NAME_WE)
	add_test(NAME ${TName} COMMAND TestSuite ${TName})
endforeach()
