cmake_minimum_required(VERSION 3.14)  # FetchContentにはCMake 3.14以上が必要
project(argparse_cpp_tests)

# C++11標準を使用
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# コンパイラ固有の警告設定
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

# インクルードディレクトリの設定
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# FetchContentを使用してGoogle Testをダウンロード・ビルド
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0  # 安定版のタグを指定
    GIT_SHALLOW    TRUE     # 浅いクローンでダウンロード時間を短縮
)

# Google Testのオプション設定（警告を抑制）
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# Google Testを利用可能にする
FetchContent_MakeAvailable(googletest)

message(STATUS "Google Test will be downloaded and built using FetchContent")

# テストターゲットを有効化
enable_testing()

# 各ユニットテストファイルの設定
set(TEST_SOURCES
    unit/basic_structure_test.cpp
    unit/anyvalue_test.cpp
    unit/type_converter_test.cpp
    unit/argument_test.cpp
    unit/argument_parser_test.cpp
    unit/argument_classification_test.cpp
    unit/argument_group_test.cpp
    unit/namespace_test.cpp
    unit/tokenizer_test.cpp
    unit/parser_test.cpp
    unit/parse_args_test.cpp
    unit/action_test.cpp
    unit/nargs_test.cpp
    unit/help_generator_test.cpp
    unit/help_option_test.cpp
    unit/choices_test.cpp
    unit/required_test.cpp
    unit/error_message_test.cpp
    unit/metavar_test.cpp
    unit/parser_integration_test.cpp
)

# 統合テストファイルの設定
set(INTEGRATION_TEST_SOURCES
    integration/python_compat_test.cpp
)

# テスト実行ファイルを作成（ユニットテスト）
foreach(TEST_SOURCE ${TEST_SOURCES})
    # ファイル名からテスト名を生成
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    # 実行ファイルを作成
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    # Google Testライブラリをリンク
    target_link_libraries(${TEST_NAME} gtest_main gtest)
    
    # pthreadが必要な場合
    find_package(Threads REQUIRED)
    target_link_libraries(${TEST_NAME} Threads::Threads)
    
    # テストを登録
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# テスト実行ファイルを作成（統合テスト）
foreach(TEST_SOURCE ${INTEGRATION_TEST_SOURCES})
    # ファイル名からテスト名を生成
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    # 実行ファイルを作成
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    # Google Testライブラリをリンク
    target_link_libraries(${TEST_NAME} gtest_main gtest)
    
    # pthreadが必要な場合
    find_package(Threads REQUIRED)
    target_link_libraries(${TEST_NAME} Threads::Threads)
    
    # テストを登録
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# すべてのテストをまとめて実行するターゲット
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS ${TEST_SOURCES}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all unit tests"
)

# カバレッジレポート（オプション）
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        
        find_program(GCOV_PATH gcov)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)
        
        if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
            add_custom_target(coverage
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info
                COMMAND ${LCOV_PATH} --list coverage.info
                COMMAND ${GENHTML_PATH} -o coverage coverage.info
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating coverage report"
            )
        endif()
    endif()
endif()

# デバッグ情報の表示
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "Google Test will be automatically downloaded and built")