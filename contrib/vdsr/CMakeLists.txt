build_lib(
  LIBNAME vdsr
  SOURCE_FILES
    helper/vdsr-helper.cc
    helper/vdsr-main-helper.cc
    model/vdsr-errorbuff.cc
    model/vdsr-fs-header.cc
    model/vdsr-gratuitous-reply-table.cc
    model/vdsr-maintain-buff.cc
    model/vdsr-network-queue.cc
    model/vdsr-option-header.cc
    model/vdsr-options.cc
    model/vdsr-passive-buff.cc
    model/vdsr-rcache.cc
    model/vdsr-routing.cc
    model/vdsr-rreq-table.cc
    model/vdsr-rsendbuff.cc
  HEADER_FILES
    helper/vdsr-helper.h
    helper/vdsr-main-helper.h
    model/vdsr-errorbuff.h
    model/vdsr-fs-header.h
    model/vdsr-gratuitous-reply-table.h
    model/vdsr-maintain-buff.h
    model/vdsr-network-queue.h
    model/vdsr-option-header.h
    model/vdsr-options.h
    model/vdsr-passive-buff.h
    model/vdsr-rcache.h
    model/vdsr-routing.h
    model/vdsr-rreq-table.h
    model/vdsr-rsendbuff.h
  LIBRARIES_TO_LINK ${libinternet}
                    ${libwifi}
  TEST_SOURCES test/vdsr-test-suite.cc
)
