#include "plaintextedit.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QFocusEvent>

PlainTextEdit::PlainTextEdit(QWidget *parent)
: QPlainTextEdit(parent), mCompleter(0)
{

}

PlainTextEdit::~PlainTextEdit()
{
}

void PlainTextEdit::setCompleter(QCompleter *completer)
{
    if (mCompleter)
        QObject::disconnect(mCompleter, 0, this, 0);

    mCompleter = completer;

    if (!mCompleter)
        return;

    mCompleter->setWidget(this);
    mCompleter->setCompletionMode(QCompleter::PopupCompletion);
    mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(mCompleter, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

QCompleter *PlainTextEdit::completer() const
{
    return mCompleter;
}

void PlainTextEdit::insertCompletion(const QString& completion)
{
    if (mCompleter->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - mCompleter->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString PlainTextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();

    // go to start of the word
    tc.movePosition(QTextCursor::StartOfWord);
    tc.movePosition(QTextCursor::PreviousCharacter);
    tc.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);

    // word starts with "."
    if (tc.selectedText() == ".") {
        tc.movePosition(QTextCursor::PreviousCharacter);
        tc.movePosition(QTextCursor::PreviousCharacter);
        tc.movePosition(QTextCursor::StartOfWord);
        tc.movePosition(QTextCursor::EndOfWord,QTextCursor::KeepAnchor);
        tc.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor);
        tc.movePosition(QTextCursor::EndOfWord,QTextCursor::KeepAnchor);
    } else {
        tc.movePosition(QTextCursor::EndOfWord);
        tc.movePosition(QTextCursor::StartOfWord,QTextCursor::KeepAnchor);
    }

    return tc.selectedText();
}

void PlainTextEdit::focusInEvent(QFocusEvent *e)
{
    if (mCompleter)
        mCompleter->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void PlainTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (!mCompleter) {
        return QPlainTextEdit::keyPressEvent(e);
    }

    if (mCompleter && mCompleter->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore(); 
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    if (e->key() == Qt::Key_Return && e->modifiers() & Qt::ControlModifier) {
        if (mCompleter->popup()->isVisible())
            mCompleter->popup()->hide();
        emit submit();
        return;
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!mCompleter || !isShortcut) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!mCompleter || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,/;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        mCompleter->popup()->hide();
        return;
    }

    if (completionPrefix != mCompleter->completionPrefix()) {
        mCompleter->setCompletionPrefix(completionPrefix);
        mCompleter->popup()->setCurrentIndex(mCompleter->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(mCompleter->popup()->sizeHintForColumn(0)
                + mCompleter->popup()->verticalScrollBar()->sizeHint().width());
    mCompleter->complete(cr);
}
