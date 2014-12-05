/*
 * Copyright 2013 Canonical Ltd.
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
 * Author: Zsombor Egri <zsombor.egri@canonical.com>
 */

#ifndef ALARMSADAPTER_P_H
#define ALARMSADAPTER_P_H

#include "alarmmanager_p_p.h"

#include <qorganizer.h>
#include <qorganizermanager.h>
#include <qorganizertodo.h>

QTORGANIZER_USE_NAMESPACE

/*-----------------------------------------------------------------------------
 * Adaptation layer for Alarms.
 */

class AlarmDataAdapter : public UCAlarmPrivate
{
public:
    AlarmDataAdapter(UCAlarm *qq);
    ~AlarmDataAdapter();

    bool enabled() const;
    bool setEnabled(bool enabled);
    QDateTime date() const;
    bool setDate(const QDateTime &date);
    QString message() const;
    bool setMessage(const QString &message);
    UCAlarm::AlarmType type() const;
    bool setType(UCAlarm::AlarmType type);
    UCAlarm::DaysOfWeek daysOfWeek() const;
    bool setDaysOfWeek(UCAlarm::DaysOfWeek days);
    QUrl sound() const;
    bool setSound(const QUrl &sound);
    QVariant cookie() const;
    UCAlarm::Error checkAlarm();

    void save();
    void cancel();
    void reset();
    bool wait(int msec);
    void completeSave();
    void completeCancel();

// adaptation specific data
    void adjustDowSettings(UCAlarm::AlarmType type, UCAlarm::DaysOfWeek days);
    static QSet<Qt::DayOfWeek> daysToSet(int days);
    static UCAlarm::DaysOfWeek daysFromSet(const QSet<Qt::DayOfWeek> &set);

    inline QOrganizerTodo const &data()
    {
        return event;
    }
    void setData(const QOrganizerTodo &data);

protected:
    QOrganizerTodo event;
    UCAlarm::AlarmType alarmType;
    UCAlarm::DaysOfWeek dow;
    QPointer<QOrganizerAbstractRequest> request;

    void startOperation(UCAlarm::Operation operation, const char *completionSlot);
};

// list of alarms
class AlarmList
{
public:
    AlarmList(){}

    void clear()
    {
        data.clear();
        idHash.clear();
    }
    int count() const
    {
        return data.count();
    }
    const QOrganizerTodo operator[](int index) const
    {
        QPair<QDateTime, QOrganizerItemId> key = data.keys()[index];
        return data.value(key);
    }
    // update event at index, returns the new event index
    int update(int index, const QOrganizerTodo &alarm)
    {
        removeAt(index);
        return insert(alarm);
    }
    // insert an alarm event into the list
    int insert(const QOrganizerTodo &alarm)
    {
        QDateTime dt = AlarmUtils::normalizeDate(alarm.startDateTime());
        idHash.insert(alarm.id(), dt);
        data.insert(QPair<QDateTime, QOrganizerItemId>(dt, alarm.id()), alarm);
        return indexOf(alarm.id());
    }
    // returns the index of the alarm matching the id, -1 on error
    int indexOf(const QOrganizerItemId &id)
    {
        QDateTime dt = idHash.value(id);
        QPair<QDateTime, QOrganizerItemId> key(dt, id);
        return data.keys().indexOf(key);
    }
    // remove alarm at index
    void removeAt(int index)
    {
        QPair<QDateTime, QOrganizerItemId> key = data.keys()[index];
        data.remove(key);
        idHash.remove(key.second);
    }

private:
    // ordered map by occurrence date + event id, ascending
    QMap< QPair<QDateTime, QOrganizerItemId>, QOrganizerTodo> data;
    // alarms ordered based on even id
    QHash<QOrganizerItemId, QDateTime> idHash;
};

class AlarmsAdapter : public QObject, public AlarmManagerPrivate
{
    Q_OBJECT
public:
    enum Error {
        FetchedEventEmpty = UCAlarm::AdaptationError + 1, // 101
        UnhandledEventType, // 102
        UnhandledRequest,   // 103
        OrganizerError      // 104 onwards
    };

    AlarmsAdapter(AlarmManager *qq);
    virtual ~AlarmsAdapter();

    static AlarmsAdapter* get(AlarmManager *instance = 0) {
        return static_cast<AlarmsAdapter*>(AlarmManagerPrivate::get(instance));
    }

    QOrganizerManager *manager;
    QOrganizerCollection collection;

    void init();
    int alarmCount();
    void getAlarmAt(const UCAlarm &alarm, int index) const;
    bool findAlarm(const UCAlarm &alarm, const QVariant &cookie) const;
    void adjustAlarmOccurrence(AlarmDataAdapter &alarm);

    void loadAlarms();
    void saveAlarms();

    bool verifyChange(UCAlarm *alarm, AlarmManager::Change change, const QVariant &value);
    UCAlarmPrivate *createAlarmData(UCAlarm *alarm);

    void insertAlarm(const QOrganizerItemId &id);
    void updateAlarm(const QOrganizerItemId &id);
    void removeAlarm(const QOrganizerItemId &id);

private Q_SLOTS:
    void completeFetchAlarms();
    bool fetchAlarms();
    void alarmOperation(QList<QPair<QOrganizerItemId,QOrganizerManager::Operation> >);

protected:
    QPointer<QOrganizerItemFetchRequest> fetchRequest;
    AlarmList alarmList;
    QOrganizerTodo todoItem(const QOrganizerItemId &id);
};

#endif // ALARMSADAPTER_P_H
