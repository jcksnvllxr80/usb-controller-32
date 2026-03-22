get_target_property(project_link_options usb_controller_32_usb_controller_32_image_8nCe6iwC LINK_OPTIONS)
list(REMOVE_ITEM project_link_options "-mreserve=boot@0x1fc00490:0x1fc00bf0")
list(APPEND project_link_options "-mreserve=boot@0x1fc00490:0x1fc00bef")
set_target_properties(
    usb_controller_32_usb_controller_32_image_8nCe6iwC
    PROPERTIES LINK_OPTIONS "${project_link_options}")
