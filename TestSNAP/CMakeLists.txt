cmake_minimum_required (VERSION 3.12)
project(matvec
  LANGUAGES C CXX
  )

# don't allow in-source builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(STATUS "Warning! Building from the source directory is not allow")
  message(STATUS "Remove 'CMakeCache.txt' and 'CMakeFiles' and build from a separate directory")
  message(ERROR "In-source build")
endif()

SET(MyTarget test_snap)
SET(Thread_MyTarget original_test_snap)
SET(OriginalBlocking original_blocking_test_snap)

SET(sources ${CMAKE_SOURCE_DIR}/src/arrayMDgpu.h ${CMAKE_SOURCE_DIR}/src/memory.cpp
    ${CMAKE_SOURCE_DIR}/src/refdata_2J14_W.h ${CMAKE_SOURCE_DIR}/src/refdata_2J4_W.h
    ${CMAKE_SOURCE_DIR}/src/sna.cpp ${CMAKE_SOURCE_DIR}/src/test_snap.cpp
    ${CMAKE_SOURCE_DIR}/src/arrayMDcpu.h ${CMAKE_SOURCE_DIR}/src/lmptype.h 
    ${CMAKE_SOURCE_DIR}/src/memory.h ${CMAKE_SOURCE_DIR}/src/refdata_2J2_W.h
    ${CMAKE_SOURCE_DIR}/src/refdata_2J8_W.h ${CMAKE_SOURCE_DIR}/src/sna.h
    ${CMAKE_SOURCE_DIR}/src/test_snap.h)

set(threading_sources ${CMAKE_SOURCE_DIR}/src/arrayMDgpu.h ${CMAKE_SOURCE_DIR}/src/memory.cpp
    ${CMAKE_SOURCE_DIR}/src/refdata_2J14_W.h ${CMAKE_SOURCE_DIR}/src/refdata_2J4_W.h
    ${CMAKE_SOURCE_DIR}/src/threading_sna.cpp ${CMAKE_SOURCE_DIR}/src/test_snap.cpp
    ${CMAKE_SOURCE_DIR}/src/arrayMDcpu.h ${CMAKE_SOURCE_DIR}/src/lmptype.h 
    ${CMAKE_SOURCE_DIR}/src/memory.h ${CMAKE_SOURCE_DIR}/src/refdata_2J2_W.h
    ${CMAKE_SOURCE_DIR}/src/refdata_2J8_W.h ${CMAKE_SOURCE_DIR}/src/sna.h
    ${CMAKE_SOURCE_DIR}/src/test_snap.h)

ADD_EXECUTABLE(
  ${MyTarget}
  ${sources}
  )

ADD_EXECUTABLE(
  ${Thread_MyTarget}
  ${threading_sources}
  )

ADD_EXECUTABLE(
  ${OriginalBlocking}
  ${sources}
  )

target_compile_definitions(${MyTarget} PRIVATE REFDATA_TWOJ=${ref_data} -DTASKGRAPH)
target_compile_definitions(${Thread_MyTarget} PRIVATE REFDATA_TWOJ=${ref_data})
target_compile_definitions(${OriginalBlocking} PRIVATE REFDATA_TWOJ=${ref_data})

if(${OPENMP})
  #find_package(OpenMP REQUIRED)
  target_link_libraries(${MyTarget} PRIVATE -fopenmp)
  target_link_libraries(${Thread_MyTarget} PRIVATE -fopenmp)
  target_link_libraries(${OriginalBlocking} PRIVATE -fopenmp)
endif()

if(${OPENMP_TARGET})
  find_package(CUDA REQUIRED)
  #find_package(OpenMP REQUIRED)
  target_compile_definitions(${MyTarget} PRIVATE OPENMP_TARGET)
  target_compile_definitions(${OriginalBlocking} PRIVATE OPENMP_TARGET)
  target_compile_definitions(${Thread_MyTarget} PRIVATE OPENMP_TARGET)

  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  target_compile_options(${MyTarget} PRIVATE -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=${CUDA_TOOLKIT_ROOT_DIR} -I${CUDA_TOOLKIT_ROOT_DIR}/include -ffp-contract=fast -fopenmp -fopenmp-taskgraph)
  target_compile_options(${Thread_MyTarget} PRIVATE -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=${CUDA_TOOLKIT_ROOT_DIR} -I${CUDA_TOOLKIT_ROOT_DIR}/include -ffp-contract=fast -fopenmp)
  target_compile_options(${OriginalBlocking} PRIVATE -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=${CUDA_TOOLKIT_ROOT_DIR} -I${CUDA_TOOLKIT_ROOT_DIR}/include -ffp-contract=fast -fopenmp)
  #target_link_options(${MyTarget} PRIVATE -fopenmp-targets=nvptx64-nvidia-cuda -fopenmp=libiomp5)
  target_link_options(${MyTarget} PRIVATE -fopenmp-targets=nvptx64-nvidia-cuda -fopenmp -fopenmp-taskgraph)
  target_link_options(${Thread_MyTarget} PRIVATE -fopenmp-targets=nvptx64-nvidia-cuda -fopenmp)
  target_link_options(${OriginalBlocking} PRIVATE -fopenmp-targets=nvptx64-nvidia-cuda -fopenmp)
  endif()

  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "XL")
  target_compile_options(${MyTarget} PRIVATE -qsmp=omp:noauto -qoffload)
  target_link_options(${MyTarget} PRIVATE -qsmp=omp:noauto -qoffload)
  endif()
  #target_link_libraries(${MyTarget} PUBLIC OpenMP::OpenMP_CXX)
endif()
