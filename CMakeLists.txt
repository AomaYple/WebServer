cmake_minimum_required(VERSION 3.26.4)

project(WebServer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_executable(${PROJECT_NAME} "")

target_sources(${PROJECT_NAME} PRIVATE
        main.cpp
        src/BufferRIng.cpp
        src/Completion.cpp
        src/Client.cpp
        src/Event.cpp
        src/Http.cpp
        src/Log.cpp
        src/Ring.cpp
        src/Server.cpp
        src/Submission.cpp
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        include
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        uring
        )
