build_lib_example(
  NAME wdsr
  SOURCE_FILES wdsr.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libapplications}
    ${libmobility}
    ${libconfig-store}
    ${libwifi}
    ${libwdsr}
)
