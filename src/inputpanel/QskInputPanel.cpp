/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#include "QskInputPanel.h"
#include "QskInputEngine.h"
#include "QskVirtualKeyboard.h"
#include "QskInputPredictionBar.h"
#include "QskTextInput.h"
#include "QskTextLabel.h"
#include "QskLinearBox.h"

#include <QString>
#include <QLocale>
#include <QPointer>
#include <QInputMethodQueryEvent>

namespace
{
    class TextInputProxy final : public QskTextInput
    {
    public:
        TextInputProxy( QQuickItem* parentItem = nullptr ):
            QskTextInput( parentItem )
        {
            setObjectName( "InputPanelInputProxy" );
            setFocusPolicy( Qt::NoFocus );
        }

        void setup( QQuickItem* inputItem )
        {
            int passwordMaskDelay = -1;
            QString passwordCharacter;

            if ( auto textInput = qobject_cast< QskTextInput* >( inputItem ) )
            {
                passwordMaskDelay = textInput->passwordMaskDelay();
                passwordCharacter = textInput->passwordCharacter();

                if ( echoMode() == QskTextInput::NoEcho )
                {
                    /*
                        Qt::ImhHiddenText does not provide information
                        to decide between NoEcho/Password
                     */
                    auto mode = textInput->echoMode();
                    if ( mode == QskTextInput::Password )
                        setEchoMode( mode );
                }
            }

            if ( passwordMaskDelay >= 0 )
                setPasswordMaskDelay( passwordMaskDelay );
            else
                resetPasswordMaskDelay();

            if ( !passwordCharacter.isEmpty() )
                setPasswordCharacter( passwordCharacter );
            else
                resetPasswordCharacter();
        }

    protected:
        virtual void focusInEvent( QFocusEvent* ) override final
        {
        }

        virtual void focusOutEvent( QFocusEvent* ) override final
        {
        }
    };
}

QSK_SUBCONTROL( QskInputPanel, Panel )

class QskInputPanel::PrivateData
{
public:
    PrivateData():
        inputHints( 0 ),
        maxChars( -1 ),
        hasPrediction( true ),
        hasInputProxy( true )
    {
    }

    QQuickItem* receiverItem()
    {
        return hasInputProxy ? inputProxy : inputItem;
    }

    QPointer< QskInputEngine > engine;
    QPointer< QQuickItem > inputItem;

    QskLinearBox* layout;
    QskTextLabel* prompt;
    TextInputProxy* inputProxy;
    QskInputPredictionBar* predictionBar;
    QskVirtualKeyboard* keyboard;

    Qt::InputMethodHints inputHints;
    int maxChars;

    bool hasPrediction : 1;
    bool hasInputProxy : 1;
};

QskInputPanel::QskInputPanel( QQuickItem* parent ):
    Inherited( parent ),
    m_data( new PrivateData() )
{
    setAutoLayoutChildren( true );
    initSizePolicy( QskSizePolicy::Expanding, QskSizePolicy::Constrained );

    m_data->prompt = new QskTextLabel();
    m_data->prompt->setVisible( false );

    m_data->inputProxy = new TextInputProxy();
    m_data->inputProxy->setVisible( m_data->hasInputProxy );

    m_data->predictionBar = new QskInputPredictionBar();
    m_data->predictionBar->setVisible( false );

    m_data->keyboard = new QskVirtualKeyboard();

    auto layout = new QskLinearBox( Qt::Vertical, this );

    layout->addItem( m_data->prompt, Qt::AlignLeft | Qt::AlignHCenter );
    layout->addItem( m_data->inputProxy, Qt::AlignLeft | Qt::AlignHCenter );
    layout->addStretch( 10 );
    layout->addItem( m_data->predictionBar );
    layout->addItem( m_data->keyboard );

    m_data->layout = layout;

    connect( m_data->predictionBar, &QskInputPredictionBar::predictiveTextSelected,
        this, &QskInputPanel::commitPredictiveText );

    connect( m_data->keyboard, &QskVirtualKeyboard::keySelected,
        this, &QskInputPanel::commitKey );
}

QskInputPanel::~QskInputPanel()
{
}

void QskInputPanel::setEngine( QskInputEngine* engine )
{
    if ( engine == m_data->engine )
        return;

    if ( m_data->engine )
        m_data->engine->disconnect( this );

    m_data->engine = engine;

    if ( engine )
    {
        connect( engine, &QskInputEngine::predictionChanged,
            this, &QskInputPanel::updatePrediction );
    }

    m_data->predictionBar->setVisible(
        m_data->hasPrediction && engine && engine->predictor() );
}

