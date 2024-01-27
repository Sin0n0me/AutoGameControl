
# Projectの設定
cmake_minimum_required(VERSION 3.8)
project(EmulateForLinux)

# 対応するC++のバージョン指定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# デフォルトのビルドタイプの設定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# パッケージの追加
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)

# ソースファイルの追加
set(SOURCES
    ../AutoGamePadOperatorForLinux.cpp
    ../AutoKeyboardOperatorForLinux.cpp
    ../AutoMouseOperatorForLinux.cpp
    ../main.cpp
    ../AutoGamePadOperatorForLinux.h
    ../AutoKeyboardOperatorForLinux.h
    ../AutoMouseOperatorForLinux.h
    ../../Common/AutoOperator.cpp
    ../../Common/CommandSeparater.cpp
    ../../Common/Common.cpp
    ../../Common/AutoOperator.hpp
    ../../Common/CommandSeparater.hpp
    ../../Common/Common.hpp
    ../../Common/Hash.hpp
    ../../Common/IntAutoGamePadOperator.hpp
    ../../Common/IntAutoKeyboardOperator.hpp
    ../../Common/IntAutoMouseOperator.hpp
    ../../Common/IntAutoOperator.hpp
)

# 実行ファイルのターゲットを定義
add_executable(${PROJECT_NAME} ${SOURCES})

# パッケージの追加
target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES} ${X11_XTest_LIB} -pthread)
target_include_directories(${PROJECT_NAME} PRIVATE ..)

