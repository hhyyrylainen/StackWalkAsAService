if(UNIX)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

# add_library(stackwalkcommon
#   )

# target_link_libraries(stackwalkcommon PUBLIC
#   clangFrontend
#   
#   )

# set_target_properties(stackwalkcommon PROPERTIES
#   CXX_STANDARD 17
#   CXX_EXTENSIONS OFF
#   POSITION_INDEPENDENT_CODE ON  
#   )

# target_include_directories(stackwalkcommon PUBLIC ${CMAKE_CURRENT_LIST_DIR})


add_executable(stackwalkwebapp
  Application.h Application.cpp
  StackWalkRunner.h StackWalkRunner.cpp
  ResultWidget.h ResultWidget.cpp
  StackWalkOperation.h StackWalkOperation.cpp
  APIV1.h APIV1.cpp
  main.cpp)

target_link_libraries(stackwalkwebapp PRIVATE wt wthttp ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(stackwalkwebapp PUBLIC ${CMAKE_CURRENT_LIST_DIR})

set_target_properties(stackwalkwebapp PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
  )

install(TARGETS stackwalkwebapp)
