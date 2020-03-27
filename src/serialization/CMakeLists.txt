add_library(serialization INTERFACE)

target_sources(serialization INTERFACE serializer.h)

target_include_directories(serialization INTERFACE ..)

target_link_libraries(serialization INTERFACE pfr INTERFACE CONAN_PKG::yas INTERFACE message)
