# Add missing include dirs for assembly files that #include FreeRTOS headers
set(_usb_toolchain_prefix usb_controller_32_usb_controller_32_usb_controller_32_toolchain)
set(_usb_asm_target "${_usb_toolchain_prefix}_assemble")
set(_usb_aspp_target "${_usb_toolchain_prefix}_assembleWithPreprocess")
set(_usb_c_target "${_usb_toolchain_prefix}_compile")
set(_usb_cpp_target "${_usb_toolchain_prefix}_compile_cpp")

target_include_directories(${_usb_aspp_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/config/default"
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/third_party/rtos/FreeRTOS/Source/include"
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/third_party/rtos/FreeRTOS/Source/portable/MPLAB/PIC32MX"
)

# ---------- Strip debug flags for production / program-only builds ----------

# --- Assembler (plain .s) — only if target exists ---
if(TARGET ${_usb_asm_target})
    get_target_property(_asm_opts ${_usb_asm_target} COMPILE_OPTIONS)
    if(_asm_opts)
        list(TRANSFORM _asm_opts REPLACE
            ",--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--gdwarf-2,--defsym=__MPLAB_DEBUGGER_PK5=1" "")
        set_target_properties(${_usb_asm_target}
            PROPERTIES COMPILE_OPTIONS "${_asm_opts}")
    endif()
    set_property(TARGET ${_usb_asm_target}
        PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")
endif()

# --- Assembler with pre-process (.S) ---
if(TARGET ${_usb_aspp_target})
    get_target_property(_aspp_opts ${_usb_aspp_target} COMPILE_OPTIONS)
    if(_aspp_opts)
        list(TRANSFORM _aspp_opts REPLACE
            ",--defsym=__MPLAB_DEBUG=1,--gdwarf-2,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1" "")
        set_target_properties(${_usb_aspp_target}
            PROPERTIES COMPILE_OPTIONS "${_aspp_opts}")
    endif()
    set_property(TARGET ${_usb_aspp_target}
        PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")
endif()

# --- C compiler ---
if(TARGET ${_usb_c_target})
    set_property(TARGET ${_usb_c_target}
        PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")
endif()

# --- C++ compiler ---
if(TARGET ${_usb_cpp_target})
    set_property(TARGET ${_usb_cpp_target}
        PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")
endif()

# --- Linker: strip debug flags and fix boot reserve ---
get_property(_usb_build_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(_target IN LISTS _usb_build_targets)
    if(_target MATCHES "^usb_controller_32_usb_controller_32_image_")
        set(_usb_image_target "${_target}")
        break()
    endif()
endforeach()

if(DEFINED _usb_image_target)
    get_target_property(project_link_options ${_usb_image_target} LINK_OPTIONS)
    list(REMOVE_ITEM project_link_options "-mdebugger" "-g")
    list(REMOVE_ITEM project_link_options "-mreserve=boot@0x1fc00490:0x1fc00bf0")
    list(APPEND project_link_options "-mreserve=boot@0x1fc00490:0x1fc00bef")
    # Remove __MPLAB_DEBUG and __DEBUG from the -Wl options string
    list(TRANSFORM project_link_options REPLACE ",--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1," ",")
    set_target_properties(
        ${_usb_image_target}
        PROPERTIES LINK_OPTIONS "${project_link_options}")
    set_property(TARGET ${_usb_image_target}
        PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")
endif()
