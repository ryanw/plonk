project(plonk_tests)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        libs::plonk
)

add_test(plonk_tests ${PROJECT_NAME})
