/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QComboBox>

#include <OpenNI.h>

#define H 640
#define W 480
#define MAX_DEPTH 10000
#define NR_STR 7

enum DisplayModes
{
    DISPLAY_MODE_OVERLAY,
    DISPLAY_MODE_DEPTH,
    DISPLAY_MODE_IMAGE
};

enum KindStream{
    VIDEO, DEPTH, THREE, TWO, IR, SWEPT, ACCEL
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

signals:

private slots:
    void on_pbStart_clicked();
    void bufferInit();
    void on_pbVideo_clicked();
    void on_pbDepth_clicked();
    void on_pb3D_clicked();
    void on_pb2D_clicked();
    void on_pbIR_clicked();
    void on_pbSwept_clicked();
    void selectDevice();
    void closeK();
    void calculateHistogram(float* pHistogram, int histogramSize, const openni::VideoFrameRef& frame);

    void on_pbStop_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    int flag[NR_STR];//if device is running exit orderly (=1 run/ =0 exit)
    QImage image;

    openni::Status rc;//for error control
    //const char* deviceURI;
    openni::Array<openni::DeviceInfo>* deviceInfoList;//connected kinects list
    openni::DeviceInfo* deviceInfo;
    int deviceNumber;//number of connected kinects
    openni::Device device;
    openni::VideoStream depth, color, ir;
    openni::VideoStream** m_streams;
    openni::VideoFrameRef m_depthFrame;
    openni::VideoFrameRef m_colorFrame;
    openni::VideoFrameRef m_irFrame;
    int	m_width, m_height;
    unsigned int m_nTexMapX;
    unsigned int m_nTexMapY;
    DisplayModes m_eViewState;
    openni::RGB888Pixel* m_pTexMap;
    float m_pDepthHist[MAX_DEPTH];

};

#endif // MAINWINDOW_H
