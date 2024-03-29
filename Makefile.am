SUBDIRS = src src_call

EXTRA_DIST = I18N.mk openscada.pc.in README* TODO*

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = openscada.pc

if ResourcesAllow
data/LibsDB/*.db data/ModelsDB/AGLKS/*.db data/ModelsDB/Boiler/*.db:
	@for db in `find $$(dirname $@) -name \*.sql`; do \
	    tDb=`dirname $$db`/`basename $$db .sql`.db ; \
	    if test ! -f $$tDb; then sqlite3 -init $$db $$tDb .exit ||: ; fi; \
	done

clean-local:
	rm -f data/LibsDB/*.db ; \
	rm -f data/ModelsDB/AGLKS/*.db ; \
	rm -f data/ModelsDB/Boiler/*.db ;

bin_SCRIPTS = data/openscada_start data/openscada-proj
sysconf_DATA = data/oscada.xml data/oscada_server.xml data/oscada_plc.xml data/oscada_start.xml data/ModelsDB/AGLKS/oscada_AGLKS.xml data/ModelsDB/Boiler/oscada_Boiler.xml

sysicodir = $(datadir)/icons
sysico_DATA = data/openscada.png data/ModelsDB/AGLKS/openscada_AGLKS.png data/ModelsDB/Boiler/openscada_Boiler.png

appsdir = $(datadir)/applications
apps_DATA = data/openscada.desktop data/ModelsDB/AGLKS/openscada_AGLKS.desktop data/ModelsDB/Boiler/openscada_Boiler.desktop

icodir = $(oscd_datadir)/icons
ico_DATA = data/icons/* data/openscada.png data/ModelsDB/AGLKS/*.png data/ModelsDB/Boiler/*.png

libdbdir = $(oscd_datadir)/LibsDB
libdb_DATA = data/LibsDB/*.db

AGLKSdbdir = $(oscd_datadir)/AGLKS
AGLKSdb_DATA = data/ModelsDB/AGLKS/*.db

Boilerdbdir = $(oscd_datadir)/Boiler
Boilerdb_DATA = data/ModelsDB/Boiler/*.db
endif

distSrc: distdir
	rm -rf $(distdir)/data $(distdir)/doc
	tar --exclude=".svn" -cvzf $(PACKAGE)-$(PACKAGE_VERSION).tar.gz $(distdir)
	$(am__remove_distdir)

distSrc-lzma: distdir
	rm -rf $(distdir)/data $(distdir)/doc
	tar --exclude=".svn" --use-compress-program=lzma -cvf $(PACKAGE)-$(PACKAGE_VERSION).tar.lzma $(distdir)
	$(am__remove_distdir)

distSrc-xz: distdir
	rm -rf $(distdir)/data $(distdir)/doc
	tar --exclude=".svn" -J -cvf $(PACKAGE)-$(PACKAGE_VERSION).tar.xz $(distdir)
	$(am__remove_distdir)

distRes:
	tar --exclude=".svn" -cvzf $(PACKAGE)-res-$(PACKAGE_VERSION).tar.gz doc data

distRes-lzma:
	tar --exclude=".svn" --use-compress-program=lzma -cvf $(PACKAGE)-res-$(PACKAGE_VERSION).tar.lzma doc data

distRes-xz:
	tar --exclude=".svn" -J -cvf $(PACKAGE)-res-$(PACKAGE_VERSION).tar.xz doc data
