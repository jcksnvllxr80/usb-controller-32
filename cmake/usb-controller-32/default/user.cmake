set(freertos_asm_include_dirs
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/config/default"
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/third_party/rtos/FreeRTOS/Source/include"
    "${CMAKE_CURRENT_LIST_DIR}/../../../usb-controller-32/src/third_party/rtos/FreeRTOS/Source/portable/MPLAB/PIC32MX")

target_include_directories(
    usb_controller_32_default_default_XC32_assembleWithPreprocess
    PRIVATE ${freertos_asm_include_dirs})

get_target_property(project_link_options usb_controller_32_default_image_vgdszqaA LINK_OPTIONS)
list(REMOVE_ITEM project_link_options "-mreserve=boot@0x1fc00490:0x1fc00bf0")
list(APPEND project_link_options "-mreserve=boot@0x1fc00490:0x1fc00bef")
set_target_properties(
    usb_controller_32_default_image_vgdszqaA
    PROPERTIES LINK_OPTIONS "${project_link_options}")