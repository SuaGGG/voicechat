#include "../include/mainwindow.h"
#include "ui_mainwindow.h"
#include "../include/voiceclient.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTimer>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QListWidget>
#include <QListWidgetItem>
#include <QString>
#include <QStringList>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <QRandomGenerator>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , voiceClient(std::make_unique<VoiceClient>())
    , updateTimer(std::make_unique<QTimer>(this))
{
    ui->setupUi(this);
    setupConnections();
    updateUI();
    updateTimer->start(5000);
}

MainWindow::~MainWindow()
{
    // ui 是智能指针，会自动删除
}

void MainWindow::setupConnections()
{
    // 连接按钮信号
    connect(ui->joinRoomButton, &QPushButton::clicked, this, &MainWindow::onJoinRoomButtonClicked);
    connect(ui->leaveRoomButton, &QPushButton::clicked, this, &MainWindow::onLeaveRoomButtonClicked);

    // 连接菜单操作信号
    connect(ui->actionConnectServer, &QAction::triggered, this, &MainWindow::onConnectServerTriggered);
    connect(ui->actionDisconnectServer, &QAction::triggered, this, &MainWindow::onDisconnectServerTriggered);
    connect(ui->actionDisconnectAll, &QAction::triggered, this, &MainWindow::onDisconnectAllTriggered);
    connect(ui->actionAudioInput, &QAction::triggered, this, &MainWindow::onAudioInputTriggered);
    connect(ui->actionAudioOutput, &QAction::triggered, this, &MainWindow::onAudioOutputTriggered);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::onSettingsTriggered);

    // 连接 VoiceClient 信号
    connect(voiceClient.get(), &VoiceClient::roomListUpdated, this, &MainWindow::onRoomListUpdated);
    connect(voiceClient.get(), &VoiceClient::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);
    connect(voiceClient.get(), &VoiceClient::roomStatusChanged, this, &MainWindow::onRoomStatusChanged);
    connect(voiceClient.get(), &VoiceClient::error, this, &MainWindow::onError);

    // 设置定时器更新房间列表
    connect(updateTimer.get(), &QTimer::timeout, voiceClient.get(), &VoiceClient::getAvailableRooms);
}

void MainWindow::updateUI()
{
    ui->roomListWidget->clear();
    ui->joinRoomButton->setEnabled(false);
    ui->leaveRoomButton->setEnabled(false);
    
    // 更新菜单项状态
    bool isConnected = voiceClient->isConnected();
    
    ui->actionConnectServer->setEnabled(!isConnected);
    ui->actionDisconnectServer->setEnabled(isConnected);
    ui->actionDisconnectAll->setEnabled(isConnected);
    ui->actionAudioInput->setEnabled(isConnected);
    ui->actionAudioOutput->setEnabled(isConnected);
}

void MainWindow::connectToServer()
{
    // 创建一个对话框
    auto dialog = std::make_unique<QDialog>(this);
    dialog->setWindowTitle("连接服务器");
    dialog->setMinimumWidth(300);
    
    // 创建布局
    auto layout = std::make_unique<QVBoxLayout>(dialog.get());
    
    // 用户名输入
    auto usernameLabel = std::make_unique<QLabel>("用户名:", dialog.get());
    auto usernameEdit = std::make_unique<QLineEdit>(dialog.get());
    usernameEdit->setText("用户" + QString::number(QRandomGenerator::global()->bounded(1000)));
    layout->addWidget(usernameLabel.get());
    layout->addWidget(usernameEdit.get());
    
    // 服务器地址输入
    auto serverLabel = std::make_unique<QLabel>("服务器地址:", dialog.get());
    auto serverEdit = std::make_unique<QLineEdit>(dialog.get());
    serverEdit->setText("127.0.0.1:8080");
    layout->addWidget(serverLabel.get());
    layout->addWidget(serverEdit.get());
    
    // 按钮
    auto buttonBox = std::make_unique<QDialogButtonBox>(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog.get());
    QObject::connect(buttonBox.get(), &QDialogButtonBox::accepted, dialog.get(), &QDialog::accept);
    QObject::connect(buttonBox.get(), &QDialogButtonBox::rejected, dialog.get(), &QDialog::reject);
    layout->addWidget(buttonBox.get());
    
    // 设置对话框的布局
    dialog->setLayout(layout.release());
    
    // 显示对话框
    if (dialog->exec() == QDialog::Accepted) {
        // 获取用户输入
        QString username = usernameEdit->text();
        QString serverAddress = serverEdit->text();
        
        // 检查输入是否有效
        if (username.isEmpty()) {
            QMessageBox::warning(this, "错误", "用户名不能为空");
            return;
        }
        
        if (serverAddress.isEmpty()) {
            QMessageBox::warning(this, "错误", "服务器地址不能为空");
            return;
        }
        
        // 设置用户名
        voiceClient->setUsername(username.toStdString());
        
        // 连接服务器
        if (voiceClient->connect(serverAddress.toStdString())) {
            ui->statusbar->showMessage("已连接到服务器 - 用户名: " + username);
        } else {
            ui->statusbar->showMessage("连接服务器失败");
        }
    }
}

