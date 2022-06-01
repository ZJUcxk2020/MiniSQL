cmake_minimum_required(VERSION 3.19)
project(totalv2)

set(CMAKE_CXX_STANDARD 14)

add_executable(totalv2 main.cpp
        Address.h API.cpp API.h Attribute.h
        Block.h BPlusTree.h BPlusTree.cpp Buffer.h BufferManager.cpp BufferManager.h
        CatalogManager.h CatalogManager.cpp Condition.h
        Index.h IndexManager.h Interpreter.cpp IndexManager.cpp Interpreter.h
        Node.h
        Response.h
        RecordManager.cpp
        RecordManager.h
        Table.h TableRow.h)