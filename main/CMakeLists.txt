# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

set(EXTRA_COMPONENT_DIRS 
    ${PROJ_DIR}/components/screen
)

set(INC_DIRS
	${PROJ_DIR}/components
    ${PROJ_DIR}/components/screen
)

idf_component_register(
    SRCS main.c         # list the source files of this component
    INCLUDE_DIRS ${INC_DIRS}       # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
