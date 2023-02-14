
cmake_minimum_required (VERSION 3.2)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include (GNUInstallDirs)
include (${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include ("${CMAKE_SOURCE_DIR}/cmake/Ronn2Man.cmake")

include_directories (${CMAKE_INSTALL_PREFIX}/include)
link_directories (${CMAKE_INSTALL_PREFIX}/lib)

project (blupimania)
set (PB_VERSION_MAJOR 1)
set (PB_VERSION_MINOR 14)
set (PB_VERSION_PATCH 2)
set (PB_VERSION_EXTRA "")
set (PB_PRODUCT_NAME "Blupimania")
set (PB_PACKAGE_NAME "blupimania")
set (PB_EXEC         "blupimania")
set (PB_ICON_REF     "blupi")
set (PB_DESCRIPTION "Blupimania - A delirious spell-binding game")

option (PB_HTTP_VERSION_CHECK "Run a version check over HTTP (with CURL)" OFF)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.h.in"
  "${PROJECT_BINARY_DIR}/include/config.h"
)

include_directories ("${PROJECT_BINARY_DIR}/include")

if (BUILD_JS)
  file (GLOB sources src/*.c src/*.h src/json/* src/platform/*_js.*)
else ()
  file (GLOB sources src/*.c src/*.h src/json/* src/platform/*_sdl.*)
endif ()

file (GLOB_RECURSE po resources/po/*.po)

if (APPIMAGE_APPRUN_PROGRAM AND APPIMAGE_ASSISTANT_PROGRAM)
  set (USE_APPIMAGE ON)
endif ()

if (NOT USE_APPIMAGE)
  configure_file (
    "${PROJECT_SOURCE_DIR}/resources/linux/application.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}.desktop"
    @ONLY
  )
endif ()

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
#file (COPY resources/sound DESTINATION share/blupimania)
#file (COPY resources/music DESTINATION share/blupimania)
#file (COPY LICENSE.all DESTINATION share/doc/blupimania)
#file (COPY COPYING DESTINATION share/doc/blupimania)
#file (RENAME "${CMAKE_BINARY_DIR}/share/doc/blupimania/LICENSE.all"
#             "${CMAKE_BINARY_DIR}/share/doc/blupimania/copyright")

if (BUILD_LINUX)
  #file (COPY resources/icon/hicolor DESTINATION share/icons)
  #file (
  #  COPY resources/linux/${PB_PACKAGE_NAME}.appdata.xml
  #  DESTINATION share/metainfo
  #)
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

find_package (Intl REQUIRED)
include_directories (${Intl_INCLUDE_DIRS})

find_package (Iconv REQUIRED)
include_directories (${Iconv_INCLUDE_DIRS})

find_package (Argagg REQUIRED)
include_directories (${Argagg_INCLUDE_DIRS})

find_package (SDLKitchensink REQUIRED)
include_directories (${SDLKitchensink_INCLUDE_DIRS})

find_package (PkgConfig REQUIRED)
if (NOT BUILD_JS)
  pkg_search_module (SDL2 REQUIRED sdl2)
  include_directories (${SDL2_INCLUDE_DIRS})
  set (blupimania_DEPS ${blupimania_DEPS} ${SDL2_STATIC_LIBRARIES})
  pkg_search_module (SDL2_IMAGE REQUIRED SDL2_image)
  include_directories (${SDL2_IMAGE_INCLUDE_DIRS})
  set (blupimania_DEPS ${blupimania_DEPS} ${SDL2_IMAGE_STATIC_LIBRARIES})
  pkg_search_module (SDL2_MIXER REQUIRED SDL2_mixer)
  include_directories (${SDL2_MIXER_INCLUDE_DIRS})
  set (blupimania_DEPS ${blupimania_DEPS} ${SDL2_MIXER_STATIC_LIBRARIES})
endif ()

if (${PB_HTTP_VERSION_CHECK})
  pkg_search_module (CURL REQUIRED libcurl)
  add_definitions (-DUSE_CURL)
endif ()

if ("${STATIC_BUILD}")
  # Static dependencies for SDL_kitchensink
  pkg_search_module (AVCODEC REQUIRED libavcodec)
  pkg_search_module (AVFORMAT REQUIRED libavformat)
  pkg_search_module (AVUTIL REQUIRED libavutil)
  pkg_search_module (SWSCALE REQUIRED libswscale)
  pkg_search_module (SWRESAMPLE REQUIRED libswresample)

  if (NOT BUILD_JS)
    pkg_search_module (PNG REQUIRED libpng)
    set (blupimania_DEPS ${blupimania_DEPS} ${PNG_STATIC_LIBRARIES})

    # Static dependencies for SDL_mixer
    pkg_search_module (VORBIS REQUIRED vorbisfile)
    set (blupimania_DEPS ${blupimania_DEPS} ${VORBIS_STATIC_LIBRARIES})

    if (BUILD_LINUX)
      pkg_search_module (ALSA REQUIRED alsa)
      pkg_search_module (PULSE REQUIRED libpulse)
    endif ()

    set (blupimania_DEPS ${blupimania_DEPS}
      ${Intl_LIBRARIES}
      ${Iconv_LIBRARIES}
    )
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
  set (blupimania_DEPS ${blupimania_DEPS}
    ${SDLKitchensink_STATIC_LIBRARIES}
    ${AVCODEC_STATIC_LIBRARIES}
    ${AVFORMAT_STATIC_LIBRARIES}
    ${AVUTIL_STATIC_LIBRARIES}
    ${SWSCALE_STATIC_LIBRARIES}
    ${SWRESAMPLE_STATIC_LIBRARIES}
  )

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
else ()
  set (blupimania_DEPS
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${CURL_LIBRARIES}
    ${SDLKitchensink_LIBRARIES}
    pthread
  )

  if (${PB_HTTP_VERSION_CHECK})
    list (APPEND blupimania_DEPS
      ${CURL_LIBRARIES}
    )
  endif ()
endif ()

target_link_libraries (${blupimania_OUTPUT} PUBLIC ${blupimania_DEPS})

##########
## GetText
##########

find_package (Gettext)

set (_potFile ${CMAKE_CURRENT_SOURCE_DIR}/resources/po/${PROJECT_NAME}.pot)

add_custom_command (OUTPUT ${_potFile}
  COMMAND xgettext --no-location --keyword=translate -o ${_potFile} ${sources}
  DEPENDS ${sources}
  COMMENT "Extract translatable messages to ${_potFile}"
)

add_custom_target (pot_file ALL ${_all}
  DEPENDS ${_potFile}
)

gettext_create_translations (${_potFile} ALL ${po})

## Put mo files to appropriate directory
foreach (file ${_gmoFiles})
  get_filename_component (_lang ${file} NAME_WE)
  set (_out "share/locale/${_lang}/LC_MESSAGES")

  add_custom_command (OUTPUT ${_out}/blupimania.mo
    COMMAND ${CMAKE_COMMAND} -E copy ${file} ${_out}/blupimania.mo
    DEPENDS translations ${file}
  )

  add_custom_target ("po-${_lang}" ALL ${_all}
    DEPENDS ${_out}/blupimania.mo
  )

  add_dependencies (${blupimania_OUTPUT} "po-${_lang}")
endforeach (file)

#########
# manpage
#########

#add_manpage_target ()
#manpage(${PROJECT_NAME} 6)

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
  #install (FILES LICENSE.all DESTINATION share/doc/blupimania RENAME copyright)
  #install (FILES COPYING DESTINATION share/doc/blupimania)

  if (BUILD_LINUX)
    #install (DIRECTORY resources/icon/hicolor DESTINATION share/icons)
    #install (
    #  FILES resources/linux/${PB_PACKAGE_NAME}.appdata.xml
    #  DESTINATION share/metainfo
    #)
  endif ()

  if (NOT USE_APPIMAGE)
    install (
      FILES ${CMAKE_CURRENT_BINARY_DIR}/${PB_PACKAGE_NAME}.desktop
      DESTINATION share/applications
    )
  endif ()

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
  # set (CPACK_STRIP_FILES TRUE)
  set (CPACK_PACKAGE_NAME ${PB_PACKAGE_NAME})
  set (CPACK_PACKAGE_VENDOR "blupi.org")
  set (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PB_DESCRIPTION})
  set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  # set (CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README-user.md")
  set (CPACK_PACKAGE_FILE_NAME ${PB_PACKAGE_NAME})
  set (CPACK_PACKAGE_VERSION "${PB_VERSION_MAJOR}.${PB_VERSION_MINOR}.${PB_VERSION_PATCH}${PB_VERSION_EXTRA}")
  set (CPACK_PACKAGE_INSTALL_DIRECTORY ${PB_PRODUCT_NAME})
  set (CPACK_PACKAGE_EXECUTABLES "blupimania;Blupimania")

  if (USE_APPIMAGE)
    include (LinuxAppImageBuild)
    set (CMAKE_PACKAGED_OUTPUT_PREFIX ${CMAKE_INSTALL_PREFIX})
    APPIMAGE_PACKAGE (blupimania ${PB_PACKAGE_NAME} ${PB_PRODUCT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/resources/linux" "${CMAKE_BINARY_DIR}/share" "" "" ${PB_ICON_REF} SIGN_APP)
  elseif (MINGW)
    set (INSTALLER_FILE_NAME "${PB_PACKAGE_NAME}-${PB_VERSION_MAJOR}.${PB_VERSION_MINOR}.${PB_VERSION_PATCH}${PB_VERSION_EXTRA}")
    set (CPACK_PACKAGE_FILE_NAME "${INSTALLER_FILE_NAME}")
    set (CPACK_GENERATOR "NSIS64")
    set (CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\resources\\\\nsis\\\\installer.bmp")
    set (CPACK_NSIS_COMPRESSOR "/SOLID lzma")
    set (CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\blupimania.exe")
    set (CPACK_NSIS_URL_INFO_ABOUT "http://www.blupi.org")
    set (CPACK_NSIS_MUI_FINISHPAGE_RUN "blupimania")
    if (SIGN_APP)
      set (CPACK_NSIS_SIGN_UNINSTALLER "sign -a -fd sha1 -t http://time.certum.pl -v")
    endif ()

    include (CPack)

    find_program (SIGNTOOL_EXECUTABLE signtool)
    if (SIGN_APP AND SIGNTOOL_EXECUTABLE)
      add_custom_command (TARGET blupimania
                          POST_BUILD
                          COMMAND ${SIGNTOOL_EXECUTABLE}
                            sign -a -fd sha1 -t http://time.certum.pl -v
                            "${CMAKE_BINARY_DIR}/blupimania.exe")
    endif ()

    add_custom_command (TARGET blupimania
                        POST_BUILD
                        COMMAND make package)

    if (SIGN_APP AND SIGNTOOL_EXECUTABLE)
      add_custom_command (TARGET blupimania
                          POST_BUILD
                          COMMAND ${SIGNTOOL_EXECUTABLE}
                            sign -a -fd sha1 -t http://time.certum.pl
                            "${CMAKE_BINARY_DIR}/${INSTALLER_FILE_NAME}.exe")
    endif ()
  elseif (APPLE)
    set (CPACK_GENERATOR "Bundle")
    set (CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/icon.icns")
    set (CPACK_BUNDLE_NAME ${PB_PRODUCT_NAME})
    set (CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/Info.plist")
    set (CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/Planet Blupi")
    if (SIGN_APP)
      set (CPACK_BUNDLE_APPLE_CERT_APP "Developer ID Application: Mathieu Schroeter")
    endif ()
    set (CPACK_PACKAGE_FILE_NAME "blupimania-${PB_VERSION_MAJOR}.${PB_VERSION_MINOR}.${PB_VERSION_PATCH}${PB_VERSION_EXTRA}")

    set (CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/background.tiff")
    set (CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/resources/darwin/dmgsetup.scpt")

    set (BUNDLE_VERSION "${PB_VERSION_MAJOR}.${PB_VERSION_MINOR}.${PB_VERSION_PATCH}${PB_VERSION_EXTRA}")
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