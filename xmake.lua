add_requires("sfml 2.6.*")

set_languages("c++23")

target("gomoku")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("sfml")