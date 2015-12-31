/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2016 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
*                                                                                       *
*  This program is free software; you can redistribute it and/or modify it under        *
*  the terms of the GNU General Public License as published by the Free Software        *
*  Foundation; either version 2 of the License, or (at your option) any later           *
*  version.                                                                             *
*                                                                                       *
*  This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
*  PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
*                                                                                       *
*  You should have received a copy of the GNU General Public License along with         *
*  this program.  If not, see <http://www.gnu.org/licenses/>.                           *
*****************************************************************************************/

/* NOTE : 
 This class is base on depreciated qxt library
   - rewrite to remove qxt dependancy
   - using media key patch (from clementine) for Media Key
   - using "Global shortcuts for X11 with Qt 5" from Lukas Holecek
        -> for Qt5 replace QX11Info by qplatformnativeinterface to get display   
*/

#include "globalshortcut_p.h"


#include <QVector>
#include <QtDebug>

#if QT_VERSION < 0x050000
#   include <QX11Info>
#else
#   include <QApplication>
#   include <qpa/qplatformnativeinterface.h>
#   include <xcb/xcb.h>
#endif

#include <X11/Xlib.h>
#include "keymapper_x11.h"


int GlobalShortcutX11Private::ref = 0;


#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
QAbstractEventDispatcher::EventFilter GlobalShortcutX11Private::prevEventFilter = 0;
#endif


QHash<QPair<quint32, quint32>, GlobalShortcut*> GlobalShortcutX11Private::shortcuts;

/* -------------------------------------------------------------------------------------------*/
/*  GlobalShortcutX11Private ( part from qxtglobalshortcut.cpp )                              */
/* -------------------------------------------------------------------------------------------*/

GlobalShortcutX11Private::GlobalShortcutX11Private(GlobalShortcut *gs) : enabled(true), key(Qt::Key(0)), mods(Qt::NoModifier),q_ptr(gs)
{
    if (ref == 0)
    {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        prevEventFilter = QAbstractEventDispatcher::instance()->setEventFilter(eventFilter);
#else
        QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
#endif
    }
    ++ref;
}

GlobalShortcutX11Private::~GlobalShortcutX11Private()
{
    --ref;
    if (ref == 0) 
    {
        QAbstractEventDispatcher *ed = QAbstractEventDispatcher::instance();
        if (ed != 0) 
        {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
            ed->setEventFilter(prevEventFilter);
#else
            ed->removeNativeEventFilter(this);
#endif
        }
    }
}

bool GlobalShortcutX11Private::setShortcut(const QKeySequence& shortcut)
{
    Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
    key = shortcut.isEmpty() ? Qt::Key(0) : Qt::Key((shortcut[0] ^ allMods) & shortcut[0]);
    mods = shortcut.isEmpty() ? Qt::KeyboardModifiers(0) : Qt::KeyboardModifiers(shortcut[0] & allMods);
    const quint32 nativeKey = nativeKeycode(key);
    const quint32 nativeMods = nativeModifiers(mods);
    const bool res = registerShortcut(nativeKey, nativeMods);
    shortcuts.insert(qMakePair(nativeKey, nativeMods), q_ptr);
    if (!res)
        qWarning() << "[GlobalShortcut] failed to register:" << QKeySequence(key + mods).toString();
    return res;
}

bool GlobalShortcutX11Private::unsetShortcut()
{
    const quint32 nativeKey = nativeKeycode(key);
    const quint32 nativeMods = nativeModifiers(mods);
    const bool res = unregisterShortcut(nativeKey, nativeMods);
    shortcuts.remove(qMakePair(nativeKey, nativeMods));
    if (!res)
        qWarning() << "[GlobalShortcut] failed to unregister:" << QKeySequence(key + mods).toString();
    key = Qt::Key(0);
    mods = Qt::KeyboardModifiers(0);
    return res;
}

void GlobalShortcutX11Private::activateShortcut(quint32 nativeKey, quint32 nativeMods)
{
    GlobalShortcut* shortcut = shortcuts.value(qMakePair(nativeKey, nativeMods));
    if (shortcut && shortcut->isEnabled())
    {
        /* activated is not available => call a public method to trigger shortcut Q CLASS signal*/
        emit shortcut->sendActivated();
    }
}




/* -------------------------------------------------------------------------------------------*/
/*  GlobalShortcutX11Private ( part from qxtglobalshortcut_x11.cpp )                          */
/* -------------------------------------------------------------------------------------------*/

namespace {

const QVector<quint32> maskModifiers = QVector<quint32>()
    << 0 << Mod2Mask << LockMask << (Mod2Mask | LockMask);

typedef int (*X11ErrorHandler)(Display *display, XErrorEvent *event);


class GS_X11ErrorHandler 
{
public:
    static bool error;

    static int qxtX11ErrorHandler(Display *display, XErrorEvent *event)
    {
        Q_UNUSED(display);
        switch (event->error_code)
        {
            case BadAccess:
            case BadValue:
            case BadWindow:
                if (event->request_code == 33 /* X_GrabKey */ ||
                        event->request_code == 34 /* X_UngrabKey */)
                {
                     error = true;
                    //TODO:
                    //char errstr[256];
                    //XGetErrorText(dpy, err->error_code, errstr, 256);
                }
        }
        return 0;
    }

