/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		86Box VM manager system details section module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2023 cold-brewed and the 86Box development team
 */

#ifndef QT_VMMANAGER_DETAILS_SECTION_H
#define QT_VMMANAGER_DETAILS_SECTION_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QAbstractAnimation>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QApplication>
//#include <QDebug>
#include "qt_vmmanager_system.hpp"

class CollapseButton : public QToolButton {
    Q_OBJECT

public:
    explicit CollapseButton(QWidget *parent = nullptr);

    void setText(const QString &text);

    void setContent(QWidget *content);

    void hideContent();

    void showContent();

private:
    QWidget *content_;
    QString text_;
    QParallelAnimationGroup animator_;
};

//QT_BEGIN_NAMESPACE
//namespace Ui { class VMManagerDetailsSection;}
//QT_END_NAMESPACE

class VMManagerDetailsSection : public QWidget {
    Q_OBJECT

public:
    explicit VMManagerDetailsSection(QString sectionName);

    ~VMManagerDetailsSection() override;

    void addSection(QString name, QString value);
    void clear();

    CollapseButton *collapseButton;
//    QGridLayout *buttonGridLayout;
    QGridLayout *frameGridLayout;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
    QFrame *frame;

    static const QString sectionSeparator;


private:
    enum class MarginSection {
        ToolButton,
        DisplayGrid,
    };

    void setSectionName(QString name);
    void setupMainLayout();
    void clearContentsSetupGrid();
    void setSections();

    static QMargins getMargins(MarginSection section);

    QString sectionName;

    struct DetailSection {
        QString name;
        QString value;
    };

    QVector<DetailSection> sections;

};

#endif // QT_VMMANAGER_DETAILS_SECTION_H
