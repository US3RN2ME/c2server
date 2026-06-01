add_library(c2server_platform_options INTERFACE)
add_library(c2server::platform_options ALIAS c2server_platform_options)
set_target_properties(c2server_platform_options PROPERTIES EXPORT_NAME platform_options)

target_compile_features(c2server_platform_options INTERFACE cxx_std_23)

target_compile_definitions(
   c2server_platform_options
   INTERFACE $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0A00;NOMINMAX;BOOST_ASIO_DISABLE_WORKING_EXPRESSION_SFINAE>)

target_compile_options(c2server_platform_options INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/utf-8;/bigobj;/wd5244>)

target_link_options(c2server_platform_options INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/INCREMENTAL:NO>)

function(c2server_configure_target TARGET_NAME)
   set_target_properties(
      ${TARGET_NAME}
      PROPERTIES CXX_STANDARD 23
                 CXX_STANDARD_REQUIRED ON
                 CXX_EXTENSIONS OFF
                 CXX_SCAN_FOR_MODULES ON
                 CXX_MODULE_STD ON)
endfunction()
