set(POSTPROCESS_SOURCES
  "main.cpp"
)

add_executable(trax-postprocess ${POSTPROCESS_SOURCES})

set_target_properties(trax-postprocess PROPERTIES
  OUTPUT_NAME "trax-postprocess"
  FOLDER "Applications"
  VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:trax-postprocess>"
)

arches_set_output_dirs(trax-postprocess)

target_compile_features(trax-postprocess PRIVATE cxx_std_20)

target_compile_options(trax-postprocess
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
    $<$<AND:$<PLATFORM_ID:Linux>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-march=native>
)

target_include_directories(trax-postprocess
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src/trax-kernel"
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(trax-postprocess
  PRIVATE
    Threads::Threads
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${POSTPROCESS_SOURCES})