    GS_X11ErrorHandler()
    {
        error = false;
        m_previousErrorHandler = XSetErrorHandler(qxtX11ErrorHandler);
    }

    ~GS_X11ErrorHandler()
    {
        XSetErrorHandler(m_previousErrorHandler);
    }

private:
    X11ErrorHandler m_previousErrorHandler;
};






bool GS_X11ErrorHandler::error = false;

class X11Data 
{
public:
    X11Data()
    {
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        m_display = QX11Info::display();
#else
        QPlatformNativeInterface *native = qApp->platformNativeInterface();
        void *display = native->nativeResourceForScreen(QByteArray("display"),
                                                        QGuiApplication::primaryScreen());
        m_display = reinterpret_cast<Display *>(display);
#endif
    }

    bool isValid()
    {
        return m_display != 0;
    }

    Display *display()
    {
        Q_ASSERT(isValid());
        return m_display;
    }

    Window rootWindow()
    {
        return DefaultRootWindow(display());
    }

    bool grabKey(quint32 keycode, quint32 modifiers, Window window)
    {
        GS_X11ErrorHandler errorHandler;

        for (int i = 0; !errorHandler.error && i < maskModifiers.size(); ++i) {
            XGrabKey(display(), keycode, modifiers | maskModifiers[i], window, True,
                     GrabModeAsync, GrabModeAsync);
        }

        if (errorHandler.error) {
            ungrabKey(keycode, modifiers, window);
            return false;
        }

        return true;
    }

    bool ungrabKey(quint32 keycode, quint32 modifiers, Window window)
    {
        GS_X11ErrorHandler errorHandler;

        foreach (quint32 maskMods, maskModifiers) {
            XUngrabKey(display(), keycode, modifiers | maskMods, window);
        }

        return !errorHandler.error;
    }

private:
    Display *m_display;
};

} // namespace



/* -------------------------------------------------------------------------------------------*/
/*  GlobalShortcutX11Private (part from qxtglobalshortcut_x11.cpp                             */
/* -------------------------------------------------------------------------------------------*/

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
bool GlobalShortcutX11Private::eventFilter(void *message)
{
    XEvent *event = static_cast<XEvent *>(message);
    if (event->type == KeyPress)
    {
        XKeyEvent *key = reinterpret_cast<XKeyEvent *>(event);
        unsigned int keycode = key->keycode;
        unsigned int keystate = key->state;
        
        activateShortcut(
            keycode,
            keystate & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)
        );
    }        
    
    return prevEventFilter ? prevEventFilter(message) : false;
}  
#else
bool GlobalShortcutX11Private::nativeEventFilter(const QByteArray & eventType,void *message, long *result)
{
    Q_UNUSED(result);

    xcb_key_press_event_t *kev = 0;
    if (eventType == "xcb_generic_event_t") 
    {
        xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);
        if ((ev->response_type & 127) == XCB_KEY_PRESS)
            kev = static_cast<xcb_key_press_event_t *>(message);
    }

    if (kev != 0) 
    {
        unsigned int keycode = kev->detail;
        unsigned int keystate = 0;
        if(kev->state & XCB_MOD_MASK_1)
            keystate |= Mod1Mask;
        if(kev->state & XCB_MOD_MASK_CONTROL)
            keystate |= ControlMask;
        if(kev->state & XCB_MOD_MASK_4)
            keystate |= Mod4Mask;
        if(kev->state & XCB_MOD_MASK_SHIFT)
            keystate |= ShiftMask;

        activateShortcut(
            keycode,
            keystate & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask)
        );
    }
    
    return false;
}
#endif

quint32 GlobalShortcutX11Private::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    // ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, and Mod5Mask
    quint32 native = 0;
    if (modifiers & Qt::ShiftModifier)
        native |= ShiftMask;
    if (modifiers & Qt::ControlModifier)
        native |= ControlMask;
    if (modifiers & Qt::AltModifier)
        native |= Mod1Mask;
    if (modifiers & Qt::MetaModifier)
        native |= Mod4Mask;

    // TODO: resolve these?
    //if (modifiers & Qt::MetaModifier)
    //if (modifiers & Qt::KeypadModifier)
    //if (modifiers & Qt::GroupSwitchModifier)
    return native;
}

quint32 GlobalShortcutX11Private::nativeKeycode(Qt::Key key)
{
    // !!!!!! KEEP THIS MODIFICATION TO HAVE media keys works !!!!!
    
    // (davidsansome) Try the table from QKeyMapper first - this seems to be
    // the only way to get Keysyms for the media keys.
    unsigned int keysym = 0;
    int i = 0;
    while (KeyTbl[i]) {
      if (KeyTbl[i+1] == static_cast<uint>(key)) {
        keysym = KeyTbl[i];
        break;
      }
      i += 2;
    }

    // If that didn't work then fall back on XStringToKeysym
    if (!keysym) {
      keysym = XStringToKeysym(QKeySequence(key).toString().toLatin1().data());
    }


    X11Data x11;
    if (!x11.isValid())
        return 0;

    return XKeysymToKeycode(x11.display(), keysym);
}

bool GlobalShortcutX11Private::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
    X11Data x11;
    return x11.isValid() && x11.grabKey(nativeKey, nativeMods, x11.rootWindow());
}

bool GlobalShortcutX11Private::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
    X11Data x11;
    return x11.isValid() && x11.ungrabKey(nativeKey, nativeMods, x11.rootWindow());
}