void QskInputPanel::attachInputItem( QQuickItem* item )
{
    if ( item == m_data->inputItem )
        return;

    m_data->inputItem = item;

    if ( m_data->engine )
        m_data->engine->reset();

    if ( item )
    {
        Qt::InputMethodQueries queries = Qt::ImQueryAll;
        queries &= ~Qt::ImEnabled;

        processInputMethodQueries( queries );

        if ( m_data->hasInputProxy )
        {
            m_data->inputProxy->setEditing( true );

            // hiding the cursor in the real input item
            const QInputMethodEvent::Attribute attribute(
                QInputMethodEvent::Cursor, 0, 0, QVariant() );

            QInputMethodEvent event( QString(), { attribute } );
            QCoreApplication::sendEvent( item, &event );

            // not all information is available from the input method query
            m_data->inputProxy->setup( item );
        }
    }
}

QQuickItem* QskInputPanel::attachedInputItem() const
{
    return m_data->inputItem;
}

QQuickItem* QskInputPanel::inputProxy() const
{
    return m_data->inputProxy;
}

void QskInputPanel::updatePrediction()
{
    m_data->predictionBar->setPrediction(
        m_data->engine->prediction() );
}

QskInputEngine* QskInputPanel::engine()
{
    return m_data->engine;
}

QskAspect::Subcontrol QskInputPanel::effectiveSubcontrol(
    QskAspect::Subcontrol subControl ) const
{
    if( subControl == QskBox::Panel )
        return QskInputPanel::Panel;

    return subControl;
}

QString QskInputPanel::inputPrompt() const
{
    return m_data->prompt->text();
}

void QskInputPanel::setInputPrompt( const QString& text )
{
    auto prompt = m_data->prompt;

    if ( text != prompt->text() )
    {
        prompt->setText( text );

        if ( m_data->hasInputProxy )
            prompt->setVisible( !text.isEmpty() );

        Q_EMIT inputPromptChanged( text );
    }
}

bool QskInputPanel::hasInputProxy() const
{
    return m_data->hasInputProxy;
}

void QskInputPanel::setInputProxy( bool on )
{
    if ( m_data->hasInputProxy == on )
        return;

    m_data->hasInputProxy = on;
    m_data->inputProxy->setVisible( on );

    auto prompt = m_data->prompt;

    if ( on )
        prompt->setVisible( !prompt->text().isEmpty() );
    else
        prompt->setVisible( false );
}

void QskInputPanel::commitPredictiveText( int index )
{
    m_data->predictionBar->setPrediction( QStringList() );

    if ( m_data->engine )
    {
        const QString text = m_data->engine->predictiveText( index );

        m_data->engine->reset();
        Q_EMIT textEntered( text, true );
    }
}

void QskInputPanel::commitKey( int key )
{
    if ( m_data->engine == nullptr || m_data->inputItem == nullptr )
        return;

    int spaceLeft = -1;

    if ( !( m_data->inputHints & Qt::ImhMultiLine ) )
    {
        auto receiver = m_data->receiverItem();

        if ( m_data->maxChars >= 0 )
        {
            QInputMethodQueryEvent event( Qt::ImSurroundingText );
            QCoreApplication::sendEvent( receiver, &event );

            const auto text = event.value( Qt::ImSurroundingText ).toString();
            spaceLeft = m_data->maxChars - text.length();
        }
    }

    processKey( key, m_data->inputHints, spaceLeft );
}

void QskInputPanel::processKey( int key,
    Qt::InputMethodHints inputHints, int spaceLeft )
{
    const auto result = m_data->engine->processKey( key, inputHints, spaceLeft );

    auto inputItem = m_data->inputItem;

    if ( result.key )
    {
        switch( result.key )
        {
            case Qt::Key_Return:
            {
                Q_EMIT done( true );
                break;
            }
            case Qt::Key_Escape:
            {
                Q_EMIT done( false );
                break;
            }
            default:
            {
                Q_EMIT keyEntered( result.key );
            }
        }
    }
    else if ( !result.text.isEmpty() )
    {
        Q_EMIT textEntered( result.text, result.isFinal );
    }
}

