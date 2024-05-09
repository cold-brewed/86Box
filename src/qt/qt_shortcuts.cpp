#include <QDebug>
#include <QFontDatabase>

#include "qt_shortcuts.hpp"
#include "ui_qt_shortcuts.h"

extern "C" {
#include <86box/86box.h>
}

KeyShortcuts::
KeyShortcuts(const QString &existing, QWidget *parent) : ui(new Ui::KeyShortcuts)
{
    ui->setupUi(this);
    setWindowTitle(tr("Mouse release shortcut"));
    connect(ui->captureButton, &QPushButton::clicked, this, &KeyShortcuts::toggleCapture);
    releaseShortcut = QString(existing);
    if (!existing.isEmpty()) {
        const QStringList printableKeys = portableToNativePrintableList(releaseShortcut);
        printShortcutKeys(ui->buttonLayout->layout(), printableKeys);
        ui->textKeys->setText(formatSequence(releaseShortcut));
    }
}

KeyShortcuts::~KeyShortcuts()
    = default;
QString
KeyShortcuts::formatSequence(const QString &sequence)
{
    QString formattedSequence;
#ifdef Q_OS_MACOS
    QStringList formattedList;
    const auto keyList = sequence.split('+');
    for (const auto &key : keyList) {
        if(key == "Ctrl") {
            formattedList.append("Command");
        } else if (key == "Meta") {
            formattedList.append("Control");
        } else {
            formattedList.append(key);
        }
    }
    formattedSequence = formattedList.join("+");
#else
    formattedSequence = sequence;
#endif
    return formattedSequence;
}

QStringList
KeyShortcuts::portableToNativePrintableList(const QString &portableTextString)
{
    QStringList output;

    qDebug() << "portableToNativePrintableList:" << portableTextString;
    const auto keyList = portableTextString.split('+');
    for (const auto &key : keyList) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        // This is for a deprecation warning after QKeyCombination
        // was added in Qt 6.2.0
        QKeyCombination keyToAdd;
#else
        int keyToAdd;
#endif
#ifdef Q_OS_MACOS
        // qDebug() << "keyPTN" << key;
        if (0 == key.compare("Alt", Qt::CaseInsensitive)) {
            keyToAdd = Qt::Key_Alt;
        } else if (0 == key.compare("Ctrl", Qt::CaseInsensitive)) {
            keyToAdd = Qt::Key_Control;
        } else if (0 == key.compare("Shift", Qt::CaseInsensitive)) {
            keyToAdd = Qt::Key_Shift;
        } else if (0 == key.compare("Meta", Qt::CaseInsensitive)) {
            keyToAdd = Qt::Key_Meta;
        } else {
            if (const QKeySequence keySeq(key); keySeq.count() == 1) {
                // qDebug() << "lastPTN:" << keySeq[0];
                keyToAdd = keySeq[0];
            }
        }
        output.append(QKeySequence(keyToAdd).toString(QKeySequence::NativeText));
#else
        if (key.compare("Alt", Qt::CaseInsensitive) == 0 || key.compare("Ctrl", Qt::CaseInsensitive) == 0 || key.compare("Shift", Qt::CaseInsensitive) == 0 || key.compare("Meta", Qt::CaseInsensitive) == 0) {
            output.append(key);
        } else {
            if (const QKeySequence keySeq(key); keySeq.count() == 1) {
                keyToAdd = keySeq[0];
                output.append(QKeySequence(keyToAdd).toString(QKeySequence::NativeText));
            } else {
                // If all else fails
                output.append(key);
            }
        }
#endif
    }

    return output;
}
QString
KeyShortcuts::getPortableShortcut()
{
    return releaseShortcut;
}

QString
KeyShortcuts::getPrintableShortcut() const
{
    return formatSequence(releaseShortcut);
}

