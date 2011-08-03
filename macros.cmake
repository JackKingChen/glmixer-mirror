
MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
 SET(DEFAULT_ARGS)
 FOREACH(arg_name ${arg_names})   
   SET(${prefix}_${arg_name})
 ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})           
    SET(larg_names ${arg_names})   
    LIST(FIND larg_names "${arg}" is_arg_name)                   
    IF (is_arg_name GREATER -1)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name GREATER -1)
      SET(loption_names ${option_names})   
      LIST(FIND loption_names "${arg}" is_option)           
      IF (is_option GREATER -1)
       SET(${prefix}_${arg} TRUE)
      ELSE (is_option GREATER -1)
       SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option GREATER -1)
    ENDIF (is_arg_name GREATER -1)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)

	
## Copies the specified Qt4 framework into the  bundle

macro(GLMIXER_INSTALL_FRAMEWORKS)
  parse_arguments(INSTALL_FRAMEWORKS
    "NAMES;FROM;TO" ""
    ${ARGN}
  )
  
   set(libfiles "")
   foreach (it ${INSTALL_FRAMEWORKS_NAMES})
     set(libfiles ${libfiles} "${INSTALL_FRAMEWORKS_FROM}/${it}.framework/Versions/Current/${it}")
   endforeach(it)
   
   install(FILES ${libfiles} DESTINATION ${INSTALL_FRAMEWORKS_TO}/)
 
endmacro(GLMIXER_INSTALL_FRAMEWORKS)


## Copies the specified .dylib files into the .app bundle TO folder

macro(GLMIXER_INSTALL_DYLIBS)
   parse_arguments(INSTALL_DYLIBS
     "NAMES;FROM;TO" ""
     ${ARGN}
   )
 
   set(libfiles "")
   foreach (it ${INSTALL_DYLIBS_NAMES})
     set(libfiles ${libfiles} ${INSTALL_DYLIBS_FROM}/${it})
   endforeach(it)
 
   install(FILES ${libfiles} DESTINATION ${INSTALL_DYLIBS_TO})
 
endmacro(GLMIXER_INSTALL_DYLIBS)

 
 