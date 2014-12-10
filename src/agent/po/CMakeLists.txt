SET(POFILES
    en.po pl.po
)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(SHAREDIR "${PREFIX}/share")
SET(LOCALEDIR "${SHAREDIR}/locale")
SET(MSGFMT "/usr/bin/msgfmt")

FOREACH(pofile ${POFILES})
    SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
    MESSAGE("PO: ${pofile}")
    GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
    GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
    SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${moFile}
        COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
        DEPENDS ${absPofile}
    )
    INSTALL(FILES ${moFile}
        DESTINATION ${LOCALEDIR}/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
    SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
