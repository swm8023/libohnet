
# libohstl
file(GLOB libohsrc ohev/*.c ohstl/*.c ohutil/*.c)
add_library(oh STATIC ${libohsrc})
add_library(oh_so SHARED ${libohsrc})
set_target_properties(oh_so PROPERTIES OUTPUT_NAME "oh")

# libohnet
# file(GLOB libohevsrc ohev/*.c ohutil/*.c)
# add_library(ohev STATIC ${libohevsrc})
# add_library(ohev_so SHARED ${libohevsrc})
# set_target_properties(ohev_so PROPERTIES OUTPUT_NAME "ohev")