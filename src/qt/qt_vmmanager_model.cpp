/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		86Box VM manager model module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
 */

#include <QDebug>
#include <QSize>
#include <QDir>
#include "qt_vmmanager_model.hpp"

VMManagerModel::VMManagerModel() {
    auto machines_vec = VMManagerSystem::scanForConfigs();
    for ( const auto& each_config : machines_vec) {
        machines.append(each_config);
        connect(each_config, &VMManagerSystem::itemDataChanged, this, &VMManagerModel::modelDataChanged);
    }
}

VMManagerModel::~VMManagerModel() {
    for ( auto machine : machines) {
        delete machine;
    }
}

int
VMManagerModel::rowCount(const QModelIndex &parent) const {
    return machines.size();
}

QVariant
VMManagerModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    if (index.row() >= machines.size())
        return {};

    int i;
    switch (role) {
        case Qt::DisplayRole:
            return machines.at(index.row())->config_name;
//        case Qt::SizeHintRole:
//            return QSize(0, 40);
        case Qt::ToolTipRole:
            return machines.at(index.row())->shortened_dir;
//        case Qt::TextAlignmentRole:
//            i = Qt::AlignVCenter | Qt::AlignLeft;
//            return i;
        case Qt::UserRole:
            return machines.at(index.row())->getAll("General");
        case Qt::UserRole + 1:
            return machines.at(index.row())->getProcessStatusString();
        case Qt::UserRole + 2:
            return QVariant::fromValue(machines.at(index.row())->getProcessStatus());
        default:
            return {};
    }
}

QVariant
VMManagerModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal)
        return QStringLiteral("Column %1").arg(section);
    else
        return QStringLiteral("Row %1").arg(section);
}

VMManagerSystem *
VMManagerModel::getConfigObjectForIndex(QModelIndex index) {
    return machines.at(index.row());
}
void
VMManagerModel::reload()
{
    // Scan for configs
    auto machines_vec = VMManagerSystem::scanForConfigs();
    for ( const auto& scanned_config : machines_vec) {
        int found = 0;
        for (const auto& existing_config : machines) {
            if(*scanned_config == *existing_config) {
                found=1;
            }
        }
        if(!found) {
            addConfigToModel(scanned_config);
        }
    }
    // TODO: Remove missing configs
}

QModelIndex
VMManagerModel::getIndexForConfigFile(QFileInfo config_file)
{
    int object_index = 0;
    for (const auto& config_object: machines) {
        if (config_object->config_file == config_file) {
            return this->index(object_index);
        }
        object_index++;
    }
    return {};
}

void
VMManagerModel::addConfigToModel(VMManagerSystem *system_config)
{
    beginInsertRows(QModelIndex(), this->rowCount(QModelIndex()), this->rowCount(QModelIndex()));
    machines.append(system_config);
    connect(system_config, &VMManagerSystem::itemDataChanged, this, &VMManagerModel::modelDataChanged);
    endInsertRows();
}
void
VMManagerModel::modelDataChanged()
{
    emit dataChanged(this->index(0), this->index(machines.size()-1));
}
