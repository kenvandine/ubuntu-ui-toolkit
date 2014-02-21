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

#include "ullayouts.h"
#include "ullayouts_p.h"
#include "ulitemlayout.h"
#include "ulconditionallayout.h"
#include "propertychanges_p.h"
#include <QtQml/QQmlInfo>

ULLayoutsPrivate::ULLayoutsPrivate(ULLayouts *qq)
    : QQmlIncubator(Asynchronous)
    , q_ptr(qq)
    , currentLayoutItem(0)
    , previousLayoutItem(0)
    , currentLayoutIndex(-1)
    , ready(false)
{
}


/******************************************************************************
 * QQmlListProperty functions
 */
void ULLayoutsPrivate::append_layout(QQmlListProperty<ULConditionalLayout> *list, ULConditionalLayout *layout)
{
    ULLayouts *_this = static_cast<ULLayouts*>(list->object);
    if (layout) {
        layout->setParent(_this);
        _this->d_ptr->layouts.append(layout);
    }
}

int ULLayoutsPrivate::count_layouts(QQmlListProperty<ULConditionalLayout> *list)
{
    ULLayouts *_this = static_cast<ULLayouts*>(list->object);
    return _this->d_ptr->layouts.count();
}

ULConditionalLayout *ULLayoutsPrivate::at_layout(QQmlListProperty<ULConditionalLayout> *list, int index)
{
    ULLayouts *_this = static_cast<ULLayouts*>(list->object);
    return _this->d_ptr->layouts.at(index);
}

void ULLayoutsPrivate::clear_layouts(QQmlListProperty<ULConditionalLayout> *list)
{
    ULLayouts *_this = static_cast<ULLayouts*>(list->object);
    _this->d_ptr->layouts.clear();
}


/******************************************************************************
 * ULLayoutsPrivate also acts as QQmlIncubator for the dynamically created layouts.
 * QQmlIncubator stuff
 */
void ULLayoutsPrivate::setInitialState(QObject *object)
{
    Q_Q(ULLayouts);
    // object context's parent is the creation context; link it to the object so we
    // delete them together
    qmlContext(object)->parentContext()->setParent(object);
    // set parent
    object->setParent(q);
    QQuickItem *item = static_cast<QQuickItem*>(object);
    // set disabled and invisible, and set its parent as last action
    item->setVisible(false);
    item->setEnabled(false);
}

/*
 * Called upon QQmlComponent::create() to notify the status of the component
 * creation.
 */
void ULLayoutsPrivate::statusChanged(Status status)
{
    Q_Q(ULLayouts);
    if (status == Ready) {
        // complete layouting
        previousLayoutItem = currentLayoutItem;

        // reset the layout
        currentLayoutItem = qobject_cast<QQuickItem*>(object());
        Q_ASSERT(currentLayoutItem);

        // hide all non-laid out items first
        hideExcludedItems();

        //reparent components to be laid out
        reparentItems();
        // set parent item, then enable and show layout
        changes.addChange(new ParentChange(currentLayoutItem, q, false));
        itemActivate(currentLayoutItem, true);
        // apply changes
        changes.apply();
        // clear previous layout
        delete previousLayoutItem;
        previousLayoutItem = 0;

        Q_EMIT q->currentLayoutChanged();
    } else if (status == Error) {
        Q_Q(ULLayouts);
        error(q, errors());
    }
}

void ULLayoutsPrivate::hideExcludedItems()
{
    for (int i = 0; i < excludedFromLayout.count(); i++) {
        itemActivate(excludedFromLayout[i], false);
    }
}

/*
 * Re-parent items to the new layout.
 */
