project(plonk_tests)

set(TestsToRun 
	math/vectors.cpp
	math/matrices.cpp
)
create_test_sourcelist(TestFiles TestSuite.cpp ${TestsToRun})

add_executable(TestSuite ${TestFiles})
target_link_libraries(TestSuite
	PRIVATE
		libs::plonk
)

foreach (TestFilename ${TestsToRun})
	get_filename_component(DirName ${TestFilename} DIRECTORY)
	get_filename_component(TName ${TestFilename} NAME_WE)
	add_test(NAME ${TName} COMMAND TestSuite "${DirName}/${TName}")
endforeach()
