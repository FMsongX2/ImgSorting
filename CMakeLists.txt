cmake_minimum_required(VERSION 3.24)
project(ImgSorting C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(IMGSORTING_VENDOR_SDL "Build SDL3 from source and link it statically" OFF)
if(IMGSORTING_VENDOR_SDL)
    include(FetchContent)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.16
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(SDL3)
    set(IMGSORTING_SDL SDL3::SDL3-static)
else()
    find_package(SDL3 3.2 REQUIRED CONFIG)
    set(IMGSORTING_SDL SDL3::SDL3)
endif()

function(embed_file source symbol)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${source})
    file(READ ${source} hex HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
    string(REPEAT "0x..," 16 line)
    string(REGEX REPLACE "(${line})" "\\1\n" bytes "${bytes}")
    file(WRITE ${CMAKE_BINARY_DIR}/${symbol}.cpp.in
        "#include <cstddef>\nextern const unsigned char ${symbol}[] = {\n${bytes}};\nextern const std::size_t ${symbol}Size = sizeof(${symbol});\n")
    configure_file(${CMAKE_BINARY_DIR}/${symbol}.cpp.in ${CMAKE_BINARY_DIR}/${symbol}.cpp COPYONLY)
endfunction()
embed_file(${CMAKE_SOURCE_DIR}/assets/default.wav kDefaultWav)
embed_file(${CMAKE_SOURCE_DIR}/assets/stalin.jpeg kStalinJpeg)

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

add_library(audio_decode STATIC src/audio_decode.cpp)
target_include_directories(audio_decode PUBLIC src)
if(APPLE)
    target_link_libraries(audio_decode PUBLIC "-framework AudioToolbox" "-framework CoreFoundation")
elseif(WIN32)
    target_link_libraries(audio_decode PUBLIC mfplat mfreadwrite mfuuid ole32)
endif()

add_executable(ImgSorting src/main.cpp third_party/stb_image_impl.cpp ${CMAKE_BINARY_DIR}/kDefaultWav.cpp ${CMAKE_BINARY_DIR}/kStalinJpeg.cpp)
set_target_properties(ImgSorting PROPERTIES WIN32_EXECUTABLE ON)
target_include_directories(ImgSorting PRIVATE third_party)
target_link_libraries(ImgSorting PRIVATE sorts audio_decode ${IMGSORTING_SDL})

enable_testing()
add_executable(sort_check tests/sort_check.cpp)
target_link_libraries(sort_check PRIVATE sorts)
add_test(NAME sort_check COMMAND sort_check)
add_executable(audio_check tests/audio_check.cpp)
target_link_libraries(audio_check PRIVATE audio_decode)
add_test(NAME audio_check COMMAND audio_check ${CMAKE_SOURCE_DIR}/tests/data/tone.m4a)
