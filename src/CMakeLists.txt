find_package(LLVM REQUIRED CONFIG)

llvm_map_components_to_libnames(LLVM_LIBS core native)

set(SOURCES "main.cpp")

add_executable(pspamm ${SOURCES})
target_compile_features(pspamm PUBLIC cxx_std_17)
target_include_directories(pspamm PRIVATE .
                                          ../submodules/multiple-dispatch/include/
                                          ${LLVM_INCLUDE_DIRS})
target_compile_definitions(pspamm PRIVATE ${LLVM_DEFINITIONS})
target_link_libraries(pspamm PRIVATE ${LLVM_LIBS})
