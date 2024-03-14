/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		Header for 86Box VM manager model module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
 */

#ifndef QT_VMMANAGER_MODEL_H
#define QT_VMMANAGER_MODEL_H


#include <QAbstractListModel>
#include "qt_vmmanager_system.hpp"

class VMManagerModel : public QAbstractListModel {

    Q_OBJECT

public:
    //    VMManagerModel(const QStringList &strings, QObject *parent = nullptr)
    //            : QAbstractListModel(parent), machines(strings) {}
    VMManagerModel();
    ~VMManagerModel() override;

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;
    void addConfigToModel(VMManagerSystem *system_config);

        VMManagerSystem * getConfigObjectForIndex(QModelIndex index);
    QModelIndex getIndexForConfigFile(QFileInfo config_file);
    void reload();

private:
    QVector<VMManagerSystem *> machines;
    void modelDataChanged();

};


#endif //QT_VMMANAGER_MODEL_H
