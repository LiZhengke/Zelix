function(zelix_apply_common_compile_options target)
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-m32>
        $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
        $<$<COMPILE_LANGUAGE:C>:-fno-builtin>
        $<$<COMPILE_LANGUAGE:C>:-fno-stack-protector>
        $<$<COMPILE_LANGUAGE:C>:-fno-pic>
        $<$<COMPILE_LANGUAGE:C>:-fno-pie>
        $<$<COMPILE_LANGUAGE:ASM>:-m32>
        $<$<COMPILE_LANGUAGE:ASM>:-fno-pic>
        $<$<COMPILE_LANGUAGE:ASM>:-fno-pie>
    )
endfunction()
