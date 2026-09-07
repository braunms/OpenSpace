##########################################################################################
#                                                                                        #
# Visualization Research Center of the University of Stuttgart (VISUS)                   #
#                                                                                        #
# Copyright (c) 2026                                                                     #
#                                                                                        #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this   #
# software and associated documentation files (the "Software"), to deal in the Software  #
# without restriction, including without limitation the rights to use, copy, modify,     #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to     #
# permit persons to whom the Software is furnished to do so, subject to the following    #
# conditions:                                                                            #
#                                                                                        #
# The above copyright notice and this permission notice shall be included in all copies  #
# or substantial portions of the Software.                                               #
#                                                                                        #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,    #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A          #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT     #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF   #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE   #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                          #
##########################################################################################

# Module is only available on Windows due to the use of the NatNet SDK, which is only supported on Windows.
# set(OPENSPACE_APPLICATION_OPENSPACE_LAUNCHER OFF) required when used on the old Powerwall at VISUS (due to WIndows Server 2016 incompatibility with Qt6)
if (WIN32)

  include(${PROJECT_SOURCE_DIR}/support/cmake/module_definition.cmake)

  set(HEADER_FILES
    include/renderabletracking.h
  )
  source_group("Header Files" FILES ${HEADER_FILES})

  set(SOURCE_FILES
    src/renderabletracking.cpp
  )
  source_group("Source Files" FILES ${SOURCE_FILES})

  create_new_module(
    "PowerwallTracking"
    powerwalltracking_module
    ${HEADER_FILES} ${SOURCE_FILES}
  )

  # Add the external powerwall tracking library as a dependency
  add_subdirectory(ext/powerwall-tracking)

  set_target_properties(tracking        PROPERTIES FOLDER "External")
  set_target_properties(vrpn            PROPERTIES FOLDER "External")
  set_target_properties(powerwall_glm   PROPERTIES FOLDER "External")
  set_target_properties(natnet          PROPERTIES FOLDER "External")

  #set(TRACKING_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/powerwall-tracking)
  #set(TRACKING_INCLUDE_DIR "${TRACKING_ROOT_DIR}/tracking/include")
  #set(TRACKING_VRPN_INCLUDE_DIR "${TRACKING_ROOT_DIR}/external/vrpn")
  #set(TRACKING_GLM_INCLUDE_DIR "${TRACKING_ROOT_DIR}/external/powerwall_glm")
  #target_include_directories(${powerwalltracking_module} SYSTEM PUBLIC ${TRACKING_INCLUDE_DIR} ${TRACKING_VRPN_INCLUDE_DIR} ${TRACKING_GLM_INCLUDE_DIR})

  target_link_libraries(${powerwalltracking_module} PUBLIC tracking vrpn powerwall_glm natnet)

  set(TRACKING_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ext/powerwall-tracking/tracking/config/tracking.cfg")
  add_custom_command(TARGET ${powerwalltracking_module} POST_BUILD  
      COMMAND ${CMAKE_COMMAND} -E copy  
          ${TRACKING_CONFIG_FILE}  
          "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(Configuration)/tracking.cfg" 
      COMMENT "Copying tracking.conf to binary directory"  
  )  

  set(NATNET_DLL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext/powerwall-tracking/external/natnet/lib/x64/NatNetLib.dll")
  add_custom_command(TARGET ${powerwalltracking_module} POST_BUILD  
      COMMAND ${CMAKE_COMMAND} -E copy  
          ${NATNET_DLL_DIR}  
          "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(Configuration)/NatNetLib.dll" 
      COMMENT "Copying NatNetLib.dll to binary directory"  
  )

else (WIN32)

  message(WARNING "The Powerwall Tracking Module is only supported on Windows operating system. The module will not be built.")

endif () # WIN32


