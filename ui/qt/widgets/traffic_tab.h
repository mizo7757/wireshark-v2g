/** @file
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TRAFFIC_TAB_H
#define TRAFFIC_TAB_H

#include "config.h"

#include <ui/recent.h>

#include <ui/qt/models/atap_data_model.h>
#include <ui/qt/filter_action.h>

#include <QTabWidget>
#include <QTreeView>
#include <QFile>
#include <QUrl>
#include <QAbstractItemDelegate>

/**
 * @brief Callback for creating an ATapDataModel
 *
 * @param protoId the protocol id for the callback to use
 * @param filter setting the filter for the tap
 * @return either null, if no model could be created, or an instance
 * of the model itself.
 */
typedef ATapDataModel * (*ATapModelCallback)(int protoId, QString filter);

/**
 * @brief Callback for creating an item delegate
 *
 * @param parent the parent for the delegate to attach to
 * @return either null if no delegate had been created, or an instance for
 * the delegate
 */
typedef QAbstractItemDelegate * (*ATapCreateDelegate)(QWidget * parent);

class TabData
{
public:
    TabData();
    TabData(const TabData &) = default;
    TabData &operator=(const TabData &) = default;

    TabData(QString name, int proto);

    QString name() const;
    int protoId() const;

private:
    QString _name;
    int _protoId;
};

Q_DECLARE_METATYPE(TabData)

/**
 * @brief A QTabWidget class, providing tap information
 *
 * This class combines all required information, to display tapped data
 * to the user. Specifically it handles all model data internally, therefore
 * removing the need of the dialog to know how data is being stored or
 * generated.
 */
class TrafficTab : public QTabWidget
{
    Q_OBJECT

public:

    /**
     * @brief Type for the selection of export
     * @see copyToClipboard
     */
    typedef enum {
        CLIPBOARD_CSV,  /* export as CSV */
        CLIPBOARD_YAML, /* export as YAML */
        CLIPBOARD_JSON  /* export as JSON */
    } eTrafficTabClipboard;

    TrafficTab(QWidget *parent = nullptr);
    virtual ~TrafficTab();

    /**
     * @brief Set the Protocol Info for the traffic tab
     *
     * This has to be called right after instantiating the class. The reason this is not
     * done inside the constructor is such, that the object can be used with Qt Designer
     * without having to removing the predefined object during setup of the UI.
     *
     * @param tableName The name for the table. Used for the protocol selection button
     * @param recentList The list to store the selected protocols in
     * @param createModel A callback, which will create the correct model for the trees
     *
     * @see ATapModelCallback
     */
    void setProtocolInfo(QString tableName, GList ** recentList, ATapModelCallback createModel);

    /**
     * @brief Ensure, that the given protocol id is the first tab set
     *
     * @param cliId a protocol id for the first tab
     */
    void setFirstTab(int cliId);

    /**
     * @brief Set the Delegate object for a specific column
     *
     * @param column the column to set the delegate for. It will apply for all models
     * residing inside this tab object
     * @param createDelegate the callback for the delegate creation
     *
     * @see ATapCreateDelegate
     */
    void setDelegate(int column, ATapCreateDelegate createDelegate);

    /**
     * @brief Returns a list of all selected protocols in the traffic tab
     *
     * @return QVector<int> a list containing all protocols currently being displayed
     */
    QVector<int> selectedProtocols() const;

    /**
     * @brief Set the filter or remove it by providing an empty filter
     *
     * This differs from filtering the model itself in such a way, that filtering is
     * being done using the epan system. Therefore, once filtered, the only way to get
     * all elements back is to set an empty string.
     *
     * @note Filtering will only work, as long as the capture file remains open. If
     * taps have been disabled and capture has stopped, filtering will no longer work.
     *
     * @param filter the string to be filtered on
     */
    void setFilter(QString filter = QString());

    /**
     * @brief Enable/Disable name resolution for the address column
     *
     * @param checked true to enable name resolution
     */
    void setNameResolution(bool checked);

