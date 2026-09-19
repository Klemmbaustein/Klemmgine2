# Add a target that runs a powershell script to convert documentation XML files into JSON files the engine can read

file(
	GLOB_RECURSE
	DOC_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/**.xml"
)
execute_process(COMMAND pwsh -h RESULT_VARIABLE PWSH_FAILED OUTPUT_QUIET)

if(PWSH_FAILED)
	message(STATUS "No Powershell installed - Not generating JSON documentation files")
	return()
endif()

set(OutDir "${CMAKE_SOURCE_DIR}/Engine/Editor/Documentation")

set(DOC_FILES)
foreach(DOC_FILE ${DOC_SOURCES})
	get_filename_component(DOC_NAME ${DOC_FILE} NAME_WLE)
	set(DOC_NAME "${OutDir}/${DOC_NAME}.json")
	list(APPEND DOC_FILES ${DOC_NAME})
endforeach()

add_custom_command(
	OUTPUT ${DOC_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND pwsh -NoProfile Build-Docs.ps1 -InPath . -OutPath ${OutDir}
	DEPENDS ${DOC_SOURCES} Build-Docs.ps1
)

make_directory(${OutDir})
add_custom_target(EditorDocumentation DEPENDS ${DOC_FILES})

message(STATUS "Documentation: ${CMAKE_CURRENT_SOURCE_DIR} -> ${OutDir}")
add_dependencies(KlemmgineEditor EditorDocumentation)