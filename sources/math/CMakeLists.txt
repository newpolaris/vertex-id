project(test_math)

enable_testing()

file(GLOB HEADER_LIST *.h)
file(GLOB SOURCE_LIST *.cpp)
source_group("sources" FILES ${HEADER_LIST} ${SOURCE_LIST})

add_executable(test_math ${HEADER_LIST} ${SOURCE_LIST})
target_link_libraries(test_math)