void ULLayoutsPrivate::reparentItems()
{
    // create copy of items list, to keep track of which ones we change
    LaidOutItemsMap unusedItems = itemsToLayout;

    // iterate through the Layout definition to find containers - those Items with
    // ConditionalLayout.items set
    QList<QQuickItem*> items = currentLayoutItem->findChildren<QQuickItem*>();
    // add the root item as that also can be the container
    items.prepend(currentLayoutItem);

    Q_FOREACH(QQuickItem *container, items) {
        // check whether we have ItemLayout declared
        ULItemLayout *itemLayout = qobject_cast<ULItemLayout*>(container);
        if (itemLayout) {
            reparentToItemLayout(unusedItems, itemLayout);
        }
    }

    // hide the rest of the unused ones
    LaidOutItemsMapIterator i(unusedItems);
    while (i.hasNext()) {
        i.next();
        itemActivate(i.value(), false);
    }
}

/*
 * Re-parent to ItemLayout.
 */
void ULLayoutsPrivate::reparentToItemLayout(LaidOutItemsMap &map, ULItemLayout *fragment)
{
    QString itemName = fragment->item();
    if (itemName.isEmpty()) {
        warning(fragment, "item not specified for ItemLayout");
        return;
    }

    QQuickItem *item = map.value(itemName);
    if (!item) {
        warning(fragment, "item \"" + itemName
                          + "\" not specified or has been specified for layout by "
                             " more than one active ItemLayout");
        return;
    }

    // the component fills the parent
    changes.addChange(new ParentChange(item, fragment, true));
    changes.addChange(new ItemStackBackup(item, currentLayoutItem, previousLayoutItem));
    changes.addChange(new AnchorChange(item, "fill", fragment));
    changes.addChange(new PropertyChange(item, "anchors.margins", 0));
    changes.addChange(new PropertyChange(item, "anchors.leftMargin", 0));
    changes.addChange(new PropertyChange(item, "anchors.topMargin", 0));
    changes.addChange(new PropertyChange(item, "anchors.rightMargin", 0));
    changes.addChange(new PropertyChange(item, "anchors.bottomMargin", 0));
           // backup size
    changes.addChange(new PropertyBackup(item, "width"));
    changes.addChange(new PropertyBackup(item, "height"));
           // break and backup anchors
    changes.addChange(new AnchorBackup(item));

    // remove from unused ones
    map.remove(itemName);
}

void ULLayoutsPrivate::itemActivate(QQuickItem *item, bool activate)
{
    changes.addChange(new PropertyChange(item, "visible", activate))
           .addChange(new PropertyChange(item, "enabled", activate));
}

// remove the deleted item from the excluded ones
void ULLayoutsPrivate::_q_removeExcludedItem(QObject *excludedItem)
{
    excludedFromLayout.removeAll(static_cast<QQuickItem*>(excludedItem));
}

/*
 * Validates the declared conditional layouts by checking whether they have name
 * property set and whether the value set is unique, and whether the conditional
 * layout has container defined.
 */
void ULLayoutsPrivate::validateConditionalLayouts()
{
    Q_Q(ULLayouts);

    QStringList names;
    for (int i = 0; i < layouts.count(); i++) {
        ULConditionalLayout *layout = layouts[i];
        if (!layout) {
            error(q, "Error in layout definitions!");
            continue;
        }

        if (layout->layoutName().isEmpty()) {
            warning(layout, "No name specified for layout. ConditionalLayout cannot be activated without name.");
            continue;
        }
        if (names.contains(layout->layoutName())) {
            warning(layout, "layout name \"" + layout->layoutName()
                            + "\" not unique. Layout may not behave as expected.");
        }

        if (!layout->layout()) {
            error(layout, "no container specified for layout \"" + layout->layoutName() +
                           "\". ConditionalLayout cannot be activated without a container.");
            continue;
        }

    }
}

/*
 * Collect items to be laid out.
 */
