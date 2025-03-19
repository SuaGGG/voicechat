#pragma once

#include <QMainWindow>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class VoiceClient;
class QTimer;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void setupConnections();
    void updateUI();
    void connectToServer();
    void onConnectButtonClicked();
    void onJoinRoomButtonClicked();
    void onLeaveRoomButtonClicked();
    void onRoomListUpdated(const std::unordered_map<std::string, std::vector<std::string> >& rooms);
    void onConnectionStatusChanged(bool connected);
    void onRoomStatusChanged(bool inRoom);
    void onError(const std::string& error);
    void onConnectServerTriggered();
    void onDisconnectServerTriggered();
    void onDisconnectAllTriggered();
    void onAudioInputTriggered();
    void onAudioOutputTriggered();
    void onSettingsTriggered();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<VoiceClient> voiceClient;
    std::unique_ptr<QTimer> updateTimer;
}; 