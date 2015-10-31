add_subdirectory(IR_Transceiver_RaspberryPi)

MAP_ADD_STR(hdrs "${PROJECT_NAME}/" headers)
set(headers ${headers} PARENT_SCOPE)
