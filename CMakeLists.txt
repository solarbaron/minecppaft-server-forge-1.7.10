cmake_minimum_required(VERSION 3.20)
project(minecppaft-server
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Minecraft 1.7.10 server rewrite in C++"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/server/MinecraftServer.cpp
    src/networking/TcpListener.cpp
    src/networking/Connection.cpp
    src/networking/PacketHandler.cpp
    src/nbt/NBT.cpp
    src/block/Block.cpp
    src/item/Item.cpp
    src/world/Chunk.cpp
    src/world/World.cpp
    src/entity/Entity.cpp
    src/inventory/Inventory.cpp
    src/crafting/Crafting.cpp
    src/mechanics/GameMechanics.cpp
    src/ai/EntityAI.cpp
    src/command/CommandSystem.cpp
    src/forge/JNIBridge.cpp
    src/types/Chat.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Dependencies
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
# Future: OpenSSL for protocol encryption, JNI for Forge bridge
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ZLIB::ZLIB)
