#include "../include/voiceclient.h"
#include <QDebug>
#include <QUuid>
#include <QTimer>
#include <QRandomGenerator>
#include <QProcess>
#include <cmath>
#include "audio_device.hpp"

// 定义M_PI如果没有定义
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

VoiceClient::VoiceClient(QObject *parent)
    : QObject(parent)
    , client(std::make_unique<voicechat::VoiceClient>(QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString()))
    , roomListTimer(std::make_unique<QTimer>(this))
    , userId(QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString())
    , connected(false)
    , inRoom(false)
{
    setupCallbacks();
    roomListTimer->setInterval(5000);
    QObject::connect(roomListTimer.get(), &QTimer::timeout, this, &VoiceClient::getAvailableRooms);
    
    // 预先获取设备列表，确保PortAudio初始化
    QStringList inputDevices = getAudioInputDevices();
    QStringList outputDevices = getAudioOutputDevices();
    
    qDebug() << "音频输入设备:" << inputDevices;
    qDebug() << "音频输出设备:" << outputDevices;
}

VoiceClient::~VoiceClient() = default;

void VoiceClient::setupCallbacks()
{
    // 在这里我们不需要设置回调，因为我们直接在各个方法中处理状态变化
}

// 设置用户名
void VoiceClient::setUsername(const std::string& username) {
    userId = username;
}

// 获取用户名
std::string VoiceClient::getUsername() const {
    return userId;
}

bool VoiceClient::connect(const std::string& serverAddress)
{
    
    // 解析地址和端口
    size_t colonPos = serverAddress.find(':');
    std::string host;
    uint16_t port = 8080; // 默认端口

    if (colonPos != std::string::npos) {
        host = serverAddress.substr(0, colonPos);
        try {
            port = static_cast<uint16_t>(std::stoi(serverAddress.substr(colonPos + 1)));
        } catch (const std::exception& e) {
            emit error("端口号无效: " + std::string(e.what()));
            return false;
        }
    } else {
        host = serverAddress;
    }

    try {
        if (client->connect(host, port)) {
            connected = true;
            emit connectionStatusChanged(true);
            roomListTimer->start();
            return true;
        }
    } catch (const std::exception& e) {
        emit error(std::string("连接失败: ") + e.what());
    }
    return false;
}

void VoiceClient::disconnect()
{
    if (connected) {
        roomListTimer->stop();
        if (inRoom) {
            leaveRoom();
        }
        client->disconnect();
        connected = false;
        emit connectionStatusChanged(false);
    }
}

bool VoiceClient::joinRoom(const std::string& roomId)
{
    if (client->joinRoom(roomId)) {
        inRoom = true;
        emit roomStatusChanged(true);
        return true;
    }
    return false;
}

bool VoiceClient::leaveRoom()
{
    if (client->leaveRoom()) {
        inRoom = false;
        emit roomStatusChanged(false);
        return true;
    }
    return false;
}

void VoiceClient::getAvailableRooms()
{
    if (connected) {
        try {
            auto rooms = client->getAvailableRooms();
            emit roomListUpdated(rooms);
        } catch (const std::exception& e) {
            emit error(std::string("获取房间列表失败: ") + e.what());
        }
    }
}

bool VoiceClient::isConnected() const
{
    return connected;
}

bool VoiceClient::isInRoom() const
{
    return inRoom;
}

// 获取底层输入设备
voicechat::PortAudioDevice* VoiceClient::getInputDevice() const {
    if (!client) {
        return nullptr;
    }
    return client->getInputDevice();
}

// 获取底层输出设备
voicechat::PortAudioDevice* VoiceClient::getOutputDevice() const {
    if (!client) {
        return nullptr;
    }
    return client->getOutputDevice();
}

// 音频设备相关方法实现
QStringList VoiceClient::getAudioInputDevices() const {
    QStringList devices;
    try {
        auto inputDevices = client->getAudioInputDevices();
        for (const auto& device : inputDevices) {
            devices.append(QString::fromStdString(device.name));
        }
        qDebug() << "找到" << devices.size() << "个音频输入设备";
    } catch (const std::exception& e) {
        qWarning() << "获取音频输入设备列表失败:" << e.what();
        const_cast<VoiceClient*>(this)->error("获取音频输入设备列表失败: " + std::string(e.what()));
    }
    return devices;
}

QStringList VoiceClient::getAudioOutputDevices() const {
    QStringList devices;
    try {
        auto outputDevices = client->getAudioOutputDevices();
        for (const auto& device : outputDevices) {
            devices.append(QString::fromStdString(device.name));
        }
        qDebug() << "找到" << devices.size() << "个音频输出设备";
    } catch (const std::exception& e) {
        qWarning() << "获取音频输出设备列表失败:" << e.what();
        const_cast<VoiceClient*>(this)->error("获取音频输出设备列表失败: " + std::string(e.what()));
    }
    return devices;
}

