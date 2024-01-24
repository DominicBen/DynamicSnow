# CMake generated Testfile for 
# Source directory: C:/repos/aburn/usr/modules/DynamicSnow/src/gtest
# Build directory: C:/repos/aburn/usr/modules/DynamicSnow/build/gtest
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[GTest]=] "C:/repos/aburn/usr/modules/DynamicSnow/build/gtest/Debug/GTest.exe")
  set_tests_properties([=[GTest]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/repos/aburn/usr/include/cmake/aftr_module_load_GTest.cmake;56;add_test;C:/repos/aburn/usr/include/cmake/aftr_module_load_GTest.cmake;0;;C:/repos/aburn/usr/modules/DynamicSnow/src/gtest/CMakeLists.txt;13;include;C:/repos/aburn/usr/modules/DynamicSnow/src/gtest/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[GTest]=] "C:/repos/aburn/usr/modules/DynamicSnow/build/gtest/Release/GTest.exe")
  set_tests_properties([=[GTest]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/repos/aburn/usr/include/cmake/aftr_module_load_GTest.cmake;56;add_test;C:/repos/aburn/usr/include/cmake/aftr_module_load_GTest.cmake;0;;C:/repos/aburn/usr/modules/DynamicSnow/src/gtest/CMakeLists.txt;13;include;C:/repos/aburn/usr/modules/DynamicSnow/src/gtest/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[GTest]=] "C:/repos/aburn/usr/modules/DynamicSnow/build/gtest/MinSizeRel/GTest.exe")
  set_tests_properties([=[GTest]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/repos/aburn/usr/include/cmake/aftr_module_load_GTest.cmake;56;add_test;C:/repos/aburn/usr/include/cmake/aftr_module_load_GTest.cmake;0;;C:/repos/aburn/usr/modules/DynamicSnow/src/gtest/CMakeLists.txt;13;include;C:/repos/aburn/usr/modules/DynamicSnow/src/gtest/CMakeLists.txt;0;")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[GTest]=] "C:/repos/aburn/usr/modules/DynamicSnow/build/gtest/RelWithDebInfo/GTest.exe")
  set_tests_properties([=[GTest]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/repos/aburn/usr/include/cmake/aftr_module_load_GTest.cmake;56;add_test;C:/repos/aburn/usr/include/cmake/aftr_module_load_GTest.cmake;0;;C:/repos/aburn/usr/modules/DynamicSnow/src/gtest/CMakeLists.txt;13;include;C:/repos/aburn/usr/modules/DynamicSnow/src/gtest/CMakeLists.txt;0;")
else()
  add_test([=[GTest]=] NOT_AVAILABLE)
endif()
