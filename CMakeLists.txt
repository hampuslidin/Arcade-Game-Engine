cmake_minimum_required (VERSION 2.6)
project ("RAW Engine")
set(CMAKE_CXX_STANDARD 14)

# directory variables
set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/lib/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(TMP_DIR ${EXTERNAL_DIR}/.tmp)

# glew variables
set(GLEW_URL https://github.com/nigels-com/glew/releases/download/glew-2.0.0/glew-2.0.0.zip)
set(GLEW_NAME glew-2.0.0)

# glm variables
set(GLM_URL https://github.com/g-truc/glm/releases/download/0.9.8.4/glm-0.9.8.4.zip)
set(GLM_NAME glm-0.9.8.4)

# imgui variables
set(IMGUI_URL https://github.com/ocornut/imgui/archive/v1.49.zip)
set(IMGUI_NAME imgui-1.49)

# SDL2 variables
if(APPLE)
  set(SDL2_URL https://www.libsdl.org/release/SDL2-2.0.5.dmg)
elseif(WIN32)
  set(SDL2_URL https://www.libsdl.org/release/SDL2-devel-2.0.5-VC.zip)
endif()
set(SDL2_NAME SDL2-2.0.5)

# stb variables
set(STB_URL https://github.com/nothings/stb/archive/master.zip)
set(STB_NAME stb)

# tinyobjloader variables
set(TINYOBJLOADER_URL https://github.com/syoyo/tinyobjloader/archive/v1.0.6.zip)
set(TINYOBJLOADER_NAME tinyobjloader-1.0.6)

# tinyxml2 variables
set(TINYXML2_URL https://github.com/leethomason/tinyxml2/archive/4.0.1.zip)
set(TINYXML2_NAME tinyxml2-4.0.1)

# set(STB_IMAGE_IMPL_PATH ${SRC_DIR}/stb_image.cpp)
# set(STB_IMAGE_IMPL_SRC "#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>\n")

# macro for downloading dependencies
set(TYPE_ZIP 0)
set(TYPE_DMG 1)
macro(download_dependency url framework_name type)
  # check if directory already exists
  set(FRAMEWORK_DIR ${EXTERNAL_DIR}/${framework_name})
  if (NOT EXISTS ${FRAMEWORK_DIR})
    if(${type} EQUAL ${TYPE_ZIP})
      # download
      set(ZIP_PATH ${TMP_DIR}/tmp.zip)
      message("Downloading ${framework_name}...")
      file(DOWNLOAD ${url} ${ZIP_PATH})

      # extract
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar -xzf ${ZIP_PATH}
        WORKING_DIRECTORY ${TMP_DIR})

      # get extracted directory
      file(GLOB CHILDREN RELATIVE ${TMP_DIR} ${TMP_DIR}/*)
      foreach(CHILD ${CHILDREN})
        if(IS_DIRECTORY ${TMP_DIR}/${CHILD})
          set(EXTRACTED_DIR ${TMP_DIR}/${CHILD})
          break()
        endif()
      endforeach()

      # move files
      file(RENAME ${EXTRACTED_DIR} ${FRAMEWORK_DIR})
    elseif(${type} EQUAL ${TYPE_DMG})
      # download
      set(DMG_PATH ${TMP_DIR}/tmp.dmg)
      message("Downloading ${framework_name}...")
      file(DOWNLOAD ${url} ${DMG_PATH})

      # attach image
      set(HDIUTIL_OUTPUT_FILE ${TMP_DIR}/hdiutil.out)
      execute_process(
        COMMAND hdiutil attach ${DMG_PATH}
        OUTPUT_VARIABLE HDIUTIL_OUTPUT)

      # get partition name
      string(REPLACE "\n" ";" HDIUTIL_OUTPUT ${HDIUTIL_OUTPUT})
      foreach(LINE ${HDIUTIL_OUTPUT})
        string(REGEX MATCH ".*SDL2" LINE ${LINE})
        if(LINE)
          set(PARTITION_LINE ${LINE})
        endif()
      endforeach()
      string(REPLACE " " ";" PARTITION_LINE_LIST ${PARTITION_LINE})
      string(REPLACE "\t" ";" PARTITION_LINE_LIST "${PARTITION_LINE_LIST}")
      list(LENGTH PARTITION_LINE_LIST PARTITION_LINE_LIST_LENGTH)
      list(GET PARTITION_LINE_LIST 0 PARTITION)
      list(GET PARTITION_LINE_LIST -1 PARTITION_DIR)

      # move
      file(GLOB PARTITION_FILES "${PARTITION_DIR}/[^.]*")
      file(COPY ${PARTITION_FILES} DESTINATION ${FRAMEWORK_DIR})

      # detach image
      execute_process(
        COMMAND hdiutil detach ${PARTITION}
        OUTPUT_QUIET)
    endif()
  endif()
endmacro(download_dependency)

# download dependencies
if (EXISTS ${TMP_DIR})
  file(REMOVE_RECURSE ${TMP_DIR}/*)
else()
  file(MAKE_DIRECTORY ${TMP_DIR})
endif()
download_dependency(${GLEW_URL} ${GLEW_NAME} ${TYPE_ZIP})
download_dependency(${GLM_URL} ${GLM_NAME} ${TYPE_ZIP})
download_dependency(${IMGUI_URL} ${IMGUI_NAME} ${TYPE_ZIP})
download_dependency(${SDL2_URL} ${SDL2_NAME} ${TYPE_DMG})
download_dependency(${STB_URL} ${STB_NAME} ${TYPE_ZIP})
download_dependency(${TINYOBJLOADER_URL} ${TINYOBJLOADER_NAME} ${TYPE_ZIP})
download_dependency(${TINYXML2_URL} ${TINYXML2_NAME} ${TYPE_ZIP})
file(REMOVE_RECURSE ${TMP_DIR})

# create stb_image implementation file
# if (NOT EXISTS ${STB_IMAGE_IMPL_PATH})
#   file(WRITE ${STB_IMAGE_IMPL_PATH} ${STB_IMAGE_IMPL_SRC})
# endif()

# include files
if(APPLE)
  find_path(SDL_INCLUDE_DIR SDL.h)
endif()
include_directories(${INCLUDE_DIR}
  ${EXTERNAL_DIR}/${GLEW_NAME}/include
  ${EXTERNAL_DIR}/${GLM_NAME}
  ${EXTERNAL_DIR}/${IMGUI_NAME}
  ${SDL_INCLUDE_DIR}
  ${EXTERNAL_DIR}/${STB_NAME}
  ${EXTERNAL_DIR}/${TINYOBJLOADER_NAME}
  ${EXTERNAL_DIR}/${TINYXML2_NAME})

# add glew, imgui and tinysml2 as external library
add_library(external
  ${EXTERNAL_DIR}/${GLEW_NAME}/src/glew.c
  ${EXTERNAL_DIR}/${IMGUI_NAME}/imgui.cpp
  ${EXTERNAL_DIR}/${IMGUI_NAME}/imgui_draw.cpp
  ${EXTERNAL_DIR}/${TINYXML2_NAME}/tinyxml2.cpp)

# find SDL2 library
if(APPLE)
  find_library(SDL2_LIBRARY SDL2 REQUIRED)
  mark_as_advanced(SDL2_LIBRARY)
endif()

# find OpenGL package
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# add executable
file(GLOB SOURCE_FILES ${SRC_DIR}/*.cpp)
add_executable(demo ${GUI_TYPE} ${SOURCE_FILES})

# link libraries
target_link_libraries(demo
  external
  ${SDL2_LIBRARY}
  ${OPENGL_LIBRARY})

# install
install(TARGET demo DESTINATION ${BIN_DIR})
