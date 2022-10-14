build_lib_example(
  NAME vdsr
  SOURCE_FILES vdsr.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libapplications}
    ${libmobility}
    ${libconfig-store}
    ${libwifi}
    ${libdsr}
)
