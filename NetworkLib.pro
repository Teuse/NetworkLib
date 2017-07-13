# ------------------------------------------------------------------------------
# --- Network 
# ------------------------------------------------------------------------------
!ios: error("NetworkLib: use this .pro file configuration just for ios!")


TEMPLATE = lib

CONFIG += c++14 \
          staticlib

# ------------------------------------------------------------------------------
# --- Collect files
# ------------------------------------------------------------------------------

INCLUDEPATH += $$_PRO_FILE_PWD_ 

HEADERS += Network/Client.h \
           Network/Server.h \
           Network/DataIO.h \
           Network/Common.h

SOURCES += Network/Client.cpp \
           Network/Server.cpp \
           Network/DataIO.cpp
            

# ------------------------------------------------------------------------------
# --- Dependencies
# ------------------------------------------------------------------------------
BOOST_ROOT = $$_PRO_FILE_PWD_/../../../boost-ios/build/boost/1.64.0/ios/prefix
isEmpty(BOOST_ROOT){
    error("NetworkLib: BOOST_ROOT for ios is not defined")
}

INCLUDEPATH += $$BOOST_ROOT/include

