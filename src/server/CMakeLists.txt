add_executable(server server.cpp)

target_link_libraries(server common serialization message cppserver cppbenchmark pfr CONAN_PKG::yas)
