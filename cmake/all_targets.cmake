# EX USAGE:
#
# include(all_targets)
# get_all_targets(MY_TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
# message(STATUS "MY_TARGETS: ${MY_TARGETS}")
#
# foreach(target ${MY_TARGETS})
# ...do something
# endforeach()

function(get_all_targets var startdir)
    set(targets)
    get_all_targets_recursive(targets ${startdir})
    set(${var} ${targets} PARENT_SCOPE)
endfunction()

macro(get_all_targets_recursive targets dir)
    get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirectories})
        get_all_targets_recursive(${targets} ${subdir})
    endforeach()

    get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${targets} ${current_targets})
endmacro()
