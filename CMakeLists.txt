cmake_minimum_required (VERSION 3.0)
project ("RAW Engine")
set(CMAKE_CXX_STANDARD 14)

# directory variables
set(BIN_DIR "${PROJECT_SOURCE_DIR}/bin")
set(DATA_DIR "${PROJECT_SOURCE_DIR}/data")
set(LIB_DIR "${PROJECT_SOURCE_DIR}/lib")
set(EXTERNAL_SRC_DIR "${LIB_DIR}/external")
set(CONFIG_DIR "${PROJECT_SOURCE_DIR}/config")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(EXTERNAL_INCLUDE_DIR "${INCLUDE_DIR}/external")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(TMP_DIR "${EXTERNAL_SRC_DIR}/.tmp")

# glew variables
if(APPLE)
  set(GLEW_URL "https://github.com/nigels-com/glew/releases/download/glew-2.0.0/glew-2.0.0.zip")
elseif(WIN32)
  set(GLEW_URL "https://sourceforge.net/projects/glew/files/glew/2.0.0/glew-2.0.0-win32.zip/download")
endif()
set(GLEW_NAME "glew-2.0.0")

# glm variables
set(GLM_URL "https://github.com/g-truc/glm/releases/download/0.9.8.4/glm-0.9.8.4.zip")
set(GLM_NAME "glm-0.9.8.4")

# imgui variables
set(IMGUI_URL "https://github.com/ocornut/imgui/archive/v1.49.zip")
set(IMGUI_NAME "imgui-1.49")

# SDL2 variables
if(APPLE)
  set(SDL2_URL "https://www.libsdl.org/release/SDL2-2.0.5.dmg")
elseif(WIN32)
  set(SDL2_URL "https://www.libsdl.org/release/SDL2-devel-2.0.5-VC.zip")
endif()
set(SDL2_NAME "SDL2-2.0.5")

# stb variables
set(STB_URL "https://github.com/nothings/stb/archive/master.zip")
set(STB_NAME "stb")

# tinyobjloader variables
set(TINYOBJLOADER_URL "https://github.com/syoyo/tinyobjloader/archive/v1.0.6.zip")
set(TINYOBJLOADER_NAME "tinyobjloader-1.0.6")

# tinyxml2 variables
set(TINYXML2_URL "https://github.com/leethomason/tinyxml2/archive/4.0.1.zip")
set(TINYXML2_NAME "tinyxml2-4.0.1")

# macro for downloading dependencies
set(TYPE_ZIP 0)
set(TYPE_DMG 1)
macro(download_dependency url framework_name type)
  # check if directory already exists
  set(FRAMEWORK_DIR ${EXTERNAL_SRC_DIR}/${framework_name})
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
if(APPLE)
  download_dependency(${SDL2_URL} ${SDL2_NAME} ${TYPE_DMG})
elseif(WIN32)
  download_dependency(${SDL2_URL} ${SDL2_NAME} ${TYPE_ZIP})
endif()
download_dependency(${STB_URL} ${STB_NAME} ${TYPE_ZIP})
download_dependency(${TINYOBJLOADER_URL} ${TINYOBJLOADER_NAME} ${TYPE_ZIP})
download_dependency(${TINYXML2_URL} ${TINYXML2_NAME} ${TYPE_ZIP})
file(REMOVE_RECURSE ${TMP_DIR})

# configure file
configure_file(
  "${CONFIG_DIR}/engineConfig.h.in" 
  "${INCLUDE_DIR}/engineConfig.h")

# include files
file(COPY
  "${EXTERNAL_SRC_DIR}/${GLEW_NAME}/include/GL"
  "${EXTERNAL_SRC_DIR}/${GLM_NAME}/glm"
  "${EXTERNAL_SRC_DIR}/${IMGUI_NAME}/imgui.h"
  "${EXTERNAL_SRC_DIR}/${IMGUI_NAME}/imconfig.h"
  "${EXTERNAL_SRC_DIR}/${STB_NAME}/stb_image.h"
  "${EXTERNAL_SRC_DIR}/${TINYOBJLOADER_NAME}/tiny_obj_loader.h"
  "${EXTERNAL_SRC_DIR}/${TINYXML2_NAME}/tinyxml2.h"
  DESTINATION "${EXTERNAL_INCLUDE_DIR}")
include_directories("${INCLUDE_DIR}" "${EXTERNAL_INCLUDE_DIR}" "${LIB_DIR}")

set(EXTERNAL_LIBRARY_SOURCES
  "${EXTERNAL_SRC_DIR}/${IMGUI_NAME}/imgui.cpp"
  "${EXTERNAL_SRC_DIR}/${IMGUI_NAME}/imgui_draw.cpp"
  "${EXTERNAL_SRC_DIR}/${TINYXML2_NAME}/tinyxml2.cpp")

# add glew library
if(APPLE)
  list(APPEND ${EXTERNAL_LIBRARY_SOURCES}
    "${EXTERNAL_SRC_DIR}/${GLEW_NAME}/src/glew.c")
elseif(WIN32)
  file(COPY
    "${EXTERNAL_SRC_DIR}/${GLEW_NAME}/lib/Release/x64/glew32.lib"
    DESTINATION "${LIB_DIR}")
  set(GLEW_LIBRARY "${LIB_DIR}/glew32.lib")
  mark_as_advanced(GLEW_LIBRARY)
endif()

# add SDL2 library
if(APPLE)
  file(COPY "${EXTERNAL_SRC_DIR}/${SDL2_NAME}/SDL2.framework"
    DESTINATION "${LIB_DIR}")
  set(SDL2_LIBRARY "${LIB_DIR}/SDL2.framework")
  include_directories("${SDL2_LIBRARY}/Headers")
elseif(WIN32)
  file(COPY
    "${EXTERNAL_SRC_DIR}/${SDL2_NAME}/lib/x64/SDL2.lib"
    "${EXTERNAL_SRC_DIR}/${SDL2_NAME}/lib/x64/SDL2main.lib"
    DESTINATION "${LIB_DIR}")
  file(GLOB SDL_INCLUDE_FILES "${EXTERNAL_SRC_DIR}/${SDL2_NAME}/include/*")
  file(COPY ${SDL_INCLUDE_FILES} DESTINATION "${EXTERNAL_INCLUDE_DIR}")
  set(SDL2_LIBRARY "${LIB_DIR}/SDL2.lib" "${LIB_DIR}/SDL2main.lib")
endif()
mark_as_advanced(SDL2_LIBRARY)

# add external library
add_library(external ${EXTERNAL_LIBRARY_SOURCES})

# find OpenGL package
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# add executable
file(GLOB SOURCE_FILES "${SRC_DIR}/*.cpp")
file(GLOB INCLUDE_FILES "${INCLUDE_DIR}/*.h*")
add_executable(demo ${SOURCE_FILES} ${INCLUDE_FILES})

# link libraries
target_link_libraries(demo
  external
  ${GLEW_LIBRARY}
  ${SDL2_LIBRARY}
  ${OPENGL_LIBRARY})

# target properties
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LIB_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIB_DIR}")

# copy dll's (if on Windows)
if(WIN32)
  add_custom_command(TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${EXTERNAL_SRC_DIR}/${GLEW_NAME}/bin/Release/x64/glew32.dll
      $<TARGET_FILE_DIR:demo>)
  add_custom_command(TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${EXTERNAL_SRC_DIR}/${SDL2_NAME}/lib/x64/SDL2.dll
      $<TARGET_FILE_DIR:demo>)
endif()
