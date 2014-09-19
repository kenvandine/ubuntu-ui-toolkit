/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>

#include "uctheme.h"

class tst_Performance : public QObject
{
    Q_OBJECT
    
public:
    tst_Performance() {}

private:
    QQuickView *quickView;
    QQmlEngine *quickEngine;

    QQuickItem *loadDocument(const QString &document)
    {
        quickView->setSource(QUrl::fromLocalFile(document));
        QTest::waitForEvents();

        return quickView->rootObject();
    }

private Q_SLOTS:

    void initTestCase()
    {
        QString modules("../../../modules");
        QVERIFY(QDir(modules).exists());

        quickView = new QQuickView(0);
        quickEngine = quickView->engine();

        quickView->setGeometry(0,0, 240, 320);
        //add modules folder so we have access to the plugin from QML
        QStringList imports = quickEngine->importPathList();
        imports.prepend(QDir(modules).absolutePath());
        quickEngine->setImportPathList(imports);
    }

    void cleanupTestCase()
    {
        delete quickView;
    }

    void benchmark_GridOfComponents_data() {
        QTest::addColumn<QString>("document");
        QTest::addColumn<QUrl>("theme");

        QTest::newRow("grid with Rectangle") << "RectangleGrid.qml" << QUrl();
        QTest::newRow("grid with Text") << "TextGrid.qml" << QUrl();
        QTest::newRow("grid with Label") << "LabelGrid.qml" << QUrl();
        QTest::newRow("grid with UbuntuShape") << "UbuntuShapeGrid.qml" << QUrl();
        QTest::newRow("grid with UbuntuShapePair") << "PairOfUbuntuShapeGrid.qml" << QUrl();
        QTest::newRow("grid with ButtonStyle") << "ButtonStyleGrid.qml" << QUrl();
        QTest::newRow("grid with Button") << "ButtonGrid.qml" << QUrl();
//        QTest::newRow("grid with CheckBoxStyle") << "CheckBoxStyleGrid.qml" << QUrl();
//        QTest::newRow("grid with CheckBox") << "CheckBoxGrid.qml" << QUrl();
//        QTest::newRow("grid with SwitchStyle") << "SwitchStyleGrid.qml" << QUrl();
//        QTest::newRow("grid with Switch") << "SwitchGrid.qml" << QUrl();
//        QTest::newRow("grid with SwitchStyle") << "SwitchStyleGrid.qml" << QUrl();
//        QTest::newRow("grid with Switch") << "SwitchGrid.qml" << QUrl();
        QTest::newRow("grid with SliderStyle") << "SliderStyleGrid.qml" << QUrl();
        QTest::newRow("grid with Slider") << "SliderGrid.qml" << QUrl();

        QTest::newRow("list of standard layout") << "ListOfStandard.qml" << QUrl();
    }

    void benchmark_GridOfComponents()
    {
        QFETCH(QString, document);
        QFETCH(QUrl, theme);

        QQuickItem *root = 0;
        QBENCHMARK {
            root = loadDocument(document);
        }
        if (root)
            delete root;
    }

    void benchmark_import_data()
    {
        QTest::addColumn<QString>("document");

        QTest::newRow("importing Ubuntu.Components") << "TextWithImportGrid.qml";
        QTest::newRow("importing Ubuntu.Components.Popups") << "TextWithImportGrid.qml";
    }

    void benchmark_import()
    {
        QFETCH(QString, document);
        QBENCHMARK {
            loadDocument(document);
        }
    }

};

QTEST_MAIN(tst_Performance)

#include "tst_performance.moc"