void MainWindow::onConnectButtonClicked()
{
    if (!voiceClient->isConnected()) {
        connectToServer();
    } else {
        voiceClient->disconnect();
        ui->statusbar->showMessage("已断开连接");
    }
}

void MainWindow::onJoinRoomButtonClicked()
{
    auto currentItem = ui->roomListWidget->currentItem();
    if (currentItem) {
        std::string roomId = currentItem->text().toStdString();
        if (voiceClient->joinRoom(roomId)) {
            ui->joinRoomButton->setEnabled(false);
            ui->leaveRoomButton->setEnabled(true);
        }
    }
}

void MainWindow::onLeaveRoomButtonClicked()
{
    if (voiceClient->leaveRoom()) {
        ui->joinRoomButton->setEnabled(true);
        ui->leaveRoomButton->setEnabled(false);
    }
}

void MainWindow::onRoomListUpdated(const std::unordered_map<std::string, std::vector<std::string> >& rooms)
{
    ui->roomListWidget->clear();
    for (const auto& [roomId, participants] : rooms) {
        QString itemText = QString("%1 (%2人)").arg(QString::fromStdString(roomId)).arg(participants.size());
        ui->roomListWidget->addItem(itemText);
        for (const auto& participant : participants) {
            ui->roomListWidget->addItem("  " + QString::fromStdString(participant));
        }
    }
    ui->joinRoomButton->setEnabled(ui->roomListWidget->currentItem() != nullptr);
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    if (connected) {
        ui->statusbar->showMessage("已连接到服务器");
    } else {
        ui->statusbar->showMessage("已断开连接");
    }
    
    // 更新菜单项状态
    ui->actionConnectServer->setEnabled(!connected);
    ui->actionDisconnectServer->setEnabled(connected);
    ui->actionDisconnectAll->setEnabled(connected);
    ui->actionAudioInput->setEnabled(connected);
    ui->actionAudioOutput->setEnabled(connected);
    
    updateUI();
}

void MainWindow::onRoomStatusChanged(bool inRoom)
{
    ui->joinRoomButton->setEnabled(!inRoom && ui->roomListWidget->currentItem() != nullptr);
    ui->leaveRoomButton->setEnabled(inRoom);
}

void MainWindow::onError(const std::string& error)
{
    QMessageBox::warning(this, "错误", QString::fromStdString(error));
}

// 菜单操作槽函数实现
void MainWindow::onConnectServerTriggered()
{
    if (!voiceClient->isConnected()) {
        connectToServer();
    }
}

void MainWindow::onDisconnectServerTriggered()
{
    if (voiceClient->isConnected()) {
        voiceClient->disconnect();
        ui->statusbar->showMessage("已断开连接");
    }
}

void MainWindow::onDisconnectAllTriggered()
{
    if (voiceClient->isConnected()) {
        voiceClient->disconnect();
        ui->statusbar->showMessage("已断开所有连接");
    }
}

