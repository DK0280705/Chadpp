if (NOT PQXX_INCLUDE_DIRS OR NOT PQXX_LIBRARIES)
  find_package(PostgreSQL REQUIRED)
  if (PostgreSQL_FOUND)
    file(TO_CMAKE_PATH "$ENV{PQXX_DIR}" _PQXX_DIR)
    find_library(PQXX_LIBRARY
      NAMES libpqxx pqxx
      PATHS
        ${_PQXX_DIR}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /usr/local/pgsql/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /usr/local/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        ${_PQXX_DIR}/lib
        ${_PQXX_DIR}
        ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/bin
        /usr/local/pgsql/lib
        /usr/local/lib
        /usr/lib
    )
    find_path(PQXX_HEADER_PATH
      NAMES pqxx/pqxx
      PATHS
        ${_PQXX_DIR}/include
        ${_PQXX_DIR}
        ${CMAKE_INSTALL_PREFIX}/include
        /usr/local/pgsql/include
        /usr/local/include
        /usr/include
    )
  endif (PostgreSQL_FOUND)

  if (PQXX_HEADER_PATH AND PQXX_LIBRARY)
    set(PQXX_FOUND TRUE)
    message(STATUS "PQXX Found")
    set(PQXX_INCLUDE_DIRS "${PQXX_HEADER_PATH};${PostgreSQL_INCLUDE_DIRECTORIES}")
    set(PQXX_LIBRARIES "${PQXX_LIBRARY};${PostgreSQL_LIBRARIES}")
  else (PQXX_HEADER_PATH AND PQXX_LIBRARY AND PQXX_FIND_REQUIRED)
    message(STATUS "Could NOT find pqxx development files")
  endif (PQXX_HEADER_PATH AND PQXX_LIBRARY)

endif (NOT PQXX_INCLUDE_DIRS OR NOT PQXX_LIBRARIES)