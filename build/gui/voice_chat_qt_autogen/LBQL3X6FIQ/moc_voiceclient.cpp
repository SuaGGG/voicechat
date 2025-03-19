/****************************************************************************
** Meta object code from reading C++ file 'voiceclient.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../gui/ui/include/voiceclient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'voiceclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VoiceClient_t {
    const uint offsetsAndSize[22];
    char stringdata0[169];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_VoiceClient_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_VoiceClient_t qt_meta_stringdata_VoiceClient = {
    {
QT_MOC_LITERAL(0, 11), // "VoiceClient"
QT_MOC_LITERAL(12, 15), // "roomListUpdated"
QT_MOC_LITERAL(28, 0), // ""
QT_MOC_LITERAL(29, 56), // "std::unordered_map<std::strin..."
QT_MOC_LITERAL(86, 5), // "rooms"
QT_MOC_LITERAL(92, 23), // "connectionStatusChanged"
QT_MOC_LITERAL(116, 9), // "connected"
QT_MOC_LITERAL(126, 17), // "roomStatusChanged"
QT_MOC_LITERAL(144, 6), // "inRoom"
QT_MOC_LITERAL(151, 5), // "error"
QT_MOC_LITERAL(157, 11) // "std::string"

    },
    "VoiceClient\0roomListUpdated\0\0"
    "std::unordered_map<std::string,std::vector<std::string>>\0"
    "rooms\0connectionStatusChanged\0connected\0"
    "roomStatusChanged\0inRoom\0error\0"
    "std::string"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VoiceClient[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x06,    1 /* Public */,
       5,    1,   41,    2, 0x06,    3 /* Public */,
       7,    1,   44,    2, 0x06,    5 /* Public */,
       9,    1,   47,    2, 0x06,    7 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void, 0x80000000 | 10,    9,

       0        // eod
};

void VoiceClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VoiceClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->roomListUpdated((*reinterpret_cast< std::add_pointer_t<std::unordered_map<std::string,std::vector<std::string>>>>(_a[1]))); break;
        case 1: _t->connectionStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->roomStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->error((*reinterpret_cast< std::add_pointer_t<std::string>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VoiceClient::*)(const std::unordered_map<std::string,std::vector<std::string>> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VoiceClient::roomListUpdated)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VoiceClient::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VoiceClient::connectionStatusChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (VoiceClient::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VoiceClient::roomStatusChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (VoiceClient::*)(const std::string & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VoiceClient::error)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject VoiceClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_VoiceClient.offsetsAndSize,
    qt_meta_data_VoiceClient,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_VoiceClient_t
, QtPrivate::TypeAndForceComplete<VoiceClient, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const std::unordered_map<std::string,std::vector<std::string>> &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<bool, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<bool, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const std::string &, std::false_type>



>,
    nullptr
} };


const QMetaObject *VoiceClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VoiceClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VoiceClient.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int VoiceClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void VoiceClient::roomListUpdated(const std::unordered_map<std::string,std::vector<std::string>> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void VoiceClient::connectionStatusChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void VoiceClient::roomStatusChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void VoiceClient::error(const std::string & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
