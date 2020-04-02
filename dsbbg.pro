PROGRAM = dsbbg

isEmpty(PREFIX) {  
	PREFIX="/usr/local"
}

isEmpty(DATADIR) {  
	DATADIR=$${PREFIX}/share/$${PROGRAM}                                    
}                   

TARGET	     = $${PROGRAM}
PATH_FEH     = $${PREFIX}/bin/feh
PATH_BGLIST  = backgrounds
APPSDIR	     = $${PREFIX}/share/applications
INSTALLS     = target desktopfile locales
TRANSLATIONS = locale/$${PROGRAM}_de.ts \
               locale/$${PROGRAM}_fr.ts
TEMPLATE     = app
QT	    += widgets
INCLUDEPATH += . lib lib/dsbcfg src 
DEFINES     += PROGRAM=\\\"$${PROGRAM}\\\" LOCALE_PATH=\\\"$${DATADIR}\\\"
DEFINES	    += PATH_FEH=\\\"$${PATH_FEH}\\\"
DEFINES	    += PATH_BGLIST=\\\"$${PATH_BGLIST}\\\"

QMAKE_POST_LINK = $(STRIP) $(TARGET)
QMAKE_EXTRA_TARGETS += distclean cleanqm
HEADERS += src/model.h \
	   src/mainwin.h \
	   src/list.h \
	   src/bglist.h \
	   lib/dsbcfg/dsbcfg.h
	   
SOURCES += src/main.cpp \
	   src/model.cpp \
	   src/mainwin.cpp \
	   src/list.cpp \
	   src/bglist.cpp \
	   lib/dsbcfg/dsbcfg.c

target.files      = $${PROGRAM}         
target.path       = $${PREFIX}/bin      

desktopfile.path  = $${APPSDIR}         
desktopfile.files = $${PROGRAM}.desktop 

locales.path = $${DATADIR}

qtPrepareTool(LRELEASE, lrelease)
for(a, TRANSLATIONS) {
	cmd = $$LRELEASE $${a}
	system($$cmd)
}
locales.files += locale/*.qm

cleanqm.commands  = rm -f $${locales.files}
distclean.depends = cleanqm
