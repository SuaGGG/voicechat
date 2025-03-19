/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../gui/ui/include/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    const uint offsetsAndSize[48];
    char stringdata0[439];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 10), // "MainWindow"
QT_MOC_LITERAL(11, 16), // "setupConnections"
QT_MOC_LITERAL(28, 0), // ""
QT_MOC_LITERAL(29, 8), // "updateUI"
QT_MOC_LITERAL(38, 15), // "connectToServer"
QT_MOC_LITERAL(54, 22), // "onConnectButtonClicked"
QT_MOC_LITERAL(77, 23), // "onJoinRoomButtonClicked"
QT_MOC_LITERAL(101, 24), // "onLeaveRoomButtonClicked"
QT_MOC_LITERAL(126, 17), // "onRoomListUpdated"
QT_MOC_LITERAL(144, 56), // "std::unordered_map<std::strin..."
QT_MOC_LITERAL(201, 5), // "rooms"
QT_MOC_LITERAL(207, 25), // "onConnectionStatusChanged"
QT_MOC_LITERAL(233, 9), // "connected"
QT_MOC_LITERAL(243, 19), // "onRoomStatusChanged"
QT_MOC_LITERAL(263, 6), // "inRoom"
QT_MOC_LITERAL(270, 7), // "onError"
QT_MOC_LITERAL(278, 11), // "std::string"
QT_MOC_LITERAL(290, 5), // "error"
QT_MOC_LITERAL(296, 24), // "onConnectServerTriggered"
QT_MOC_LITERAL(321, 27), // "onDisconnectServerTriggered"
QT_MOC_LITERAL(349, 24), // "onDisconnectAllTriggered"
QT_MOC_LITERAL(374, 21), // "onAudioInputTriggered"
QT_MOC_LITERAL(396, 22), // "onAudioOutputTriggered"
QT_MOC_LITERAL(419, 19) // "onSettingsTriggered"

    },
    "MainWindow\0setupConnections\0\0updateUI\0"
    "connectToServer\0onConnectButtonClicked\0"
    "onJoinRoomButtonClicked\0"
    "onLeaveRoomButtonClicked\0onRoomListUpdated\0"
    "std::unordered_map<std::string,std::vector<std::string>>\0"
    "rooms\0onConnectionStatusChanged\0"
    "connected\0onRoomStatusChanged\0inRoom\0"
    "onError\0std::string\0error\0"
    "onConnectServerTriggered\0"
    "onDisconnectServerTriggered\0"
    "onDisconnectAllTriggered\0onAudioInputTriggered\0"
    "onAudioOutputTriggered\0onSettingsTriggered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  110,    2, 0x08,    1 /* Private */,
       3,    0,  111,    2, 0x08,    2 /* Private */,
       4,    0,  112,    2, 0x08,    3 /* Private */,
       5,    0,  113,    2, 0x08,    4 /* Private */,
       6,    0,  114,    2, 0x08,    5 /* Private */,
       7,    0,  115,    2, 0x08,    6 /* Private */,
       8,    1,  116,    2, 0x08,    7 /* Private */,
      11,    1,  119,    2, 0x08,    9 /* Private */,
      13,    1,  122,    2, 0x08,   11 /* Private */,
      15,    1,  125,    2, 0x08,   13 /* Private */,
      18,    0,  128,    2, 0x08,   15 /* Private */,
      19,    0,  129,    2, 0x08,   16 /* Private */,
      20,    0,  130,    2, 0x08,   17 /* Private */,
      21,    0,  131,    2, 0x08,   18 /* Private */,
      22,    0,  132,    2, 0x08,   19 /* Private */,
      23,    0,  133,    2, 0x08,   20 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void, QMetaType::Bool,   14,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->setupConnections(); break;
        case 1: _t->updateUI(); break;
        case 2: _t->connectToServer(); break;
        case 3: _t->onConnectButtonClicked(); break;
        case 4: _t->onJoinRoomButtonClicked(); break;
        case 5: _t->onLeaveRoomButtonClicked(); break;
        case 6: _t->onRoomListUpdated((*reinterpret_cast< std::add_pointer_t<std::unordered_map<std::string,std::vector<std::string>>>>(_a[1]))); break;
        case 7: _t->onConnectionStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->onRoomStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->onError((*reinterpret_cast< std::add_pointer_t<std::string>>(_a[1]))); break;
        case 10: _t->onConnectServerTriggered(); break;
        case 11: _t->onDisconnectServerTriggered(); break;
        case 12: _t->onDisconnectAllTriggered(); break;
        case 13: _t->onAudioInputTriggered(); break;
        case 14: _t->onAudioOutputTriggered(); break;
        case 15: _t->onSettingsTriggered(); break;
        default: ;
        }
    }
}

const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.offsetsAndSize,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_MainWindow_t
, QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const std::unordered_map<std::string,std::vector<std::string> > &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<bool, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<bool, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const std::string &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