void QskInputPanel::processInputMethodQueries( Qt::InputMethodQueries queries )
{
    if ( m_data->inputItem == nullptr )
        return;

    QInputMethodQueryEvent event( queries );
    QCoreApplication::sendEvent( m_data->inputItem, &event );

    if ( event.queries() & Qt::ImHints )
    {
        bool hasPrediction = true;
        QskTextInput::EchoMode echoMode = QskTextInput::Normal;

        const auto hints = static_cast< Qt::InputMethodHints >(
            event.value( Qt::ImHints ).toInt() );

        if ( hints & Qt::ImhHiddenText )
        {
            echoMode = QskTextInput::NoEcho;
        }

        /*
            - Qt::ImhSensitiveData
            - Qt::ImhNoAutoUppercase
            - Qt::ImhMultiLine

            // we should start in a specific mode

            - Qt::ImhPreferNumbers
            - Qt::ImhPreferUppercase
            - Qt::ImhPreferLowercase
            - Qt::ImhPreferLatin

            // we should lock all other modes

            - Qt::ImhFormattedNumbersOnly
            - Qt::ImhUppercaseOnly
            - Qt::ImhDialableCharactersOnly
            - Qt::ImhEmailCharactersOnly
            - Qt::ImhUrlCharactersOnly
            - Qt::ImhLatinOnly

            // we should have specific input panels

            - Qt::ImhDate
            - Qt::ImhTime
            - Qt::ImhDigitsOnly
            - Qt::ImhFormattedNumbersOnly
         */

        m_data->hasPrediction = 
            !( hints & ( Qt::ImhNoPredictiveText | Qt::ImhExclusiveInputMask ) );

        m_data->predictionBar->setVisible(
            hasPrediction && m_data->engine && m_data->engine->predictor() );

        m_data->inputProxy->setEchoMode( echoMode );

        m_data->inputHints = hints;
    }

    if ( event.queries() & Qt::ImPreferredLanguage )
    {
        // already handled by the input context
    }

    if ( event.queries() & Qt::ImMaximumTextLength )
    {
        // needs to be handled before Qt::ImCursorPosition !

        m_data->maxChars = event.value( Qt::ImMaximumTextLength ).toInt();
#if 1
        if ( m_data->maxChars >= 32767 )
            m_data->maxChars = -1;
#endif

        if ( m_data->hasInputProxy )
            m_data->inputProxy->setMaxLength( m_data->maxChars );
    }


    if ( event.queries() & Qt::ImSurroundingText )
    {
        if ( m_data->hasInputProxy )
        {
            const auto text = event.value( Qt::ImSurroundingText ).toString();
            m_data->inputProxy->setText( text );
        }
    }

    if ( event.queries() & Qt::ImCursorPosition )
    {
        if ( m_data->hasInputProxy )
        {
            const auto pos = event.value( Qt::ImCursorPosition ).toInt();
            m_data->inputProxy->setCursorPosition( pos );
        }
    }

    if ( event.queries() & Qt::ImCurrentSelection )
    {
#if 0
        const auto text = event.value( Qt::ImCurrentSelection ).toString();
        if ( !text.isEmpty() )
        {
        }
#endif
    }
    /*
        Qt::ImMicroFocus
        Qt::ImCursorRectangle
        Qt::ImFont
        Qt::ImAnchorPosition

        Qt::ImAbsolutePosition
        Qt::ImTextBeforeCursor
        Qt::ImTextAfterCursor
        Qt::ImPlatformData     // hard to say...
        Qt::ImEnterKeyType
        Qt::ImAnchorRectangle
        Qt::ImInputItemClipRectangle
     */
}

void QskInputPanel::keyPressEvent( QKeyEvent* event )
{
    // animate the corresponding key button TODO

    switch( event->key() )
    {
        case Qt::Key_Return:
        case Qt::Key_Escape:
        {
            commitKey( event->key() );
            break;
        }

        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Meta:
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_CapsLock:
        case Qt::Key_NumLock:
        case Qt::Key_ScrollLock:
        {
            break;
        }

        default:
        {
            const auto text = event->text();
            if ( !text.isEmpty() )
                commitKey( text[0].unicode() );
            else
                commitKey( event->key() );
        }
    }
}

void QskInputPanel::keyReleaseEvent( QKeyEvent* event )
{
    return Inherited::keyReleaseEvent( event );
}

#include "moc_QskInputPanel.cpp"