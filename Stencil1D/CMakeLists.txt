set(CMAKE_CXX_COMPILER "/usr/bin/mpicxx")
set(UPCXX_DIR /home/blankpower/opt/upcxx-2021.9.0/share/cmake/UPCXX)
find_package( UPCXX 2019.9.1 REQUIRED )

add_executable(rb1d rb1d.cpp)
target_link_libraries(rb1d PUBLIC UPCXX::upcxx)

add_executable(rb1d-darts utils.h rb1d-darts.h rb1d-darts.cpp)
target_link_libraries(rb1d-darts darts)

add_executable(rb1d-raw rb1d-raw.cpp)
target_link_libraries(rb1d-raw PUBLIC UPCXX::upcxx)
