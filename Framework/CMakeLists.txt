add_subdirectory(Core)
add_subdirectory(TestWorkflows)
add_subdirectory(Utils)
# FIXME: only run if glfw3 is detected.
if(ARROW_FOUND)
add_subdirectory(ArrowTests)
endif()
add_subdirectory(DebugGUI)
add_subdirectory(Foundation)
