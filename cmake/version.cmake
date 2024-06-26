file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/../vcpkg.json" GERIUM_VCPKG_JSON)

string(REGEX MATCH "\"version\"[ \t\n\r]*:[ \t\n\r]*\"[^\"]*\"" GERIUM_TMP_VERSION_PAIR ${GERIUM_VCPKG_JSON})
string(REGEX REPLACE "\"version\"[ \t\n\r]*:[ \t\n\r]*\"([^\"]*)\"" "\\1" GERIUM_VERSION ${GERIUM_TMP_VERSION_PAIR})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\1" GERIUM_VERSION_MAJOR ${GERIUM_VERSION})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\2" GERIUM_VERSION_MINOR ${GERIUM_VERSION})
string(REGEX REPLACE "^([0-9]*)\\.([0-9]*)\\.([0-9]*)" "\\3" GERIUM_VERSION_MICRO ${GERIUM_VERSION})

unset(GERIUM_TMP_VERSION_PAIR)
unset(GERIUM_VCPKG_JSON)