bool VoiceClient::setAudioInputDevice(const QString& deviceName) {
    try {
        auto inputDevices = client->getAudioInputDevices();
        for (const auto& device : inputDevices) {
            if (QString::fromStdString(device.name) == deviceName) {
                bool success = client->setAudioInputDevice(device.index);
                if (success) {
                    qDebug() << "成功切换音频输入设备到:" << deviceName;
                } else {
                    qWarning() << "切换音频输入设备失败:" << deviceName;
                    emit error("切换音频输入设备失败");
                }
                return success;
            }
        }
        qWarning() << "未找到指定的音频输入设备:" << deviceName;
        emit error("未找到指定的音频输入设备: " + deviceName.toStdString());
    } catch (const std::exception& e) {
        qWarning() << "设置音频输入设备时发生错误:" << e.what();
        emit error("设置音频输入设备失败: " + std::string(e.what()));
    }
    return false;
}

bool VoiceClient::setAudioOutputDevice(const QString& deviceName) {
    try {
        auto outputDevices = client->getAudioOutputDevices();
        for (const auto& device : outputDevices) {
            if (QString::fromStdString(device.name) == deviceName) {
                bool success = client->setAudioOutputDevice(device.index);
                if (success) {
                    qDebug() << "成功切换音频输出设备到:" << deviceName;
                } else {
                    qWarning() << "切换音频输出设备失败:" << deviceName;
                    emit error("切换音频输出设备失败");
                }
                return success;
            }
        }
        qWarning() << "未找到指定的音频输出设备:" << deviceName;
        emit error("未找到指定的音频输出设备: " + deviceName.toStdString());
    } catch (const std::exception& e) {
        qWarning() << "设置音频输出设备时发生错误:" << e.what();
        emit error("设置音频输出设备失败: " + std::string(e.what()));
    }
    return false;
}

// 音量控制方法实现
bool VoiceClient::setInputVolume(float volume) {
    try {
        if (volume < 0.0f || volume > 1.0f) {
            qWarning() << "输入音量超出范围[0.0, 1.0]:" << volume;
            return false;
        }
        client->setInputVolume(volume);
        qDebug() << "设置输入音量:" << volume;
        return true;
    } catch (const std::exception& e) {
        qWarning() << "设置输入音量时发生错误:" << e.what();
        emit error("设置输入音量失败: " + std::string(e.what()));
        return false;
    }
}

bool VoiceClient::setOutputVolume(float volume) {
    try {
        if (volume < 0.0f || volume > 1.0f) {
            qWarning() << "输出音量超出范围[0.0, 1.0]:" << volume;
            return false;
        }
        client->setOutputVolume(volume);
        qDebug() << "设置输出音量:" << volume;
        return true;
    } catch (const std::exception& e) {
        qWarning() << "设置输出音量时发生错误:" << e.what();
        emit error("设置输出音量失败: " + std::string(e.what()));
        return false;
    }
}

float VoiceClient::getInputVolume() const {
    try {
        float volume = client->getInputVolume();
        qDebug() << "当前输入音量:" << volume;
        return volume;
    } catch (const std::exception& e) {
        qWarning() << "获取输入音量时发生错误:" << e.what();
        const_cast<VoiceClient*>(this)->error("获取输入音量失败: " + std::string(e.what()));
        return 0.0f;
    }
}

float VoiceClient::getOutputVolume() const {
    try {
        float volume = client->getOutputVolume();
        qDebug() << "当前输出音量:" << volume;
        return volume;
    } catch (const std::exception& e) {
        qWarning() << "获取输出音量时发生错误:" << e.what();
        const_cast<VoiceClient*>(this)->error("获取输出音量失败: " + std::string(e.what()));
        return 0.0f;
    }
}

