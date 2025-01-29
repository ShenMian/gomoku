add_requires("sfml 3.0.0")

set_languages("cxx23")

target("gomoku")
    set_kind("binary")
    add_files("src/*.cpp")
    add_headerfiles("src/*.hpp")
    add_packages("sfml")
