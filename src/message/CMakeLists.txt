add_library(message INTERFACE)

target_include_directories(message INTERFACE ..)
target_link_libraries(message INTERFACE CONAN_PKG::boost)
