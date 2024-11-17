
cmake_minimum_required (VERSION 3.2)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include (GNUInstallDirs)
include (${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include ("${CMAKE_SOURCE_DIR}/cmake/Ronn2Man.cmake")

include_directories (${CMAKE_INSTALL_PREFIX}/include)
link_directories (${CMAKE_INSTALL_PREFIX}/lib)

project (blupimania)
set (BM_VERSION_MAJOR 1)
set (BM_VERSION_MINOR 6)
set (BM_VERSION_PATCH 3)
set (BM_VERSION_EXTRA "")
set (BM_PRODUCT_NAME "Blupimania")
set (BM_PACKAGE_NAME "blupimania")
set (BM_EXEC         "blupimania")
set (BM_ICON_REF     "blupimania")
set (BM_DESCRIPTION "Blupimania - A mind boggling (brain twisting) game of logic")

option (BM_HTTP_VERSION_CHECK "Run a version check over HTTP (with CURL)" OFF)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.h.in"
  "${PROJECT_BINARY_DIR}/include/config.h"
)

include_directories ("${PROJECT_BINARY_DIR}/include")

if (BUILD_JS)
  file (GLOB sources src/*.c src/*.h src/argtable3/* src/sdl/* src/platform/*_js.*)
else ()
  file (GLOB sources src/*.c src/*.h src/argtable3/* src/sdl/* src/platform/*_sdl.*)
endif ()

file (GLOB_RECURSE po resources/po/*.po)

if (APPIMAGE_APPRUN_PROGRAM AND APPIMAGE_ASSISTANT_PROGRAM)
  set (USE_APPIMAGE ON)
endif ()

configure_file (
  "${PROJECT_SOURCE_DIR}/resources/linux/application.desktop.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${BM_PACKAGE_NAME}.desktop"
  @ONLY
)

set (blupimania_OUTPUT blupimania)

if (NOT DEFINED BUILD_LINUX)
  set (BUILD_LINUX UNIX AND NOT APPLE)
endif ()

if (BUILD_JS)
  set (CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${EMSCRIPTEN_FLAGS})
  set (CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "--preload-file share")
  set (blupimania_OUTPUT blupimania.html)
endif ()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -L${CMAKE_INSTALL_PREFIX}/lib")
endif ()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" AND "${STATIC_BUILD}")
  set (CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static-libgcc -static-libstdc++")
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -DDEBUG")
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
endif ()

# These copies are necessary with our stuff for AppImage because it's not
# supported by CPack.
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  file (COPY resources/data DESTINATION share/blupimania)
else ()
  file (
    COPY resources/data DESTINATION share/blupimania
    PATTERN "world2*.blp" EXCLUDE
    PATTERN "world*.dev.blp" EXCLUDE
  )
endif ()
file (COPY resources/data DESTINATION share/blupimania)
file (COPY resources/image DESTINATION share/blupimania)
file (COPY resources/sound DESTINATION share/blupimania)
file (COPY resources/music DESTINATION share/blupimania)
file (COPY LICENSE.all DESTINATION share/doc/blupimania)
file (COPY COPYING DESTINATION share/doc/blupimania)
file (COPY CREDITS DESTINATION share/doc/blupimania)
file (RENAME "${CMAKE_BINARY_DIR}/share/doc/blupimania/LICENSE.all"
             "${CMAKE_BINARY_DIR}/share/doc/blupimania/copyright")

if (BUILD_LINUX)
  file (COPY resources/icon/hicolor DESTINATION share/icons)
  file (
    COPY resources/linux/org.blupi.${BM_PACKAGE_NAME}.appdata.xml
    DESTINATION share/metainfo
  )
  file (
    COPY ${CMAKE_CURRENT_BINARY_DIR}/${BM_PACKAGE_NAME}.desktop
    DESTINATION share/applications
  )
endif ()

# Windows stuff

if (MINGW)
  file (COPY resources/icon/blupi.ico DESTINATION "${CMAKE_BINARY_DIR}")
  configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/win32/blupimania.rc.in"
    "${CMAKE_BINARY_DIR}/blupimania.rc"
    @ONLY
  )

  list (APPEND sources "${CMAKE_BINARY_DIR}/blupimania.rc")

  set (CMAKE_RC_COMPILER_INIT windres)
  enable_language (RC)
  set (CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")

  # Remove cmd window when executing blupimania
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mwindows")
endif (MINGW)

# Dependencies

if ("${STATIC_BUILD}")
  set (CMAKE_INCLUDE_PATH ${CMAKE_INSTALL_PREFIX}/include)
  set (CMAKE_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib)
endif ()

find_package (PkgConfig REQUIRED)
# Needed for argtable3's ceil
find_library (MATH_LIBRARY m)
if (NOT BUILD_JS)
  set (SDL2_DIR       "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2")
  set (SDL2_image_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2_image")
  set (SDL2_mixer_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/SDL2_mixer")

  if ("${STATIC_BUILD}")
    find_package (SDL2 REQUIRED NO_DEFAULT_PATH)
    include_directories (${SDL2_INCLUDE_DIRS})
    set (blupimania_DEPS ${blupimania_DEPS} ${SDL2_STATIC_LIBRARIES})
    find_package (SDL2_image REQUIRED NO_DEFAULT_PATH)
    include_directories (${SDL2_IMAGE_INCLUDE_DIRS})
    set (blupimania_DEPS ${blupimania_DEPS} SDL2_image::SDL2_image-static)
    find_package (SDL2_mixer REQUIRED NO_DEFAULT_PATH)
    include_directories (${SDL2_MIXER_INCLUDE_DIRS})
    set (blupimania_DEPS ${blupimania_DEPS} SDL2_mixer::SDL2_mixer-static)
  else ()
    find_package (SDL2 REQUIRED)
    find_package (SDL2_image REQUIRED)
    find_package (WebP REQUIRED)
    find_package (SDL2_mixer REQUIRED)
  endif ()
endif ()

if (${PB_HTTP_VERSION_CHECK})
  pkg_search_module (CURL REQUIRED libcurl)
  add_definitions (-DUSE_CURL)
endif ()

if ("${STATIC_BUILD}")
  if (NOT BUILD_JS)
    pkg_search_module (PNG REQUIRED libpng)
    set (blupimania_DEPS ${blupimania_DEPS} ${PNG_STATIC_LIBRARIES})

    # Static dependencies for SDL_mixer
    pkg_search_module (VORBIS REQUIRED vorbisfile)
    set (blupimania_DEPS ${blupimania_DEPS} ${VORBIS_STATIC_LIBRARIES})

    # Static dependencies for SDL_image
    pkg_search_module (WEBPDEMUX REQUIRED libwebpdemux)
    set (blupimania_DEPS ${blupimania_DEPS} ${WEBPDEMUX_STATIC_LIBRARIES})

    if (BUILD_LINUX)
      pkg_search_module (ALSA REQUIRED alsa)
      pkg_search_module (PULSE REQUIRED libpulse)
    endif ()
  endif ()

  if (${PB_HTTP_VERSION_CHECK})
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCURL_STATICLIB")
  endif ()
endif ()

###########################
## Main binary dependencies
###########################

add_executable (${blupimania_OUTPUT} ${sources})

if ("${STATIC_BUILD}")
  if (${PB_HTTP_VERSION_CHECK})
    list (APPEND blupimania_DEPS
      ${CURL_STATIC_LIBRARIES}
    )
  endif ()

  if (BUILD_LINUX)
    list (APPEND blupimania_DEPS
      ${ALSA_STATIC_LIBRARIES}
      ${PULSE_STATIC_LIBRARIES} -lrt
    )
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_INSTALL_PREFIX}/lib/pulseaudio")
  endif ()

  if (BUILD_WINDOWS)
    # Missing SDL2 static dependencies
    list (APPEND blupimania_DEPS
      -lwinmm -limm32 -lsetupapi -lversion
    )
  endif ()
else ()
  set (blupimania_DEPS
    SDL2::SDL2
    SDL2_image::SDL2_image
    WebP::webpdemux
    SDL2_mixer::SDL2_mixer
    ${CURL_LIBRARIES}
    pthread
    ${MATH_LIBRARY}
  )

  if (${PB_HTTP_VERSION_CHECK})
    list (APPEND blupimania_DEPS
      ${CURL_LIBRARIES}
    )
  endif ()
endif ()

target_link_libraries (${blupimania_OUTPUT} PUBLIC ${blupimania_DEPS})

#########
# manpage
#########

add_manpage_target ()
manpage(${PROJECT_NAME} 6)

##############
# Installation
##############

if (NOT BUILD_JS)
  install (TARGETS ${blupimania_OUTPUT}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
  install (
    DIRECTORY resources/data DESTINATION share/blupimania
    PATTERN "*.dat"
  )
  install (DIRECTORY resources/image DESTINATION share/blupimania)
  install (DIRECTORY resources/sound DESTINATION share/blupimania)
  install (DIRECTORY resources/music DESTINATION share/blupimania)
  install (FILES LICENSE.all DESTINATION share/doc/blupimania RENAME copyright)
  install (FILES COPYING DESTINATION share/doc/blupimania)
  install (FILES CREDITS DESTINATION share/doc/blupimania)

  if (BUILD_LINUX)
    install (DIRECTORY resources/icon/hicolor DESTINATION share/icons)
    install (
      FILES resources/linux/org.blupi.${BM_PACKAGE_NAME}.appdata.xml
      DESTINATION share/metainfo
    )
  endif ()

  install (
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${BM_PACKAGE_NAME}.desktop
    DESTINATION share/applications
  )

  # Copy libwinpthread-1.dll which seems not be linkable statically
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows" AND MINGW)
    execute_process (COMMAND cygpath.exe --windows $ENV{MINGW_PREFIX} OUTPUT_VARIABLE MINGW_PATH)
    string (REGEX REPLACE "[ \t\n\r]+$" "" MINGW_PATH "${MINGW_PATH}")
    string (REGEX REPLACE "[\\]" "\\\\\\\\" MINGW_PATH "${MINGW_PATH}")
    install (FILES "${MINGW_PATH}\\\\bin\\\\libwinpthread-1.dll" DESTINATION bin)
  endif ()
endif ()

#########
## Deploy
#########

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND "${STATIC_BUILD}")
  set (CPACK_STRIP_FILES TRUE)
  set (CPACK_PACKAGE_NAME ${BM_PACKAGE_NAME})
  set (CPACK_PACKAGE_VENDOR "blupi.org")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${BM_DESCRIPTION})
  set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  # set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README-user.md")
  set (CPACK_PACKAGE_FILE_NAME ${BM_PACKAGE_NAME})
  set (CPACK_PACKAGE_VERSION "${PB_VERSION_MAJOR}.${PB_VERSION_MINOR}.${PB_VERSION_PATCH}${PB_VERSION_EXTRA}")
  set (CPACK_PACKAGE_INSTALL_DIRECTORY ${BM_PRODUCT_NAME})
  set (CPACK_PACKAGE_EXECUTABLES "blupimania;Blupimania")

  if (USE_APPIMAGE)
    include (LinuxAppImageBuild)
    set (CMAKE_PACKAGED_OUTPUT_PREFIX ${CMAKE_INSTALL_PREFIX})
    APPIMAGE_PACKAGE (blupimania ${BM_PACKAGE_NAME} ${BM_PRODUCT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/resources/linux" "${CMAKE_BINARY_DIR}/share" "" "" ${BM_ICON_REF} SIGN_APP)
  elseif (MINGW)
    set (INSTALLER_FILE_NAME "${BM_PACKAGE_NAME}-${BM_VERSION_MAJOR}.${BM_VERSION_MINOR}.${BM_VERSION_PATCH}${BM_VERSION_EXTRA}")
    set (CPACK_PACKAGE_FILE_NAME "${INSTALLER_FILE_NAME}")
    set (CPACK_GENERATOR "NSIS64")
    set (CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\resources\\\\nsis\\\\installer.bmp")
    set (CPACK_NSIS_COMPRESSOR "/SOLID lzma")
    set (CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\blupimania.exe")
    set (CPACK_NSIS_URL_INFO_ABOUT "https://www.blupi.org")
    set (CPACK_NSIS_MUI_FINISHPAGE_RUN "blupimania")
    if (SIGN_APP)
      set (CPACK_NSIS_SIGN_UNINSTALLER "sign -a -fd sha256 -t http://time.certum.pl -v")
    endif ()

    include (CPack)

    find_program (SIGNTOOL_EXECUTABLE signtool)
    if (SIGN_APP AND SIGNTOOL_EXECUTABLE)
      add_custom_command (TARGET blupimania
                          POST_BUILD
                          COMMAND ${SIGNTOOL_EXECUTABLE}
                            sign -a -fd sha256 -t http://time.certum.pl -v
                            "${CMAKE_BINARY_DIR}/blupimania.exe")
    endif ()

    add_custom_command (TARGET blupimania
                        POST_BUILD
                        COMMAND make package)

    if (SIGN_APP AND SIGNTOOL_EXECUTABLE)
      add_custom_command (TARGET blupimania
                          POST_BUILD
                          COMMAND ${SIGNTOOL_EXECUTABLE}
                            sign -a -fd sha256 -t http://time.certum.pl
                            "${CMAKE_BINARY_DIR}/${INSTALLER_FILE_NAME}.exe")
    endif ()
  elseif (APPLE)
    set (CPACK_GENERATOR "Bundle")
    set (CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/icon.icns")
    set (CPACK_BUNDLE_NAME ${BM_PRODUCT_NAME})
    set (CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/Info.plist")
    set (CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/Blupimania")
    if (SIGN_APP)
      set (CPACK_BUNDLE_APPLE_CERT_APP "Developer ID Application: Mathieu Schroeter")
      set (CPACK_BUNDLE_APPLE_CODESIGN_PARAMETER "--deep -f --options runtime --timestamp")
      set (CPACK_BUNDLE_APPLE_CODESIGN_FILES "/Contents/Resources/bin/blupimania")
      set (CPACK_APPLE_BUNDLE_ID "blupimania")
    endif ()
    set (CPACK_PACKAGE_FILE_NAME "blupimania-${BM_VERSION_MAJOR}.${BM_VERSION_MINOR}.${BM_VERSION_PATCH}${BM_VERSION_EXTRA}")

    set (CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/background.tiff")
    set (CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/dmgsetup.scpt")

    set (BUNDLE_VERSION "${BM_VERSION_MAJOR}.${BM_VERSION_MINOR}.${BM_VERSION_PATCH}${BM_VERSION_EXTRA}")
    set (BUNDLE_IDENTIFIER "org.blupi.mania")
    set (BUNDLE_ICON_REF "Blupimania")

    configure_file (
      "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/Info.plist.in"
      "${CMAKE_BINARY_DIR}/Info.plist"
      @ONLY
    )

    include (CPack)
    add_custom_command (TARGET blupimania
                        POST_BUILD
                        COMMAND make package)
  endif ()
endif ()
