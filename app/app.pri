TEMPLATE = app
QMAKE_LFLAGS += "-Wl,--hash-style=gnu -Wl,--as-needed"

load(configure)
qtCompileTest(libhomescreen)

CONFIG += link_pkgconfig
PKGCONFIG += libhomescreen qlibwindowmanager qtappfw

config_libhomescreen {
    CONFIG += link_pkgconfig
    PKGCONFIG += homescreen
    DEFINES += HAVE_LIBHOMESCREEN
}

DESTDIR = $${OUT_PWD}/../package/root/bin
