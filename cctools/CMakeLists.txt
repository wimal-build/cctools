cmake_minimum_required(VERSION 3.0)

project(cctools)

# Endian check
include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    add_definitions(-D__BIG_ENDIAN__=1)
else()
    add_definitions(-D__LITTLE_ENDIAN__=1)
endif()

# Common definitions
add_definitions(
    -D_MACH_I386_THREAD_STATUS_FPSTATE_LEGACY_FIELD_NAMES_
    -D_ARCHITECTURE_I386_FPU_FPSTATE_LEGACY_FIELD_NAMES_
    -DLTO_SUPPORT
)

# Include directories
include_directories(port sources/include)

### libstuff
aux_source_directory(sources/libstuff stuff_sources)
# Use cpp for symbol __cxa_demangle
list(APPEND stuff_sources port/port.cpp)
add_library(stuff ${stuff_sources})
target_link_libraries(stuff dl)
set_property(
    SOURCE sources/libstuff/execute.c
    PROPERTY COMPILE_DEFINITIONS
    # Make the prefix contains the prefix in executable names.
    # i.e. `/usr/bin/apple-ar` is prefixed with `/usr/bin/apple-`.
    # Remove the default implementation here, reimplement in port.cpp
    add_execute_list_with_prefix=no_add_execute_list_with_prefix
    cmd_with_prefix=no_cmd_with_prefix
)

### ar
aux_source_directory(sources/ar ar_sources)
add_executable(apple-ar ${ar_sources})
# Avoid duplicated symbols.
target_compile_definitions(apple-ar PRIVATE -Derror=xerror -Dprint=xprint)
target_link_libraries(apple-ar stuff)

### otool
aux_source_directory(sources/otool otool_sources)
add_executable(apple-otool ${otool_sources})
target_include_directories(apple-otool BEFORE PRIVATE port)

# Find LLVMMCDisassembler for otool
if(NOT TARGET LLVMMCDisassembler)
    find_library(LIB-LLVMMCDisassembler NAMES LLVMMCDisassembler LLVM LLVM-8)

    if(LIB-LLVMMCDisassembler STREQUAL LIB-LLVMMCDisassembler-NOTFOUND)
        message(STATUS "LLVMMCDisassembler is not found, use shared llvm instead.")
    else()
        message(STATUS "Found LLVMMCDisassembler: ${LIB-LLVMMCDisassembler}")

        add_library(LLVMMCDisassembler UNKNOWN IMPORTED)
        set_target_properties(
            LLVMMCDisassembler PROPERTIES
            IMPORTED_LOCATION "${LIB-LLVMMCDisassembler}"
        )
    endif()
endif()
if(TARGET LLVMMCDisassembler)
    set(EXTRA_OTOOL_FLAGS -DSTATIC_LLVM)
    set(EXTRA_OTOOL_LIB LLVMMCDisassembler)
endif()

target_compile_definitions(
    apple-otool PRIVATE
    # Flags from otool/Makefile
    -D_MACH_I386_THREAD_STATUS_FPSTATE_LEGACY_FIELD_NAMES_
	-D_ARCHITECTURE_I386_FPU_FPSTATE_LEGACY_FIELD_NAMES_
    -DLTO_SUPPORT
    -DEFI_SUPPORT
    # Avoid name conflicts for sqrt
    -D__DYNAMIC__
    # Extra flags
    ${EXTRA_OTOOL_FLAGS}
)
target_link_libraries(apple-otool stuff ${EXTRA_OTOOL_LIB} dl)

### checksyms
add_executable(apple-checksyms sources/misc/checksyms.c)
target_link_libraries(apple-checksyms stuff)

### cmpdylib
add_executable(apple-cmpdylib sources/misc/cmpdylib.c)
target_link_libraries(apple-cmpdylib stuff)

### lipo
add_executable(apple-lipo sources/misc/lipo.c)
target_link_libraries(apple-lipo stuff)

### size
add_executable(apple-size sources/misc/size.c)
target_link_libraries(apple-size stuff)

### strings
add_executable(apple-strings sources/misc/strings.c)
target_link_libraries(apple-strings stuff)

