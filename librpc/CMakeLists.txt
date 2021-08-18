include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(mdtried
        src/rpc_constants.cc
        src/rpc_service.cc
        src/rpc_types.cc
        src/confluo_server.cc
        src/rpc_thread_factory.cc
        src/rpc_handler_registry.cc)
target_link_libraries(mdtried ${MDTRIE_LIBS} ${RPC_LIBS})
add_dependencies(mdtried thrift)

add_library(rpcclient STATIC
        rpc/rpc_constants.h
        src/rpc_constants.cc
        rpc/rpc_thread_factory.h
        src/rpc_thread_factory.cc
        rpc/rpc_handler_registry.h
        src/rpc_handler_registry.cc
        rpc/rpc_server.h
        src/rpc_server.cc
        rpc/rpc_service.h
        rpc/rpc_service.tcc
        src/rpc_service.cc
        rpc/rpc_types.h
        src/rpc_types.cc
        rpc/rpc_client.h
        src/rpc_client.cc
        rpc/rpc_alert_stream.h
        src/rpc_alert_stream.cc
        rpc/rpc_record_batch_builder.h
        src/rpc_record_batch_builder.cc
        rpc/rpc_type_conversions.h
        src/rpc_type_conversions.cc
        rpc/rpc_record_stream.h
        src/rpc_record_stream.cc
        rpc/rpc_configuration_params.h
        rpc/rpc_defaults.h)
target_link_libraries(mdtrieclient ${RPC_LIBS})
add_dependencies(mdtrieclient thrift)
