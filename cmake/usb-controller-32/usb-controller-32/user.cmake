# Add missing include dirs for assembly files that #include FreeRTOS headers
target_include_directories(usb_controller_32_usb_controller_32_usb_controller_32_toolchain_assembleWithPreprocess PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/config/default"
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/third_party/rtos/FreeRTOS/Source/include"
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/third_party/rtos/FreeRTOS/Source/portable/MPLAB/PIC32MX"
)

# ---------- Strip debug flags for production / program-only builds ----------

# --- Assembler (plain .s) — only if target exists ---
if(TARGET usb_controller_32_usb_controller_32_usb_controller_32_toolchain_assemble)
    get_target_property(_asm_opts usb_controller_32_usb_controller_32_usb_controller_32_toolchain_assemble COMPILE_OPTIONS)
    if(_asm_opts)
        list(TRANSFORM _asm_opts REPLACE
            ",--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--gdwarf-2,--defsym=__MPLAB_DEBUGGER_PK5=1" "")
        set_target_properties(usb_controller_32_usb_controller_32_usb_controller_32_toolchain_assemble
            PROPERTIES COMPILE_OPTIONS "${_asm_opts}")
    endif()
    set_property(TARGET usb_controller_32_usb_controller_32_usb_controller_32_toolchain_assemble
        PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")
endif()

# --- Assembler with pre-process (.S) ---
get_target_property(_aspp_opts usb_controller_32_usb_controller_32_usb_controller_32_toolchain_assembleWithPreprocess COMPILE_OPTIONS)
if(_aspp_opts)
    list(TRANSFORM _aspp_opts REPLACE
        ",--defsym=__MPLAB_DEBUG=1,--gdwarf-2,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK5=1" "")
    set_target_properties(usb_controller_32_usb_controller_32_usb_controller_32_toolchain_assembleWithPreprocess
        PROPERTIES COMPILE_OPTIONS "${_aspp_opts}")
endif()
set_property(TARGET usb_controller_32_usb_controller_32_usb_controller_32_toolchain_assembleWithPreprocess
    PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")

# --- C compiler ---
set_property(TARGET usb_controller_32_usb_controller_32_usb_controller_32_toolchain_compile
    PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")

# --- C++ compiler ---
set_property(TARGET usb_controller_32_usb_controller_32_usb_controller_32_toolchain_compile_cpp
    PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")

# --- Linker: strip debug flags and fix boot reserve ---
get_target_property(project_link_options usb_controller_32_usb_controller_32_image_8nCe6iwC LINK_OPTIONS)
list(REMOVE_ITEM project_link_options "-mdebugger" "-g")
list(REMOVE_ITEM project_link_options "-mreserve=boot@0x1fc00490:0x1fc00bf0")
list(APPEND project_link_options "-mreserve=boot@0x1fc00490:0x1fc00bef")
# Remove __MPLAB_DEBUG and __DEBUG from the -Wl options string
list(TRANSFORM project_link_options REPLACE ",--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1," ",")
set_target_properties(
    usb_controller_32_usb_controller_32_image_8nCe6iwC
    PROPERTIES LINK_OPTIONS "${project_link_options}")
set_property(TARGET usb_controller_32_usb_controller_32_image_8nCe6iwC
    PROPERTY COMPILE_DEFINITIONS "XPRJ_usb_controller_32=usb_controller_32")
