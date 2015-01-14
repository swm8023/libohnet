
# libohstl
file(GLOB libohstlsrc ohstl/*.c ohutil/*.c)
#add_library(ohstl STATIC ${libohstlsrc})
add_library(ohstl_so SHARED ${libohstlsrc})
set_target_properties(ohstl_so PROPERTIES OUTPUT_NAME "ohstl")

# libohnet
file(GLOB libohevsrc ohev/*.c ohutil/*.c)
#add_library(ohev STATIC ${libohevsrc})
add_library(ohev_so SHARED ${libohevsrc})
set_target_properties(ohev_so PROPERTIES OUTPUT_NAME "ohev")