### nm
add_executable(apple-nm sources/misc/nm.c)
target_link_libraries(apple-nm stuff)

### libtool
add_executable(apple-libtool sources/misc/libtool.c)
# Set `cmd_flags.ranlib` to TRUE when the executable ends with `ranlib`.
target_compile_definitions(apple-libtool PRIVATE -Dstrncmp=cctools_strncmp)
target_link_libraries(apple-libtool stuff)

### ranlib
add_executable(apple-ranlib sources/misc/libtool.c)
# Set `cmd_flags.ranlib` to TRUE when the executable ends with `ranlib`.
target_compile_definitions(apple-ranlib PRIVATE -DHOOK_STRNCMP)
target_link_libraries(apple-ranlib stuff)

### redo_prebinding
add_executable(apple-redo_prebinding sources/misc/redo_prebinding.c)
target_link_libraries(apple-redo_prebinding stuff)

### seg_addr_table
add_executable(apple-seg_addr_table sources/misc/seg_addr_table.c)
target_link_libraries(apple-seg_addr_table stuff)

### check_dylib
add_executable(apple-check_dylib sources/misc/check_dylib.c)
target_link_libraries(apple-check_dylib stuff)

### inout
add_executable(apple-inout sources/misc/inout.c)
target_link_libraries(apple-inout stuff)

### codesign_allocate
add_executable(apple-codesign_allocate sources/misc/codesign_allocate.c)
target_link_libraries(apple-codesign_allocate stuff)

### ctf_insert
add_executable(apple-ctf_insert sources/misc/ctf_insert.c)
target_link_libraries(apple-ctf_insert stuff)

### seg_hack
add_executable(apple-seg_hack sources/misc/seg_hack.c)
target_link_libraries(apple-seg_hack stuff)

### install_name_tool
add_executable(apple-install_name_tool sources/misc/install_name_tool.c)
target_link_libraries(apple-install_name_tool stuff)

### indr
add_executable(apple-indr sources/misc/indr.c)
target_link_libraries(apple-indr stuff)

### strip
add_executable(apple-strip sources/misc/strip.c)
target_link_libraries(apple-strip stuff)

### nmedit
add_executable(apple-nmedit sources/misc/strip.c)
target_link_libraries(apple-nmedit stuff)
target_compile_definitions(apple-nmedit PRIVATE -DNMEDIT)

### segedit
add_executable(apple-segedit sources/misc/segedit.c)
target_link_libraries(apple-segedit stuff)

### pagestuff
add_executable(apple-pagestuff sources/misc/pagestuff.c)
target_link_libraries(apple-pagestuff stuff)

### bitcode_strip
add_executable(apple-bitcode_strip sources/misc/bitcode_strip.c)
target_link_libraries(apple-bitcode_strip stuff)

### makerelocs
add_executable(apple-makerelocs sources/efitools/makerelocs.c)
target_link_libraries(apple-makerelocs stuff)

### mtoc
add_executable(apple-mtoc sources/efitools/mtoc.c)
target_link_libraries(apple-mtoc stuff)

### gprof
aux_source_directory(sources/gprof gprof_sources)
add_executable(apple-gprof ${gprof_sources})
target_compile_definitions(apple-gprof PRIVATE -DNeXT_MOD)
target_link_libraries(apple-gprof stuff)

### install
set(
    cctools_targets
    apple-ar
    apple-otool
    apple-checksyms
    apple-cmpdylib
    apple-lipo
    apple-size
    apple-strings
    apple-nm
    apple-libtool
    apple-ranlib
    apple-redo_prebinding
    apple-seg_addr_table
    apple-check_dylib
    apple-inout
    apple-codesign_allocate
    apple-ctf_insert
    apple-seg_hack
    apple-install_name_tool
    apple-indr
    apple-strip
    apple-nmedit
    apple-segedit
    apple-pagestuff
    apple-bitcode_strip
    apple-makerelocs
    apple-mtoc
    apple-gprof
)
add_custom_target(cctools DEPENDS ${cctools_targets})
install(TARGETS ${cctools_targets} DESTINATION bin COMPONENT cctools)