// 音频测试方法
bool VoiceClient::startAudioInputTest() {
    try {
        qDebug() << "开始音频输入测试";
        
        // 在WSL环境下，使用PulseAudio工具而不是arecord
        QProcess* process = new QProcess(this);
        process->setProcessChannelMode(QProcess::MergedChannels);
        
        // 使用pactl列出音频源
        process->start("pactl", QStringList() << "list" << "sources");
        if (process->waitForFinished(2000)) {
            QString output = process->readAllStandardOutput();
            qDebug() << "可用音频输入设备:" << output;
        } else {
            qWarning() << "获取音频输入设备列表超时";
        }
        
        // 确保连接
        if (!connected) {
            qWarning() << "未连接到服务器，无法进行音频测试";
            emit error("未连接到服务器，无法进行音频测试");
            return false;
        }
        
        // 尝试激活音频设备（取消挂起状态）
        QProcess activateProcess;
        activateProcess.start("pactl", QStringList() << "suspend-source" << "RDPSource" << "0");
        if (activateProcess.waitForFinished(2000)) {
            qDebug() << "已尝试激活RDPSource设备，结果:" << activateProcess.exitCode();
        } else {
            qWarning() << "激活音频设备超时";
        }
        
        // 检查设备状态是否已改变
        QProcess checkProcess;
        checkProcess.start("pactl", QStringList() << "list" << "sources" << "RDPSource");
        if (checkProcess.waitForFinished(2000)) {
            QString output = checkProcess.readAllStandardOutput();
            qDebug() << "激活后RDPSource状态:" << output;
        }
        
        // 尝试使用parecord录制短音频示例
        QProcess* recordProcess = new QProcess(this);
        recordProcess->setProcessChannelMode(QProcess::MergedChannels);
        
        // 连接信号以便在进程完成时删除它并获取输出
        QObject::connect(recordProcess, &QProcess::readyReadStandardOutput, [recordProcess]() {
            qDebug() << "录音进程输出:" << recordProcess->readAllStandardOutput();
        });
        
        QObject::connect(recordProcess, &QProcess::readyReadStandardError, [recordProcess]() {
            qDebug() << "录音进程错误:" << recordProcess->readAllStandardError();
        });
        
        QObject::connect(recordProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [recordProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                    qDebug() << "录音进程结束，退出码:" << exitCode;
                    recordProcess->deleteLater();
                });
        
        // 使用RDPSource设备录制3秒音频，使用不同参数尝试
        recordProcess->start("parecord", QStringList() 
                             << "--verbose"
                             << "--device=RDPSource" 
                             << "--channels=1"
                             << "--format=s16le"
                             << "--rate=44100"
                             << "/tmp/test_recording.wav"
                             << "--record-time=3");
        
        // 如果第一次尝试失败，可以尝试不指定设备
        if (recordProcess->error() != QProcess::UnknownError) {
            qDebug() << "尝试使用默认设备录音";
            recordProcess->start("parecord", QStringList()
                                << "--verbose"
                                << "--channels=1" 
                                << "--format=s16le"
                                << "--rate=44100"
                                << "/tmp/test_recording_default.wav"
                                << "--record-time=3");
        }
        
        qDebug() << "音频输入测试已启动，正在录制3秒测试音频";
        return true;
    } catch (const std::exception& e) {
        qWarning() << "启动音频输入测试失败:" << e.what();
        emit error("启动音频输入测试失败: " + std::string(e.what()));
        return false;
    }
}

bool VoiceClient::stopAudioInputTest() {
    try {
        qDebug() << "停止音频输入测试";
        // 相关清理工作
        return true;
    } catch (const std::exception& e) {
        qWarning() << "停止音频输入测试失败:" << e.what();
        emit error("停止音频输入测试失败: " + std::string(e.what()));
        return false;
    }
}

bool VoiceClient::playAudioOutputTest() {
    try {
        qDebug() << "播放音频输出测试";
        
        // 在WSL环境中使用系统命令播放测试音频
        if (client) {
            // 先尝试使用playTestSoundQtBased方法
            playTestSoundQtBased();
            return true;
        } else {
            qWarning() << "客户端未初始化，无法播放测试音频";
            emit error("客户端未初始化，无法播放测试音频");
            return false;
        }
    } catch (const std::exception& e) {
        qWarning() << "播放音频输出测试失败:" << e.what();
        emit error("播放音频输出测试失败: " + std::string(e.what()));
        return false;
    }
}

void VoiceClient::playTestSoundQtBased() {
    qDebug() << "使用系统命令播放测试音频";
    
    // 尝试不同的方式播放声音，优先使用PulseAudio命令
    QStringList commands;
    
    // 优先尝试PulseAudio命令，使用RDPSink设备
    commands << "paplay --device=RDPSink --volume=65536 /usr/share/sounds/freedesktop/stereo/bell.oga"
             << "paplay --device=RDPSink --volume=65536 /usr/share/sounds/freedesktop/stereo/message.oga"
             << "paplay --device=RDPSink --volume=65536 /usr/share/sounds/ubuntu/stereo/bell.ogg"
             // 备用命令
             << "paplay --volume=65536 /usr/share/sounds/freedesktop/stereo/bell.oga"
             << "aplay -D default /usr/share/sounds/alsa/Front_Center.wav"
             << "play -q /usr/share/sounds/alsa/Front_Center.wav";
    
    bool success = false;
    for (const QString& cmd : commands) {
        qDebug() << "尝试命令:" << cmd;
        QProcess process;
        process.start("bash", QStringList() << "-c" << cmd);
        if (process.waitForFinished(3000)) {
            int exitCode = process.exitCode();
            qDebug() << "命令执行完成，退出码:" << exitCode;
            if (exitCode == 0) {
                success = true;
                break;
            }
        } else {
            qWarning() << "命令执行超时";
            process.kill();
        }
    }
    
    if (!success) {
        qWarning() << "所有播放声音的尝试都失败了";
        
        // 最后的尝试：生成一个测试文件然后播放
        QProcess process;
        process.start("bash", QStringList() << "-c" << "echo -e '\\a'");
        process.waitForFinished(1000);
        
        // 尝试使用蜂鸣器
        process.start("bash", QStringList() << "-c" << "echo -e '\\007'");
        process.waitForFinished(1000);
    }
    
    qDebug() << "测试音频播放完成";
}

    