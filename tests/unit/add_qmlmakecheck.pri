# Do not use CONFIG += testcase that would add a 'make check' because it also
# adds a 'make install' that installs the test cases, which we do not want.
# Instead add a 'make check' manually.

check.target = check
check.commands = "set -e;"
for(TEST, TESTS) {
  check.commands += QML2_IMPORT_PATH=../../../modules:$$QML2_IMPORT_PATH UBUNTU_UI_TOOLKIT_THEMES_PATH=../../../modules qmltestrunner -platform minimal -input $${TEST} -import "../../../modules" -o ../../$${TEST}.xml,xunitxml -o -,txt;
}
check.commands += cd ../../..;
check.commands += qmlplugindump Ubuntu.Components 0.1 modules > plugins.qmltypes;
# Palette gets included in Qt 5.2 qmlplugindump even though it's qml
check.commands += BUILTINS=QQuick,QQml,U1db::,Palette python tests/qmlapicheck.py modules/Ubuntu/Components/qmldir modules/Ubuntu/Components/Colors/UbuntuColors.qml modules/Ubuntu/Components/*/qmldir plugins.qmltypes > components.api.new;
check.commands += diff -Fqml -u components.api components.api.new || exit 1; cd tests/unit
