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

#include "qt_vmmanager_details_section.hpp"

#include <utility>

const QString VMManagerDetailsSection::sectionSeparator = ";";

VMManagerDetailsSection::VMManagerDetailsSection(QString sectionName)
{
    // Initialize even though they will get wiped out
    // (to keep clang-tidy happy)
    frameGridLayout = new QGridLayout();
    mainLayout = new QVBoxLayout();
//    mainLayout->setContentsMargins(0, 50, 0, 50);
    buttonLayout = new QHBoxLayout();

    collapseButton = new CollapseButton();
    setupMainLayout();
//    buttonGridLayout = new QGridLayout();
    setSectionName(sectionName);
//    buttonLayout->setContentsMargins(0, 0, 5, 0);
    buttonLayout->setContentsMargins(getMargins(VMManagerDetailsSection::MarginSection::ToolButton));
    buttonLayout->addWidget(collapseButton);

//    buttonLayout->addStretch();
    QFrame *hLine;
    hLine = new QFrame();
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);
    hLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    buttonLayout->addWidget(hLine);

    mainLayout->addLayout(buttonLayout);
//    mainLayout->addWidget(collapseButton);

    frame = new QFrame();
    frame->setFrameShape(QFrame::Box);
    frame->setFrameStyle(QFrame::NoFrame);
//    frame->setContentsMargins(20, 0, 0, 0);
    collapseButton->setContent(frame);

//    layout->addLayout(frameGridLayout, 1, 0);
    mainLayout->addWidget(frame);

}
VMManagerDetailsSection::~VMManagerDetailsSection()
{
}
void
VMManagerDetailsSection::setSectionName(QString name)
{
    sectionName = std::move(name);
    collapseButton->setText(" " + sectionName);
    // Bold the section headers
    collapseButton->setStyleSheet(collapseButton->styleSheet().append("font-weight: bold;"));
}

void
VMManagerDetailsSection::addSection(QString name, QString value)
{
    auto new_section = DetailSection { std::move(name), std::move(value)};
    sections.push_back(new_section);
    setSections();
}

void
VMManagerDetailsSection::setupMainLayout()
{
    // clang-tidy says I don't need to check before deleting
    delete mainLayout;
    mainLayout = new QVBoxLayout;
//    mainLayout->setContentsMargins(0, 0, 0, 0);
}
void
VMManagerDetailsSection::clearContentsSetupGrid()
{
    // Clear everything out
    if(frameGridLayout) {
        while(frameGridLayout->count()) {
            QLayoutItem * cur_item = frameGridLayout->takeAt(0);
            if(cur_item->widget())
                delete cur_item->widget();
            delete cur_item;
        }
    }

    delete frameGridLayout;
//    sections.clear();
    frameGridLayout = new QGridLayout();
    qint32 *left = nullptr, *top = nullptr, *right = nullptr, *bottom = nullptr;
    frameGridLayout->getContentsMargins(left, top, right, bottom);
    frameGridLayout->setContentsMargins(getMargins(VMManagerDetailsSection::MarginSection::DisplayGrid));
    frame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    frame->setLayout(frameGridLayout);
}
void
VMManagerDetailsSection::setSections()
{
    clearContentsSetupGrid();
    int row = 0;


    for ( const auto& section : sections) {
        // if the string contains the separator (defined elsewhere) then split and
        // add each entry on a new line. Otherwise, just add the one.
        QStringList sectionsToAdd;
        if(section.value.contains(VMManagerDetailsSection::sectionSeparator)) {
            sectionsToAdd = section.value.split(VMManagerDetailsSection::sectionSeparator);
        } else {
            sectionsToAdd.push_back(section.value);
        }
        for(const auto&line : sectionsToAdd) {
//            auto labelDescription = new QLabel();
            auto labelValue = new QLabel();
            labelValue->setTextInteractionFlags(labelValue->textInteractionFlags() | Qt::TextSelectableByMouse);

            // Reduce the text size for the label
            // First, get the existing font
            auto smaller_font = labelValue->font();
            // Get a smaller size
            auto smaller_size = 0.85 * smaller_font.pointSize();
            // Set the font to the smaller size
            smaller_font.setPointSizeF(smaller_size);
            // Assign that new, smaller font to the label
            labelValue->setFont(smaller_font);

            labelValue->setText(line);
            frameGridLayout->addWidget(labelValue, row, 1, Qt::AlignLeft);
            row++;
        }
    }
}
void
VMManagerDetailsSection::clear()
{
    sections.clear();
}

// QT for Linux and Windows doesn't have the same default margins as QT on MacOS.
// For consistency in appearance we'll have to return the margins on a per-OS basis
QMargins
VMManagerDetailsSection::getMargins(VMManagerDetailsSection::MarginSection section)
{
    switch (section) {
        case MarginSection::ToolButton:
#if defined(Q_OS_WINDOWS) or defined(Q_OS_LINUX)
            return {10, 0, 5, 0};
#else
            return {0, 0, 5, 0};
#endif
        case MarginSection::DisplayGrid:
#if defined(Q_OS_WINDOWS) or defined(Q_OS_LINUX)
            return {10, 0, 0, 10};
#else
            return {0, 0, 0, 10};
#endif
    }
}

// CollapseButton Class

CollapseButton::CollapseButton(QWidget *parent) : QToolButton(parent), content_(nullptr) {
    setCheckable(true);
    setStyleSheet("background:none; border:none;");
    setIconSize(QSize(8, 8));
    setFont(QApplication::font());
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(this, &QToolButton::toggled, [=](bool checked) {
        setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
        content_ != nullptr && checked ? showContent() : hideContent();
    });
    setChecked(true);
}

void CollapseButton::setText(const QString &text) {
    //        text_ = QString(" " + text);
    QToolButton::setText(" " + text);
}

void CollapseButton::setContent(QWidget *content) {
    assert(content != nullptr);
    content_ = content;
    auto animation_ = new QPropertyAnimation(content_, "maximumHeight"); // QObject with auto delete
    animation_->setStartValue(0);
    animation_->setEasingCurve(QEasingCurve::InOutQuad);
    animation_->setDuration(300);
    animation_->setEndValue(content->geometry().height() + 10);
    animator_.addAnimation(animation_);
    if (!isChecked()) {
        content->setMaximumHeight(0);
    }
}

void CollapseButton::hideContent() {
    animator_.setDirection(QAbstractAnimation::Backward);
    animator_.start();
}

void CollapseButton::showContent() {
    animator_.setDirection(QAbstractAnimation::Forward);
    animator_.start();
}

