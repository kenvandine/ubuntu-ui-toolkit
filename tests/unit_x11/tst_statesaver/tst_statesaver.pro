include(../test-include.pri)
DEFINES += SRCDIR=\\\"$$PWD/\\\"

SOURCES += \
    tst_statesaver.cpp

OTHER_FILES += \
    SaveArrays.qml \
    SaveSupportedTypes.qml \
    ValidUID.qml \
    InvalidUID.qml \
    ValidGroupProperty.qml \
    InvalidGroupProperty.qml \
    Dynamic.qml \
    TwoDynamics.qml \
    DisabledStateSaver.qml \
    SaveObject.qml \
    FirstComponent.qml \
    SecondComponent.qml \
    SameIdsInDifferentComponents.qml \
    SavePropertyGroups.qml \
    ComponentsWithStateSavers.qml \
    CustomControl.qml \
    ComponentsWithStateSaversNoId.qml \
    NestedDynamics.qml \
    RepeaterStates.qml \
    ListViewItems.qml \
    GridViewItems.qml \
    NormalAppClose.qml \
    PermanentStates.qml