void MainWindow::onAudioInputTriggered()
{
    if (!voiceClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接到服务器");
        return;
    }
    
    // 获取设备列表
    QStringList inputDevices = voiceClient->getAudioInputDevices();
    
    // 检查是否有可用设备
    if (inputDevices.isEmpty()) {
        QMessageBox::warning(this, "音频输入", "没有找到可用的音频输入设备");
        return;
    }
    
    // 创建对话框
    auto dialog = std::make_unique<QDialog>(this);
    dialog->setWindowTitle("音频输入设置");
    dialog->setMinimumWidth(400);
    
    // 创建布局
    auto layout = std::make_unique<QVBoxLayout>(dialog.get());
    
    // 设备选择
    auto deviceLabel = std::make_unique<QLabel>("选择输入设备:", dialog.get());
    auto deviceComboBox = std::make_unique<QComboBox>(dialog.get());
    deviceComboBox->addItems(inputDevices);
    layout->addWidget(deviceLabel.get());
    layout->addWidget(deviceComboBox.get());
    
    // 音量控制
    auto volumeLabel = std::make_unique<QLabel>("输入音量:", dialog.get());
    auto volumeSlider = std::make_unique<QSlider>(Qt::Horizontal, dialog.get());
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(voiceClient->getInputVolume() * 100);
    auto volumeValueLabel = std::make_unique<QLabel>(QString::number(volumeSlider->value()) + "%", dialog.get());
    
    // 连接音量滑块的值变化信号
    connect(volumeSlider.get(), &QSlider::valueChanged,
            [volumeValueLabel = volumeValueLabel.get()](int value) mutable {
                volumeValueLabel->setText(QString::number(value) + "%");
            });
    
    auto volumeLayout = std::make_unique<QHBoxLayout>();
    volumeLayout->addWidget(volumeSlider.get());
    volumeLayout->addWidget(volumeValueLabel.get());
    
    layout->addWidget(volumeLabel.get());
    layout->addLayout(volumeLayout.release());
    
    // 测试按钮
    auto testButton = std::make_unique<QPushButton>("测试", dialog.get());
    QLabel* levelIndicator = new QLabel("音量级别: 0%", dialog.get());
    levelIndicator->setMinimumWidth(100);

    // 创建一个定时器用于更新音量指示器
    QTimer* levelTimer = new QTimer(dialog.get());
    levelTimer->setInterval(100); // 100ms更新一次

    // 连接测试按钮
    bool isTestingInput = false;
    QObject::connect(testButton.get(), &QPushButton::clicked, 
        [this, levelTimer, levelIndicator, testButton = testButton.get(), &isTestingInput]() mutable {
            if (!isTestingInput) {
                // 开始测试
                isTestingInput = true;
                testButton->setText("停止测试");
                
                if (voiceClient->startAudioInputTest()) {
                    // 启动定时器来模拟音量变化
                    QObject::connect(levelTimer, &QTimer::timeout, [levelIndicator]() {
                        // 这里应该获取实际音量，现在我们模拟一个随机值
                        static int direction = 1;
                        static int level = 0;
                        
                        level += direction * (rand() % 5);
                        if (level > 100) {
                            level = 100;
                            direction = -1;
                        } else if (level < 0) {
                            level = 0;
                            direction = 1;
                        }
                        
                        // 更新UI
                        levelIndicator->setText("音量级别: " + QString::number(level) + "%");
                    });
                    levelTimer->start();
                    QMessageBox::information(this, "测试", "正在测试麦克风输入，请对着麦克风说话");
                } else {
                    QMessageBox::warning(this, "测试", "无法启动麦克风测试");
                    isTestingInput = false;
                    testButton->setText("测试");
                }
            } else {
                // 停止测试
                isTestingInput = false;
                testButton->setText("测试");
                levelTimer->stop();
                voiceClient->stopAudioInputTest();
                levelIndicator->setText("音量级别: 0%");
            }
        });

    // 添加音量指示器
    layout->addWidget(testButton.get());
    layout->addWidget(levelIndicator);
    
    // 按钮
    auto buttonBox = std::make_unique<QDialogButtonBox>(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog.get());
    QObject::connect(buttonBox.get(), &QDialogButtonBox::accepted, dialog.get(), &QDialog::accept);
    QObject::connect(buttonBox.get(), &QDialogButtonBox::rejected, dialog.get(), &QDialog::reject);
    layout->addWidget(buttonBox.get());
    
    // 设置对话框的布局
    dialog->setLayout(layout.release());
    
    // 显示对话框
    if (dialog->exec() == QDialog::Accepted) {
        // 设置选中的设备
        QString selectedDevice = deviceComboBox->currentText();
        if (voiceClient->setAudioInputDevice(selectedDevice)) {
            // 设置音量
            float volume = volumeSlider->value() / 100.0f;
            if (voiceClient->setInputVolume(volume)) {
                QMessageBox::information(this, "音频输入", "已设置音频输入设备: " + selectedDevice + 
                                        "\n音量: " + QString::number(volume * 100) + "%");
            } else {
                QMessageBox::warning(this, "音频输入", "设置音量失败");
            }
        } else {
            QMessageBox::warning(this, "音频输入", "设置音频输入设备失败");
        }
    }
}

