#####################################################################################
# Set source files
#####################################################################################

# find_package(OPENCL REQUIRED)
# include_directories(OpenCL_INCLUDE_DIRS)
# message(STATUS "Loaded OpenCL version ${OpenCL_VERSION_STRING}")
# message(STATUS "OpenCL Libraries: ${OpenCL_LIBRARIES}")

set(LVR2_CL_NORMAL_SRC
    Main.cpp
    Options.cpp
)

#####################################################################################
# Setup dependencies to external libraries
#####################################################################################

set(LVR2_CL_NORMAL_DEPS
    lvr2
    ${OpenCL_LIBRARIES}
)



#####################################################################################
# Add executable
#####################################################################################
add_executable(lvr2_cl_normals ${LVR2_CL_NORMAL_SRC})

target_link_libraries(lvr2_cl_normals ${LVR2_CL_NORMAL_DEPS})
