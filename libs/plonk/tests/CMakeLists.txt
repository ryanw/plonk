project(plonk_tests)

create_test_sourcelist(TestFiles
	TestSuite.cpp
	math/vectors.cpp
	math/matrices.cpp
)

add_executable(TestSuite ${TestFiles})
target_link_libraries(TestSuite
	PRIVATE
		libs::plonk
)
set(TestsToRun ${TestFiles})
remove(TestsToRun TestSuite.cpp)

foreach (TestFilename ${TestsToRun})
	get_filename_component(DirName ${TestFilename} DIRECTORY)
	get_filename_component(TName ${TestFilename} NAME_WE)
	add_test(NAME ${TName} COMMAND TestSuite "${DirName}/${TName}")
endforeach()