void ULLayoutsPrivate::getLaidOutItems()
{
    Q_Q(ULLayouts);

    QList<QQuickItem*> items = q->findChildren<QQuickItem*>();
    for (int i = 0; i < items.count(); i++) {
        QQuickItem *item = items[i];
        ULLayoutsAttached *marker = qobject_cast<ULLayoutsAttached*>(
                    qmlAttachedPropertiesObject<ULLayouts>(item, false));
        if (marker && !marker->item().isEmpty()) {
            itemsToLayout.insert(marker->item(), item);
        } else {
            // the item is not marked to be laid out but one of its parents
            // can be, therefore check
            // check if the item's  parent is included in the layout
            QQuickItem *pl = item->parentItem();
            marker = 0;
            if (!pl && item->parent()) {
                // this may be an item instance assigned to a property
                // like "property var anItem: Item {}"
                // in which case we must get the parent object of it, not the parent item
                pl = qobject_cast<QQuickItem*>(item->parent());
            }
            while (pl) {
                marker = qobject_cast<ULLayoutsAttached*>(
                            qmlAttachedPropertiesObject<ULLayouts>(pl, false));
                if (marker && !marker->item().isEmpty()) {
                    break;
                }
                pl = pl->parentItem();
            }
            if (!marker || (marker && marker->item().isEmpty())) {
                // remember theese so we hide them once we switch away from default layout
                excludedFromLayout << item;
                // and make sure we remove the item from excluded ones in case the item is destroyed
                QObject::connect(item, SIGNAL(destroyed(QObject*)),
                                 q, SLOT(_q_removeExcludedItem(QObject*)));
            }
        }
    }
}

/*
 * Apply layout change. The new layout creation will be completed in statusChange().
 */
void ULLayoutsPrivate::reLayout()
{
    if (!ready || (currentLayoutIndex < 0)) {
        return;
    }
    if (!layouts[currentLayoutIndex]->layout()) {
        return;
    }

    // redo changes
    changes.revert();
    changes.clear();

    // clear the incubator before using it
    clear();
    QQmlComponent *component = layouts[currentLayoutIndex]->layout();
    // create using incubation as it may be created asynchronously,
    // case when the attached properties are not yet enumerated
    Q_Q(ULLayouts);
    QQmlContext *context = new QQmlContext(qmlContext(q), q);
    component->create(*this, context);
}

/*
 * Updates the current layout.
 */
void ULLayoutsPrivate::updateLayout()
{
    if (!ready) {
        return;
    }

    // go through conditions and re-parent for the first valid one
    for (int i = 0; i < layouts.count(); i++) {
        ULConditionalLayout *layout = layouts[i];
        if (!layout->layout()) {
            warning(layout, "Cannot activate layout \"" + layout->layoutName() +
                    "\" with no container specified. Falling back to default layout.");
            break;
        }
        if (!layout->layoutName().isEmpty() && layout->when() && layout->when()->evaluate().toBool()) {
            if (currentLayoutIndex == i) {
                return;
            }
            currentLayoutIndex = i;
            // update layout
            reLayout();
            return;
        }
    }
    // check if we need to switch back to default layout
    if (currentLayoutIndex >= 0) {
        // revert and clear changes
        changes.revert();
        changes.clear();
        delete currentLayoutItem;
        currentLayoutItem = 0;
        currentLayoutIndex = -1;
        Q_Q(ULLayouts);
        Q_EMIT q->currentLayoutChanged();
    }
}

void ULLayoutsPrivate::error(QObject *item, const QString &message)
{
    qmlInfo(item) << "ERROR: " << message;
    QQmlEngine *engine = qmlEngine(item);
    if (engine) {
        engine->quit();
    }
}

void ULLayoutsPrivate::error(QObject *item, const QList<QQmlError> &errors)
{
    qmlInfo(item, errors);
    QQmlEngine *engine = qmlEngine(item);
    if (engine) {
        engine->quit();
    }
}

void ULLayoutsPrivate::warning(QObject *item, const QString &message)
{
    qmlInfo(item) << "WARNING: " << message;
}


