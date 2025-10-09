﻿#pragma once

#include <QWidget>
#include <QList>
#include "AntMessage.h"

class AntMessageManager : public QWidget
{
	Q_OBJECT

public:
	static AntMessageManager* instance();
	// duration 表示显示多久ms
	void showMessage(AntMessage::Type type, const QString& message, int msgDuration = 3000, QWidget *mainWindow= nullptr);
	void showOnTop();
signals:
	void onNewMsgAdd(AntMessage* msg);
private slots:
	void onMessageRequestExit(AntMessage* msg);
	void startExitAnimation();
private:
	explicit AntMessageManager(QWidget* parent = nullptr);
	~AntMessageManager();
private:
	QList<AntMessage*> m_messages;			// 主队列：当前屏幕上存在的所有消息
	static AntMessageManager* m_instance;
	int spacingY = 10;
	int msgHeight = 0;
	bool isAnimating = false;
	int animDuration = 350;				// 动画组的持续时间
	bool m_isBatchAnimating = false;	// 批量上移是否进行中
};
