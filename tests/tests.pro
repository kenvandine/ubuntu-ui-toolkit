TEMPLATE = subdirs
SUBDIRS += unit unit_x11

autopilot_module.path = /usr/lib/python3/dist-packages/ubuntuuitoolkit
autopilot_module.files = autopilot/ubuntuuitoolkit/*

autopilot2_module.path = /usr/lib/python2.7/dist-packages/ubuntuuitoolkit
autopilot2_module.files = autopilot/ubuntuuitoolkit/*

INSTALLS += autopilot_module autopilot2_module
