/*
 * Copyright 2012-2013 Canonical Ltd.
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
 * Author: Christian Dywan <christian.dywan@canonical.com>
 */

#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtCore/QStandardPaths>
#include <QtCore/QProcessEnvironment>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <QtCore/QCoreApplication>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>
#include <QtCore/QThread>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtTest/qtest_gui.h>

namespace C {
#include <libintl.h>
}

#include "ucunits.h"
#include "i18n.h"

class tst_I18n : public QObject
{
    Q_OBJECT

private:
    QQuickView *view;

public:
    tst_I18n() :
        view(0)
    {
    }

    QQuickItem *loadTest(const QString &document)
    {
        // load the document
        view->setSource(QUrl::fromLocalFile(document));
        QTest::waitForEvents();

        return view->rootObject();
    }

    QQuickItem *testItem(QQuickItem *that, const QString &identifier)
    {
        if (that->property(identifier.toLocal8Bit()).isValid())
            return that->property(identifier.toLocal8Bit()).value<QQuickItem*>();

        QList<QQuickItem*> children = that->findChildren<QQuickItem*>(identifier);
        return (children.count() > 0) ? children[0] : 0;
    }

private Q_SLOTS:

    void initTestCase()
    {
        // Set test locale folder in the environment
        // Using setenv because QProcessEnvironment ignores changes
        QString testDataFolder(QCoreApplication::applicationDirPath());
        setenv("XDG_DATA_HOME", testDataFolder.toUtf8(), 1);

        // Verify that we set it correctly
        QString doubleCheckLocalePath(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
            "locale", QStandardPaths::LocateDirectory));
        QCOMPARE(doubleCheckLocalePath, testDataFolder + "/locale");
        QVERIFY(QFileInfo(testDataFolder + "/locale/en/LC_MESSAGES/localizedApp.mo").exists());

        QString modules("../../../modules");
        QVERIFY(QDir(modules).exists());

        view = new QQuickView;
        QQmlEngine *quickEngine = view->engine();

        view->setGeometry(0,0, UCUnits::instance().gu(40), UCUnits::instance().gu(30));
        //add modules folder so we have access to the plugin from QML
        QStringList imports = quickEngine->importPathList();
        imports.prepend(QDir(modules).absolutePath());
        quickEngine->setImportPathList(imports);
    }

    void cleanupTestCase()
    {
        delete view;
    }

    void testCase_LocalizedApp()
    {
        UbuntuI18n* i18n = &UbuntuI18n::instance();
        // By default no domain is set
        QCOMPARE(i18n->domain(), QString(""));

        // Start out with no localization
        i18n->setLanguage("C");
        // Load the app which should pick up the locale we prepared
        QQuickItem *root = loadTest("src/LocalizedApp.qml");
        QVERIFY(root);
        QQuickItem *mainView = root;
        // Sanity checks to avoid confusion
        QString applicationName(mainView->property("applicationName").toString());
        QCOMPARE(applicationName, QString("localizedApp"));
        QCOMPARE(applicationName, QCoreApplication::applicationName());
        QCOMPARE(applicationName, i18n->domain());

        // Was the locale folder detected and set?
        QString boundDomain(C::bindtextdomain(i18n->domain().toUtf8(), ((const char*)0)));
        QString expectedLocalePath(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
            "locale", QStandardPaths::LocateDirectory));
        QCOMPARE(boundDomain, expectedLocalePath);
        // Is the domain gettext uses correct?
        QString gettextDomain(C::textdomain(((const char*)0)));
        QCOMPARE(gettextDomain, i18n->domain());
        // Is the compiled en_US message catalog in the right location?
        QString messageCatalog(boundDomain + "/en/LC_MESSAGES/localizedApp.mo");
        QVERIFY(QFileInfo(messageCatalog).exists());

        /* For manual testing one can do something like
            env LANGUAGE=en_US TEXTDOMAINDIR=./tests/unit/tst_i18n/locale/ gettext localizedApp 'Welcome'
        */

        // Check if system has en_US locale, otherwise gettext won't work
        QProcess localeA;
        localeA.start("locale -a");
        QVERIFY(localeA.waitForFinished());
        QVERIFY(QString(localeA.readAll()).split("\n").contains("en_US.utf8"));

        i18n->setLanguage("en_US.utf8");
        QSignalSpy spy(i18n, SIGNAL(languageChanged()));
        spy.wait();

        // Inspect translated strings in QML
        QQuickItem* page(testItem(mainView, "page"));
        QVERIFY(page);
        QCOMPARE(page->property("title").toString(), QString("Greets"));
        QQuickItem* button(testItem(page, "button"));
        QVERIFY(button);
        QCOMPARE(button->property("text").toString(), QString("Count the clicks"));

        // Translate in C++
        QCOMPARE(i18n->dtr(i18n->domain(), QString("Welcome")), QString("Greets"));
        QCOMPARE(i18n->tr(QString("Count the kilometres")), QString("Count the clicks"));
    }
};

// The C++ equivalent of QTEST_MAIN(tst_I18n) with added initialization
int main(int argc, char *argv[])
{
    // LC_ALL would fail the test case; it must be unset before execution
    unsetenv("LC_ALL");

    QGuiApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    tst_I18n* testObject = new tst_I18n();
    return QTest::qExec(static_cast<QObject*>(testObject), argc, argv);
}

#include "tst_i18n.moc"
