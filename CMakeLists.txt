cmake_minimum_required(VERSION 3.20)

project(pokemon_fin C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(POKEMON_ENABLE_LLM "Build with libcurl-backed LLM support" OFF)

add_executable(pokemon
    pokemon.c
    dogam/dogam.c
    skill/skill.c
    battlelogic/battlelogic.c
    entry/entry.c
    llm/llm.c
    score/score.c
)

target_include_directories(pokemon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/llm
    ${CMAKE_CURRENT_SOURCE_DIR}/score
)

if(POKEMON_ENABLE_LLM)
    find_package(CURL REQUIRED)
    target_link_libraries(pokemon PRIVATE CURL::libcurl)
else()
    target_compile_definitions(pokemon PRIVATE LLM_DISABLED)
endif()

if(MSVC)
    target_compile_options(pokemon PRIVATE /utf-8 /W4)
else()
    target_compile_options(pokemon PRIVATE -Wall -Wextra)
endif()
