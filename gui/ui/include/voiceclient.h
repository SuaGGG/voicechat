#pragma once

#include <QObject>
#include <QStringList>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <asio_network.hpp>
#include "../../../include/voice_client.hpp"

class QTimer;
class QProcess;

class VoiceClient : public QObject {
    Q_OBJECT

public:
    explicit VoiceClient(QObject *parent = nullptr);
    ~VoiceClient() override;

    // 连接相关方法
    bool connect(const std::string& serverAddress);
    void disconnect();
    bool isConnected() const;
    bool isInRoom() const;

    // 房间相关方法
    bool joinRoom(const std::string& roomId);
    bool leaveRoom();
    void getAvailableRooms();

    // 用户相关方法
    void setUsername(const std::string& username);
    std::string getUsername() const;

    // 音频设备相关方法
    QStringList getAudioInputDevices() const;
    QStringList getAudioOutputDevices() const;
    bool setAudioInputDevice(const QString& deviceName);
    bool setAudioOutputDevice(const QString& deviceName);
    float getInputVolume() const;
    float getOutputVolume() const;
    bool setInputVolume(float volume);
    bool setOutputVolume(float volume);
    
    // 音频测试方法
    bool startAudioInputTest();
    bool stopAudioInputTest();
    bool playAudioOutputTest();

    // 内部音频设备访问方法
    voicechat::PortAudioDevice* getInputDevice() const;
    voicechat::PortAudioDevice* getOutputDevice() const;

signals:
    void roomListUpdated(const std::unordered_map<std::string, std::vector<std::string>>& rooms);
    void connectionStatusChanged(bool connected);
    void roomStatusChanged(bool inRoom);
    void error(const std::string& error);

private:
    void setupCallbacks();
    void playTestSoundQtBased();

    std::unique_ptr<voicechat::VoiceClient> client;
    std::unique_ptr<QTimer> roomListTimer;
    std::string userId;
    bool connected{false};
    bool inRoom{false};
    std::string currentRoomId;
    std::unordered_map<std::string, int> inputDeviceMap;
    std::unordered_map<std::string, int> outputDeviceMap;
}; 