    /**
     * @brief Disables the taps for this traffic tab.
     *
     * Disables all taps for models used by this traffic tab. They cannot be re-enabled on purpose,
     * as in most cases, disabling them is being done during closing of the original capture file.
     * This also disabled all filter actions, as well as the tap selection button.
     */
    void disableTap();

    /**
     * @brief Create a menu containing clipboard copy entries for this tab
     *
     * It will create all entries, including copying the content of the currently selected tab
     * to CSV, YAML and JSON
     *
     * @param parent the parent object or null
     * @return QMenu* the resulting menu or null
     */
    QMenu * createCopyMenu(QWidget * parent = nullptr);

    /**
     * @brief Checks, wether the given tabpage support name resolution on the address column
     *
     * @param tabIdx the index of the page. If it is out of bounds or < 0, the current index is being used
     * @return true if name resolution is being supported
     * @return false if name resolution is not supported
     */
    bool hasNameResolution(int tabIdx = -1);

#ifdef HAVE_MAXMINDDB
    /**
     * @brief Checks, wether the given tabpage support GeoIP data
     *
     * @param tabIdx the index of the page. If it is out of bounds or < 0, the current index is being used
     * @return true if geoIP data is being supported
     * @return false if geoIP data is not supported
     */
    bool hasGeoIPData(int tabIdx = -1);

    /**
     * @brief Create a map of GeoIP data and write it to a temporary file
     *
     * @param onlyJSON only put the json content into the temporary file
     * @param tabIdx the index of the page. If it is out of bounds or < 0, the current index is being used
     * @return The path to the temporary file for the data
     */
    QUrl createGeoIPMap(bool onlyJSON, int tabIdx = -1);
#endif

    /**
     * @brief Return the itemData for the currently selected index in the currently
     * displayed treeview.
     *
     * @param role the role to be used, defaults to Qt::DisplayRole
     * @return QVariant the resulting value as QVariant type
     */
    QVariant currentItemData(int role = Qt::DisplayRole);

    /**
     * @brief Use nanosecond timestamps if requested
     *
     * @param useNSTime use nanosecond timestamps if required and requested
     */
    void useNanosecondTimestamps(bool useNSTime);

public slots:

    /**
     * @brief Use absolute time for the time columns
     *
     * @param absolute true if absolute time should be used
     */
    void useAbsoluteTime(bool absolute);

signals:
    void filterAction(QString filter, FilterAction::Action action, FilterAction::ActionType type);
    void tabDataChanged(int index);

private:
    QString _tableName;
    int _cliId;
    QVector<int> _protocols;
    QMap<int, QString> _allTaps;
    QMap<int, int> _tabs;
    GList ** _recentList;
    ATapModelCallback _createModel;
    QMap<int, ATapCreateDelegate> _createDelegates;

    bool _disableTaps;
    bool _nameResolution;

    int _exportRole;
    bool _saveRaw;

    void updateTabs();
    QTreeView * createTree(int protoId);
    ATapDataModel * modelForTabIndex(int tabIdx = -1);

    /**
     * @brief Copy the content of the tab to the clipboard as CSV or YAML
     *
     * @param type Either CSV or YAML has to be selected, defaults to CSV
     * @param idx the index of the page. If it is out of bounds or < 0, the current index is being used
     * @see eTrafficTabClipboard
     */
    void copyToClipboard(eTrafficTabClipboard type, int idx = -1);

    QMenu * createActionSubMenu(FilterAction::Action cur_action, QModelIndex idx, bool isConversation);

#ifdef HAVE_MAXMINDDB
    bool writeGeoIPMapFile(QFile * fp, bool json_only, ATapDataModel * dataModel);
#endif

private slots:
    void toggleTab(bool checked = false);
    void customContextMenuRequested(const QPoint &pos);
    void filterActionTriggered();
    void clipboardAction();
    void toggleSaveRaw();

    void modelReset();

    void doCurrentIndexChange(const QModelIndex & cur, const QModelIndex & prev);
};

#endif // TRAFFIC_TAB_H
