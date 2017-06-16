set (lib_SOURCES
	h264motion.c)

add_library (h264motion SHARED ${lib_SOURCES})


if (FFMPEG_FOUND)
  message("Found FFMPEG/LibAV libraries")
  include_directories(${FFMPEG_INCLUDE_DIR})
  target_link_libraries (h264motion ${FFMPEG_LIBRARIES})
else (FFMPEG_FOUND)
  message("Can't find libavcodec, libavformat or libavutil. Add them!")
endif (FFMPEG_FOUND)


INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(h264motion ${SDL2_LIBRARIES})

install (TARGETS h264motion
   LIBRARY DESTINATION lib
)

