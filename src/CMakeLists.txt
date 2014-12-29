
file(GLOB libfnsrc fnutil/*.c)

add_library(fastnet STATIC ${libfnsrc})
add_library(fastnet_so SHARED ${libfnsrc})
set_target_properties(fastnet_so PROPERTIES OUTPUT_NAME "fastnet")

target_link_libraries(fastnet_so -O2)
