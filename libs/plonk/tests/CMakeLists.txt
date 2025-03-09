project(plonk_tests)

set(TestsToRun 
	math/vectors.cpp
	math/matrices.cpp
	resource_id.cpp
)
create_test_sourcelist(TestFiles TestSuite.cpp ${TestsToRun})

add_executable(TestSuite ${TestFiles})
target_link_libraries(TestSuite
	PRIVATE
		libs::plonk
)

foreach (TestFilename ${TestsToRun})
	string(REPLACE ".cpp" "" TestName ${TestFilename})
	add_test(NAME ${TestName} COMMAND TestSuite ${TestName})
endforeach()
