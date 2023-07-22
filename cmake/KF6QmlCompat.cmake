# SPDX-FileCopyrightText: Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

find_program(PERL_EXECUTABLE NAMES perl)
if (NOT PERL_EXECUTABLE)
    return()
endif()

set(_kf6_adapt_qml_basedir ${CMAKE_CURRENT_LIST_DIR})

function(kf6_adapt_qml)
    file(GLOB_RECURSE _qml_files *.qml)
    foreach(_qml_file ${_qml_files})
        message("Converting ${_qml_file}")
        execute_process(COMMAND ${_kf6_adapt_qml_basedir}/kf6qmlconvert.sh ${_qml_file})
    endforeach()
endfunction()