void MainWindow::onAudioOutputTriggered()
{
    if (!voiceClient->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接到服务器");
        return;
    }
    
    // 获取设备列表
    QStringList outputDevices = voiceClient->getAudioOutputDevices();
    
    // 检查是否有可用设备
    if (outputDevices.isEmpty()) {
        QMessageBox::warning(this, "音频输出", "没有找到可用的音频输出设备");
        return;
    }
    
    // 创建对话框
    auto dialog = std::make_unique<QDialog>(this);
    dialog->setWindowTitle("音频输出设置");
    dialog->setMinimumWidth(400);
    
    // 创建布局
    auto layout = std::make_unique<QVBoxLayout>(dialog.get());
    
    // 设备选择
    auto deviceLabel = std::make_unique<QLabel>("选择输出设备:", dialog.get());
    auto deviceComboBox = std::make_unique<QComboBox>(dialog.get());
    deviceComboBox->addItems(outputDevices);
    layout->addWidget(deviceLabel.get());
    layout->addWidget(deviceComboBox.get());
    
    // 音量控制
    auto volumeLabel = std::make_unique<QLabel>("输出音量:", dialog.get());
    auto volumeSlider = std::make_unique<QSlider>(Qt::Horizontal, dialog.get());
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(voiceClient->getOutputVolume() * 100);
    auto volumeValueLabel = std::make_unique<QLabel>(QString::number(volumeSlider->value()) + "%", dialog.get());
    
    // 连接音量滑块的值变化信号
    connect(volumeSlider.get(), &QSlider::valueChanged,
            [volumeValueLabel = volumeValueLabel.get()](int value) mutable {
                volumeValueLabel->setText(QString::number(value) + "%");
            });
    
    auto volumeLayout = std::make_unique<QHBoxLayout>();
    volumeLayout->addWidget(volumeSlider.get());
    volumeLayout->addWidget(volumeValueLabel.get());
    
    layout->addWidget(volumeLabel.get());
    layout->addLayout(volumeLayout.release());
    
    // 测试按钮
    auto testButton = std::make_unique<QPushButton>("测试声音", dialog.get());
    QObject::connect(testButton.get(), &QPushButton::clicked, 
        [this]() {
            if (voiceClient->playAudioOutputTest()) {
                QMessageBox::information(this, "测试", "正在播放测试音频，请注意聆听");
            } else {
                QMessageBox::warning(this, "测试", "播放测试音频失败");
            }
        });
    layout->addWidget(testButton.get());
    
    // 按钮
    auto buttonBox = std::make_unique<QDialogButtonBox>(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog.get());
    QObject::connect(buttonBox.get(), &QDialogButtonBox::accepted, dialog.get(), &QDialog::accept);
    QObject::connect(buttonBox.get(), &QDialogButtonBox::rejected, dialog.get(), &QDialog::reject);
    layout->addWidget(buttonBox.get());
    
    // 设置对话框的布局
    dialog->setLayout(layout.release());
    
    // 显示对话框
    if (dialog->exec() == QDialog::Accepted) {
        // 设置选中的设备
        QString selectedDevice = deviceComboBox->currentText();
        if (voiceClient->setAudioOutputDevice(selectedDevice)) {
            // 设置音量
            float volume = volumeSlider->value() / 100.0f;
            if (voiceClient->setOutputVolume(volume)) {
                QMessageBox::information(this, "音频输出", "已设置音频输出设备: " + selectedDevice + 
                                        "\n音量: " + QString::number(volume * 100) + "%");
            } else {
                QMessageBox::warning(this, "音频输出", "设置音量失败");
            }
        } else {
            QMessageBox::warning(this, "音频输出", "设置音频输出设备失败");
        }
    }
}

void MainWindow::onSettingsTriggered()
{
    QMessageBox::information(this, "设置", "设置功能尚未实现");
} 