/*!
 * \qmltype Layouts
 * \instantiates ULLayouts
 * \inqmlmodule Ubuntu.Layouts 0.1
 * \ingroup ubuntu-layouts
 * \brief The Layouts component allows one to specify multiple different layouts for a
 * fixed set of Items, and applies the desired layout to those Items.
 *
 * Layouts is a layout block component incorporating layout definitions and
 * components to lay out. The layouts are defined in the \l layouts property, which
 * is a list of ConditionalLayout components, each declaring the sizes and positions
 * of the components specified to be laid out.
 *
 * \qml
 * Layouts {
 *     id: layouts
 *     layouts: [
 *         ConditionalLayout {
 *             name: "flow"
 *             when: layouts.width > units.gu(60) && layouts.width <= units.gu(100)
 *             Flow {
 *                 anchors.fill: parent
 *                 //[...]
 *             }
 *         },
 *         ConditionalLayout {
 *             name: "column"
 *             when: layouts.width > units.gu(100)
 *             Flickable {
 *                 anchors.fill: parent
 *                 contentHeight: column.childrenRect.height
 *                 Column {
 *                     id: column
 *                     //[...]
 *                 }
 *             }
 *         }
 *     ]
 * }
 * \endqml
 *
 * The components to be laid out must be declared as children of the Layouts component,
 * each set an attached property "Layouts.item" to be a unique string.
 *
 * \qml
 * Layouts {
 *     id: layouts
 *     layouts: [
 *         ConditionalLayout {
 *             name: "flow"
 *             when: layouts.width > units.gu(60) && layouts.width <= units.gu(100)
 *             Flow {
 *                 anchors.fill: parent
 *                 //[...]
 *             }
 *         },
 *         ConditionalLayout {
 *             name: "column"
 *             when: layouts.width > units.gu(100)
 *             Flickable {
 *                 anchors.fill: parent
 *                 contentHeight: column.childrenRect.height
 *                 Column {
 *                     id: column
 *                     //[...]
 *                 }
 *             }
 *         }
 *     ]
 *
 *     Row {
 *         anchors.fill: parent
 *         Button {
 *             text: "Press me"
 *             Layouts.item: "item1"
 *         }
 *         Button {
 *             text: "Cancel"
 *             Layouts.item: "item2"
 *         }
 *     }
 * }
 * \endqml
 *
 * The layout of the children of Layouts is considered the default layout, i.e.
 * currentLayout is an empty string. So in the above example, the buttons arranged
 * in a row is the default layout.
 *
 * The layouts defined by ConditionalLayout components are created and activated
 * when at least one of the layout's condition is evaluated to true. In which
 * case components marked for layout are re-parented to the components defined
 * to lay out those defined in the ConditionalLayout. In case multiple conditions
 * are evaluated to true, the first one in the list will be activated. The deactivated
 * layout is destroyed, exception being the default layout, which is kept in memory for
 * the entire lifetime of the Layouts component.
 *
 * Upon activation, the created component fills in the entire layout block.
 *
 * \qml
 * Layouts {
 *     id: layouts
 *     layouts: [
 *         ConditionalLayout {
 *             name: "flow"
 *             when: layouts.width > units.gu(60) && layouts.width <= units.gu(100)
 *             Flow {
 *                 anchors.fill: parent
 *                 ItemLayout {
 *                     item: "item1"
 *                 }
 *                 ItemLayout {
 *                     item: "item2"
 *                 }
 *             }
 *         },
 *         ConditionalLayout {
 *             name: "column"
 *             when: layouts.width > units.gu(100)
 *             Flickable {
 *                 anchors.fill: parent
 *                 contentHeight: column.childrenRect.height
 *                 Column {
 *                     id: column
 *                     ItemLayout {
 *                         item: "item1"
 *                     }
 *                     ItemLayout {
 *                         item: "item2"
 *                     }
 *                 }
 *             }
 *         }
 *     ]
 *
 *     Row {
 *         anchors.fill: parent
 *         Button {
 *             text: "Press me"
 *             Layouts.item: "item1"
 *         }
 *         Button {
 *             text: "Cancel"
 *             Layouts.item: "item2"
 *         }
 *     }
 * }
 * \endqml
 *
 * Conditional layouts must be named in order to be activatable. These names (strings)
 * should be unique within a Layouts item and can be used to identify changes in
 * between layouts in scripts, so additional layout specific customization on laid
 * out items can be done. The current layout is presented by the currentLayout
 * property.
 *
 * Extending the previous example by changing the button color to green when the
 * current layout is "column", the code would look as follows:
 * \qml
 * Layouts {
 *     id: layouts
 *     layouts: [
 *         ConditionalLayout {
 *             name: "flow"
 *             when: layouts.width > units.gu(60) && layouts.width <= units.gu(100)
 *             Flow {
 *                 anchors.fill: parent
 *                 ItemLayout {
 *                     item: "item1"
 *                 }
 *                 ItemLayout {
 *                     item: "item2"
 *                 }
 *             }
 *         },
 *         ConditionalLayout {
 *             name: "column"
 *             when: layouts.width > units.gu(100)
 *             Flickable {
 *                 anchors.fill: parent
 *                 contentHeight: column.childrenRect.height
 *                 Column {
 *                     id: column
 *                     ItemLayout {
 *                         item: "item1"
 *                     }
 *                     ItemLayout {
 *                         item: "item2"
 *                     }
 *                 }
 *             }
 *         }
 *     ]
 *
 *     Row {
 *         anchors.fill: parent
 *         Button {
 *             text: "Press me"
 *             Layouts.item: "item1"
 *             color: (layouts.currentLayout === "column") ? "green" : "gray"
 *         }
 *         Button {
 *             text: "Cancel"
 *             Layouts.item: "item2"
 *             color: (layouts.currentLayout === "column") ? "green" : "gray"
 *         }
 *     }
 * }
 * \endqml
 */

