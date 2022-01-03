set(INC
  .
  ..
  ../..
  ../../../makesdna
)

set(INC_SYS

)

set(SRC
  kra_api.h

  kra_api.c
)

set(LIB
)

if(WITH_IMAGE_KRA)
  list(APPEND LIB
    -lzip
  )
  add_definitions(-DWITH_KRA)
endif()

blender_add_lib(bf_imbuf_kra "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
