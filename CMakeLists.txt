cmake_minimum_required(VERSION 3.24)
project(ImgSorting CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 3.2 REQUIRED CONFIG)

add_library(sorts STATIC
    src/sorts.cpp
    src/sorts/bubble_sort.cpp
    src/sorts/cocktail_shaker_sort.cpp
    src/sorts/odd_even_sort.cpp
    src/sorts/gnome_sort.cpp
    src/sorts/comb_sort.cpp
    src/sorts/selection_sort.cpp
    src/sorts/cycle_sort.cpp
    src/sorts/insertion_sort.cpp
    src/sorts/pancake_sort.cpp
    src/sorts/merge_sort.cpp
    src/sorts/block_merge_sort.cpp
    src/sorts/in_place_merge_sort.cpp
    src/sorts/tim_sort.cpp
    src/sorts/patience_sort.cpp
    src/sorts/strand_sort.cpp
    src/sorts/quick_sort.cpp
    src/sorts/pdq_sort.cpp
    src/sorts/tournament_sort.cpp
    src/sorts/tree_sort.cpp
    src/sorts/heap_sort.cpp
    src/sorts/intro_sort.cpp
    src/sorts/smooth_sort.cpp
    src/sorts/shell_sort.cpp
    src/sorts/counting_sort.cpp
    src/sorts/pigeonhole_sort.cpp
    src/sorts/bucket_sort.cpp
    src/sorts/flash_sort.cpp
    src/sorts/msd_radix_sort.cpp
    src/sorts/binary_msd_radix_sort.cpp
    src/sorts/radix_sort.cpp
    src/sorts/gravity_sort.cpp
    src/sorts/bitonic_sort.cpp
    src/sorts/stooge_sort.cpp
    src/sorts/slow_sort.cpp
    src/sorts/stalin_sort.cpp
    src/sorts/bozo_sort.cpp
    src/sorts/bogobogo_sort.cpp
    src/sorts/bogo_sort.cpp
)
target_include_directories(sorts PUBLIC src)

add_executable(ImgSorting src/main.cpp third_party/stb_image_impl.cpp)
target_include_directories(ImgSorting PRIVATE third_party)
target_link_libraries(ImgSorting PRIVATE sorts SDL3::SDL3)
target_compile_definitions(ImgSorting PRIVATE
    ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
    IMAGE_DIR="${CMAKE_SOURCE_DIR}/Images")

enable_testing()
add_executable(sort_check tests/sort_check.cpp)
target_link_libraries(sort_check PRIVATE sorts)
add_test(NAME sort_check COMMAND sort_check)
