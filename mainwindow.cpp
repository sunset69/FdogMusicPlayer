﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QtMultimedia/QMediaPlayer>
#include<QtMultimedia/QMediaPlaylist>
#include<QFileDialog>
#include<QDir>
#include<QPoint>
#include<QNetworkAccessManager>
#include<QVariant>
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);//去掉标题栏
    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist(this);
    playlist->setPlaybackMode(QMediaPlaylist::Sequential);
    player->setPlaylist(playlist);
    connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),
            this,SLOT(onStateChanged(QMediaPlayer::State)));
    connect(player,SIGNAL(positionChanged(qint64)),
            this,SLOT(onPositionChanged(qint64)));
    connect(player,SIGNAL(durationChanged(qint64)),
            this,SLOT(onDurationChanged(qint64)));
    connect(playlist,SIGNAL(currentIndexChanged(int)),
            this,SLOT(onPlaylistChanged(int)));

    network_manager = new QNetworkAccessManager();
    network_request = new QNetworkRequest();
    network_manager2 = new QNetworkAccessManager();
    network_request2 = new QNetworkRequest();

    connect(network_manager2, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished2);

    connect(network_manager, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bIsWindowMoveable = true;
        m_point = event->pos();
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bIsWindowMoveable)
    {
        move(event->pos() - m_point + pos());
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    m_bIsWindowMoveable = false;
}

void MainWindow::search(QString str)
{
//    QString KGAPISTR1 = QString("https://complexsearch.kugou.com/v2/search/song?callback=callback123&keyword=%1"
//         "&page=1&pagesize=30&bitrate=0&isfuzzy=0&tag=em&inputtype=0&platform=WebFilter&userid=-1"
//         "&clientver=2000&iscorrection=1&privilege_filter=0&srcappid=2919&clienttime=1597134090150"
//         "&mid=1597134090150&uuid=1597134090150&dfid=-&signature=F3F2D045B6F17C6D606353AD42E8C1FF").arg(ui->lineEdit_3->text());
   QString KGAPISTR1 = QString("http://mobilecdn.kugou.com/api/v3/search/song?format=json"
                               "&keyword=%1&page=1&pagesize=30").arg(str);
    qDebug()<<KGAPISTR1;
    network_request->setUrl(QUrl(KGAPISTR1));
    network_manager->get(*network_request);
}



void MainWindow::on_pushButton_clicked()
{
   QString curPash =QDir::currentPath();
   QString dlgTitle="选择音频文件";
   QString filter="音频文件(*.mp3 *.wav *.wma)mp3文件(*.mp3);;wav文件(*.wav);;wma文件(*.wma);;所有文件(*.*)";
   QStringList fileList = QFileDialog::getOpenFileNames(this,dlgTitle,curPash,filter);
   if(fileList.count()<1)
       return;
   for(int i = 0;i<fileList.count();i++)
   {
       QString aFile = fileList.at(i);
       playlist->addMedia(QUrl::fromLocalFile(aFile));
       QFileInfo fileInfo(aFile);
       ui->listWidget->addItem(fileInfo.fileName());
       ui->plainTextEdit->appendPlainText(fileList.at(i));

   }
   if(player->state()!=QMediaPlayer::PlayingState)
   {
       playlist->setCurrentIndex(M_Value);
   }
   player->play();
}

void MainWindow::onStateChanged(QMediaPlayer::State state)
{

}

void MainWindow::onPlaylistChanged(int position)
{
    ui->listWidget->setCurrentRow(position);
    QListWidgetItem * item = ui->listWidget->currentItem();
    if(item)
    {
        ui->label_2->setText(item->text());
    }
}

void MainWindow::onDurationChanged(qint64 duration)
{
    ui->horizontalSlider->setMaximum(duration);
    int secs = duration/1000;
    int mins = secs/60;
    secs = secs % 60;
    durationTime = QString::asprintf("%d:%d",mins,secs);
    ui->label->setText(positionTime+"/"+durationTime);
}

void MainWindow::onPositionChanged(qint64 position)
{
    if(ui->horizontalSlider->isSliderDown())
        return;
    ui->horizontalSlider->setSliderPosition(position);
    int secs = position/1000;
    int mins = secs/60;
    secs = secs % 60;
    positionTime = QString::asprintf("%d:%d",mins,secs);
    ui->label->setText(positionTime+"/"+durationTime);
}
void MainWindow::on_horizontalSlider_sliderReleased()
{
     player->setPosition(ui->horizontalSlider->value());
}

void MainWindow::on_pushButton_3_clicked()
{
    M_Amount = playlist->mediaCount();//总数
    if(M_Value == M_Amount)
    {
        M_Value =0;
        playlist->setCurrentIndex(M_Value);
        if(m_IsPause==true)
        {
            player->play();
        }
        return;
    }
    playlist->setCurrentIndex(++M_Value);
    if(m_IsPause==true)
    {
        player->play();
    }
}

void MainWindow::on_pushButton_1_clicked()
{
    M_Amount = playlist->mediaCount();
    if(M_Value == 0)
    {
        M_Value=M_Amount;
        playlist->setCurrentIndex(M_Value);
        if(m_IsPause==true)
        {
            player->play();
        }
        return;
    }
    playlist->setCurrentIndex(--M_Value);
    if(m_IsPause==true)
    {
        player->play();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    if(m_IsPause==true)//false 为播放
    {
        ui->pushButton_2->setStyleSheet("border-image: url(:/lib/1bofang.png);");
        m_IsPause = false;
        player->pause();
        return;
    }
    m_IsPause = true;
    ui->pushButton_2->setStyleSheet("border-image: url(:/lib/1zantingtingzhi.png);");
    player->play();
}

void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    player->setVolume(value);
}

void MainWindow::on_pushButton_5_clicked()
{
    if(m_Volume ==true)
    {
        m_Volume = false;
        ui->pushButton_5->setStyleSheet("border-image: url(:/lib/shengyin.png);");
        player->setVolume(ui->horizontalSlider_2->value());
        return;
    }
    m_Volume = true;
    ui->pushButton_5->setStyleSheet("border-image: url(:/lib/jingyin.png);");
    player->setVolume(0);
}

void MainWindow::on_verticalSlider_valueChanged(int value)
{
    player->setPlaybackRate(value);
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    //获取响应的信息，状态码为200表示正常
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //无错误返回
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();  //获取字节
        QString result(bytes);  //转化为字符串
        //parseJson_getAlbumID(result);  //自定义方法，解析歌曲数据
        ui->lineEdit_3->setText(result);
    }
    else
    {
        //处理错误
        qDebug()<<"处理错误1";
    }
}

void MainWindow::replyFinished2(QNetworkReply *reply)
{
    //获取响应的信息，状态码为200表示正常
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        //无错误返回
        if(reply->error() == QNetworkReply::NoError)
        {
            QByteArray bytes = reply->readAll();  //获取字节
            QString result(bytes);  //转化为字符串

            //parseJson_getplay_url(result);  //自定义方法，解析歌曲数据
            ui->lineEdit_3->setText(result);
        }
        else
        {
            //处理错误
            qDebug()<<"处理错误2";
        }
}

void MainWindow::on_pushButton_6_clicked()
{
    search(ui->lineEdit_3->text());
}
