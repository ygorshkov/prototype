add_executable(client main.cpp)

target_link_libraries(client common serialization message cppserver cppbenchmark pfr CONAN_PKG::yas)
