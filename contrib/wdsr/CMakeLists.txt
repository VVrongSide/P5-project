build_lib(
  LIBNAME wdsr
  SOURCE_FILES
    helper/wdsr-helper.cc
    helper/wdsr-main-helper.cc
    model/wdsr-errorbuff.cc
    model/wdsr-fs-header.cc
    model/wdsr-gratuitous-reply-table.cc
    model/wdsr-maintain-buff.cc
    model/wdsr-network-queue.cc
    model/wdsr-option-header.cc
    model/wdsr-options.cc
    model/wdsr-passive-buff.cc
    model/wdsr-rcache.cc
    model/wdsr-routing.cc
    model/wdsr-rreq-table.cc
    model/wdsr-rsendbuff.cc
  HEADER_FILES
    helper/wdsr-helper.h
    helper/wdsr-main-helper.h
    model/wdsr-errorbuff.h
    model/wdsr-fs-header.h
    model/wdsr-gratuitous-reply-table.h
    model/wdsr-maintain-buff.h
    model/wdsr-network-queue.h
    model/wdsr-option-header.h
    model/wdsr-options.h
    model/wdsr-passive-buff.h
    model/wdsr-rcache.h
    model/wdsr-routing.h
    model/wdsr-rreq-table.h
    model/wdsr-rsendbuff.h
  LIBRARIES_TO_LINK ${libinternet}
                    ${libwifi}
  TEST_SOURCES test/wdsr-test-suite.cc
)
