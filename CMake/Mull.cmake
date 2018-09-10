if (NOT LLVM_ROOT)
  message(FATAL_ERROR "This CMakeLists.txt file expects cmake to be called with
  LLVM_ROOT provided:\n \
  -DLLVM_ROOT=path-to-llvm-installation")
endif()

list(APPEND CMAKE_PREFIX_PATH "${LLVM_ROOT}")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

set (MULL_CXX_FLAGS -fno-rtti)

# set_target_properties(LLVMSupport PROPERTIES
#   INTERFACE_LINK_LIBRARIES "curses;z;m"
# )


# set_target_properties(clangEdit PROPERTIES
#   INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;LLVMSupport"
# )


get_target_property(pp clangEdit INTERFACE_LINK_LIBRARIES)

# add_executable(foobar main.cpp)
# target_link_libraries(foobar LLVMSupport)

message("${pp}")
