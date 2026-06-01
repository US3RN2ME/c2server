add_library(c2server_platform_options INTERFACE)

target_compile_definitions(
   c2server_platform_options
   INTERFACE $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0A00;NOMINMAX;BOOST_ASIO_DISABLE_WORKING_EXPRESSION_SFINAE>)

target_compile_options(c2server_platform_options INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/bigobj;/wd5244>)

target_link_options(c2server_platform_options INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/INCREMENTAL:NO>)
