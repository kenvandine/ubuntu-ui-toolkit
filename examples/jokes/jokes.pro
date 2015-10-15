TEMPLATE = subdirs

filetypes = qml png svg js jpg qmlproject desktop wav

OTHER_FILES = ""

for(filetype, filetypes) {
  OTHER_FILES += *.$$filetype
}

desktop_files.path = $$[QT_INSTALL_EXAMPLES]/ubuntu-ui-toolkit/examples/jokes
desktop_files.files = jokes.desktop

other_files.path = $$[QT_INSTALL_EXAMPLES]/ubuntu-ui-toolkit/examples/jokes
other_files.files = $$OTHER_FILES

INSTALLS += other_files desktop_files