ULLayouts::ULLayouts(QQuickItem *parent):
    QQuickItem(parent),
    d_ptr(new ULLayoutsPrivate(this))
{
}

ULLayouts::~ULLayouts()
{
}

ULLayoutsAttached * ULLayouts::qmlAttachedProperties(QObject *owner)
{
    return new ULLayoutsAttached(owner);
}

void ULLayouts::componentComplete()
{
    QQuickItem::componentComplete();
    Q_D(ULLayouts);
    d->ready = true;
    d->validateConditionalLayouts();
    d->getLaidOutItems();
    d->updateLayout();
}

/*!
 * \qmlproperty string Layouts::currentLayout
 * The property holds the active layout name. The default layout is identified
 * by an empty string. This property can be used for additional customization
 * of the components which are not supported by the layouting.
 */

QString ULLayouts::currentLayout() const
{
    Q_D(const ULLayouts);
    return d->currentLayoutIndex >= 0 ? d->layouts[d->currentLayoutIndex]->layoutName() : QString();
}

/*!
 * \internal
 * Provides a list of layouts for internal use.
 */
QList<ULConditionalLayout*> ULLayouts::layoutList()
{
    Q_D(ULLayouts);
    return d->layouts;
}

/*!
 * \qmlproperty list<ConditionalLayout> Layouts::layouts
 * The property holds the list of different ConditionalLayout elements.
 */
QQmlListProperty<ULConditionalLayout> ULLayouts::layouts()
{
    Q_D(ULLayouts);
    return QQmlListProperty<ULConditionalLayout>(this, &(d->layouts),
                                                 &ULLayoutsPrivate::append_layout,
                                                 &ULLayoutsPrivate::count_layouts,
                                                 &ULLayoutsPrivate::at_layout,
                                                 &ULLayoutsPrivate::clear_layouts);
}

#include "moc_ullayouts.cpp"