void
KeyShortcuts::printShortcutKeys(QLayout *layout, const QStringList &keyList)
{
    // First, remove all existing key widgets (stylized QFrame and QLabel)
    // Process them in reverse order
    for (int i=layout->count(); i>0 ; i--)
    {
        if (const auto layoutWidget = layout->itemAt(i-1)->widget(); layoutWidget)
        {
            layout->removeWidget(layoutWidget);
            layoutWidget->deleteLater();
        }
    }

    int largestWidth = 0;
    // Loop through all the keys to get the largest potential label size
    for(const auto &a_key: keyList) {
        auto sizingLabel = new QLabel();
        // Fixed font to experiment with
        QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        fixedFont.setWeight(QFont::Bold);
        // Set the font weight to bold
        auto labelFont = sizingLabel->font();
        labelFont.setWeight(QFont::Bold);
        sizingLabel->setFont(labelFont);
        // sizingLabel->setFont(fixedFont);
        auto currentWidth = sizingLabel->fontMetrics().boundingRect(a_key).width();
        largestWidth = currentWidth > largestWidth ? currentWidth : largestWidth;
        sizingLabel->deleteLater();
    }

    // Add each "key" (stylized QFrame and QLabel) to the layout
    for(const auto &a_key: keyList) {
        // The frame will provide the "key" look and the background
        const auto frame = new QFrame(this);
        frame->setStyleSheet(".QFrame{border: 2px solid black; border-radius: 10px; background-color: white;} .QLabel{color: black;}");
        // The label will provide the text of the key
        const auto label = new QLabel(frame);
        // Font options. Note: Not all are in use, some are here for experimentation
        QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        fixedFont.setWeight(QFont::Bold);
        auto labelFont = label->font();
        labelFont.setWeight(QFont::Bold);
        label->setFont(labelFont);
        // label->setFont(fixedFont);
        // Finally set the key text
        label->setText(a_key);
        label->setAlignment(Qt::AlignCenter);
        label->setFixedWidth(largestWidth);
        // The frame gets the layout, which gets the widget
        frame->setLayout(new QHBoxLayout(frame));
        frame->layout()->addWidget(label);
        // Finally, it is all added to the parent layout
        layout->addWidget(frame);
    }
}
void
KeyShortcuts::toggleCapture()
{
    if(capturing) {
        capturing = false;
        ui->captureButton->setText(tr("Capture"));
        this->removeEventFilter(this);
        ui->statusText->clear();
    } else {
        ui->captureButton->setText(tr("Capturing.."));
        this->installEventFilter(this);
        ui->statusText->clear();
        capturing = true;
    }
}
bool
KeyShortcuts::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride){
        const auto *keyEvent = dynamic_cast<QKeyEvent*>(event);

        int keyInt = keyEvent->key();
        const auto key = static_cast<Qt::Key>(keyInt);
        if(key == Qt::Key_unknown){
            qDebug() << "Unknown key";
            return false;
        }
        // We've only received the modifier keys: Ctrl, Shift, Alt, Meta.
        if(key == Qt::Key_Control ||
            key == Qt::Key_Shift ||
            key == Qt::Key_Alt ||
            key == Qt::Key_Meta)
        {
            qDebug() << "Not accepting single click of only a modifier key: Ctrl, Shift, Alt or Meta";
            ui->statusText->setText("Non-modifier key also required");
            qDebug() << "QKeySequence:" << QKeySequence(keyInt).toString(QKeySequence::NativeText);
            return false;
        }

        // Check for a combination of keys
        // NOTE: Qt documentation says this about modifiers():
        // > This function cannot always be trusted. The user can
        // > confuse it by pressing both Shift keys simultaneously and releasing one of them, for example.
        const Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        const QString keyText = keyEvent->text();
        qDebug() << "Pressed Key:" << keyText;

        // QList<Qt::Key> modifiersList;
        if(modifiers & Qt::ShiftModifier)
            keyInt += Qt::SHIFT;
        if(modifiers & Qt::ControlModifier)
            keyInt += Qt::CTRL;
        if(modifiers & Qt::AltModifier)
            keyInt += Qt::ALT;
        if(modifiers & Qt::MetaModifier)
            keyInt += Qt::META;

        if(modifiers == Qt::NoModifier) {
            ui->statusText->setText("Modifier required (only " + QKeySequence(keyInt).toString(QKeySequence::NativeText) + " was pressed)");
            return false;
        }

        const QStringList printableKeys = portableToNativePrintableList(QKeySequence(keyInt).toString(QKeySequence::PortableText));
        printShortcutKeys(ui->buttonLayout->layout(), printableKeys);


        qDebug() << "Final QKeySequence (native):" << QKeySequence(keyInt).toString(QKeySequence::NativeText);
        qDebug() << "Final QKeySequence (portable):" << QKeySequence(keyInt).toString(QKeySequence::PortableText);
        const auto portableText = QKeySequence(keyInt).toString(QKeySequence::PortableText);
        ui->textKeys->setText(formatSequence(portableText));
        releaseShortcut = QKeySequence(keyInt).toString(QKeySequence::PortableText);
        qDebug() << "releaseShortcut is" << releaseShortcut;
        ui->statusText->clear();
        toggleCapture();
    }
    return false;
}
