# CMake generated Testfile for 
# Source directory: C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning
# Build directory: C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test([=[simulation]=] "C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/build/Debug/sphere_tests.exe")
  set_tests_properties([=[simulation]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/CMakeLists.txt;33;add_test;C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test([=[simulation]=] "C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/build/Release/sphere_tests.exe")
  set_tests_properties([=[simulation]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/CMakeLists.txt;33;add_test;C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test([=[simulation]=] "C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/build/MinSizeRel/sphere_tests.exe")
  set_tests_properties([=[simulation]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/CMakeLists.txt;33;add_test;C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test([=[simulation]=] "C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/build/RelWithDebInfo/sphere_tests.exe")
  set_tests_properties([=[simulation]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/CMakeLists.txt;33;add_test;C:/Users/Nikor/OneDrive/デスクトップ/sphere_is_learning/CMakeLists.txt;0;")
else()
  add_test([=[simulation]=] NOT_AVAILABLE)
endif()
