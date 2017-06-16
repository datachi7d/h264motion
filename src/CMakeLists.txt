set (exe_SOURCES
	main.c)

include_directories ( ../lib )

add_executable (h264m ${exe_SOURCES})
target_link_libraries(h264m h264motion)