#ifndef QT_SHORTCUTS_H
#define QT_SHORTCUTS_H

#include <QDialog>
#include <QWidget>
#include <QKeyEvent>
#include <QItemSelection>
#include <QPushButton>

#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QKeyCombination>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class KeyShortcuts; }
QT_END_NAMESPACE

class KeyShortcuts : public QDialog {
    Q_OBJECT

    public:
    explicit KeyShortcuts(const QString &existing = {}, QWidget *parent = nullptr);
    ~KeyShortcuts() override;
    // This is primarily for macOS, where Qt's Control corresponds to Command
    // and Meta corresponds to Alt.
    // Important: Only use the output of this command for displaying the
    // portable text. Do not store the value because it is altered from
    // what Qt expects
    static QString formatSequence(const QString &sequence);
    static QStringList portableToNativePrintableList(const QString &portableTextString);
    QString getPortableShortcut();
    QString getPrintableShortcut() const;

private:
    Ui::KeyShortcuts *ui;
    bool capturing = false;
    QString releaseShortcut;
    void printShortcutKeys(QLayout *layout, const QStringList &keyList);

private slots:
    void toggleCapture();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};


#endif //QT_SHORTCUTS_H