set(CMAKE_CXX_COMPILER "/usr/bin/mpicxx")
set(UPCXX_DIR /home/blankpower/opt/upcxx-2021.9.0/share/cmake/UPCXX)
find_package( UPCXX 2019.9.1 REQUIRED )

add_executable (hello hello.cpp)
target_link_libraries(hello PUBLIC UPCXX::upcxx)

add_executable(hello_darts hello_darts.cpp)
target_link_libraries(hello_darts darts)