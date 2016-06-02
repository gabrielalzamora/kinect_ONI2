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


#include <QDebug>///-----qDebug()--------------DEBUG
#include <QCloseEvent>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <libfreenect.h>
#include <libusb.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new QGraphicsScene;
    ui->view->setScene(scene);

    rc = openni::OpenNI::initialize();
    if( rc != openni::STATUS_OK){
        qDebug() << "After initialization:\n" << openni::OpenNI::getExtendedError();
    }

    selectDevice();
}

MainWindow::~MainWindow(){
    qDebug() << "MainWindow::~MainWindow()";
    delete[] deviceInfo;
    delete[] m_pTexMap;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event){
    closeK();
    event->accept();
}

void MainWindow::on_pbStart_clicked(){
    qDebug() << "MainWindow::on_pbStart_clicked()";

    rc = device.open( deviceInfo[ui->combo->currentText().toInt()].getUri() );
    if (rc != openni::STATUS_OK){
        qDebug() << "  Device open failed:" << openni::OpenNI::getExtendedError();
        openni::OpenNI::shutdown();
        return;
    }

    rc = color.create(device, openni::SENSOR_COLOR);//--------------------------color.create
    if (rc == openni::STATUS_OK){
        rc = color.start();
        if (rc != openni::STATUS_OK){
            qDebug() << "  pbStart_clicked() Couldn't start color stream";
            qDebug() << openni::OpenNI::getExtendedError();
            color.destroy();
        }else{ ui->pbVideo->setEnabled(true);}
    }else{
        qDebug() << "  pbStart_clicked() Couldn't find color stream";
        qDebug() << openni::OpenNI::getExtendedError();
    }

    rc = depth.create(device, openni::SENSOR_DEPTH);//-------------------------depth.create
    if (rc == openni::STATUS_OK){
        rc = depth.start();
        if (rc != openni::STATUS_OK){
            qDebug() << "  pbStart_clicked() Couldn't start depth stream:";
            qDebug() << openni::OpenNI::getExtendedError();
            depth.destroy();
        }else{ ui->pbDepth->setEnabled(true);}
    }else{
        qDebug() << "  pbStart_clicked() Couldn't find depth stream: ";
        qDebug() << openni::OpenNI::getExtendedError();
    }

    rc = ir.create(device, openni::SENSOR_IR);//--------------------------------ir.create
    qDebug() << "  ir.create adelante";
    if (rc == openni::STATUS_OK){
        rc = ir.start();
        if (rc != openni::STATUS_OK){
            qDebug() << "  pbStart_clicked() couldn't ir.start() stream:";
            qDebug() << openni::OpenNI::getExtendedError();
            depth.destroy();
        }else{ ui->pbIR->setEnabled(true);}
    }else{
        qDebug() << "  pbStart_clicked() Couldn't find IR stream: ";
        qDebug() << openni::OpenNI::getExtendedError();
    }

    bufferInit();
    return;
}

void MainWindow::bufferInit(){
    qDebug() << "MainWindow::bufferInit()";
    openni::VideoMode depthVideoMode;
    openni::VideoMode colorVideoMode;
    openni::VideoMode irVideoMode;

    if ( depth.isValid() && color.isValid() ){
        depthVideoMode = depth.getVideoMode();
        colorVideoMode = color.getVideoMode();
        int depthWidth = depthVideoMode.getResolutionX();
        int depthHeight = depthVideoMode.getResolutionY();
        int colorWidth = colorVideoMode.getResolutionX();
        int colorHeight = colorVideoMode.getResolutionY();

        if (depthWidth == colorWidth &&
            depthHeight == colorHeight){
            m_width = depthWidth;
            m_height = depthHeight;
        }else{
            qDebug() << "  Error - expect color and depth to be in same resolution";
            qDebug() << "  depth w: " <<  depthWidth << " color w: " << colorWidth;
            qDebug() << "  depth h: " <<  depthHeight << " color h: " << colorHeight;
            return;
        }
    }else if (depth.isValid()){
        depthVideoMode = depth.getVideoMode();
        m_width = depthVideoMode.getResolutionX();
        m_height = depthVideoMode.getResolutionY();
    }else if (color.isValid()){
        colorVideoMode = color.getVideoMode();
        m_width = colorVideoMode.getResolutionX();
        m_height = colorVideoMode.getResolutionY();
    }else{
        qDebug() << "  Error - expects at least one of the streams to be valid...";
        return;
    }

    if( ir.isValid()){
        irVideoMode = ir.getVideoMode();
        int irWidth = irVideoMode.getResolutionX();
        int irHeight = irVideoMode.getResolutionY();
        if( irWidth>m_width || irHeight>=m_height ){
            qDebug() << "  PROBLEM IR width or height bigger than video/depth";
            qDebug() << "  ir w: " <<  irWidth;
            qDebug() << "  ir h: " <<  irHeight;
        }
    }

    // map init
    qDebug() << "  m_width = " << m_width;
    qDebug() << "  m_height = " << m_height;
    m_pTexMap = new openni::RGB888Pixel[m_width * m_height];

    return;
}

void MainWindow::on_pbVideo_clicked(){
    qDebug() << "MainWindow::on_pbVideo_clicked()";

    on_pbStop_clicked();//stop other streams
    flag[VIDEO]=1;//allow video loop

    int changedIndex;
    openni::VideoStream** stream;
    stream = new openni::VideoStream*[1];
    stream[0] = &color;

while ( flag[VIDEO] ){

    openni::Status rc = openni::OpenNI::waitForAnyStream(stream, 1, &changedIndex);//1 for single stream in m_streams
    if (rc != openni::STATUS_OK){
        qDebug() << "Wait failed";
        return;
    }

    memset(m_pTexMap, 0, m_width*m_height*sizeof(openni::RGB888Pixel));

    color.readFrame(&m_colorFrame);
    if( m_colorFrame.isValid() ){
        const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_colorFrame.getData();
        openni::RGB888Pixel* pTexRow = m_pTexMap;
        int rowSize = m_colorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel);

        for (int y = 0; y < m_colorFrame.getHeight(); ++y){
            const openni::RGB888Pixel* pImage = pImageRow;
            openni::RGB888Pixel* pTex = pTexRow;

            for (int x = 0; x < m_colorFrame.getWidth(); ++x, ++pImage, ++pTex){
                *pTex = *pImage;
            }
            pImageRow += rowSize;
            pTexRow += m_width;
        }
    }

    ///------ aquí C parece una mierda ¿habrá que copiarlo uno a uno?----> Sí, hay que apuntar byte a byte
    image = QImage((uchar*)m_pTexMap,m_width,m_height,QImage::Format_RGB888);
    scene->addPixmap(QPixmap::fromImage(image));
    ui->view->show();
    m_colorFrame.release();
    qApp->processEvents();//stay responsive to button click

}//end while(flag)

}//end on_pbVideo_clicked()

void MainWindow::on_pbDepth_clicked(){
    qDebug() << "MainWindow::on_pbDepth_clicked()";

    on_pbStop_clicked();//stop other streams
    flag[DEPTH]=1;//allow video loop

    int changedIndex;
    openni::VideoStream** stream;
    stream = new openni::VideoStream*[1];
    stream[0] = &depth;

    memset(m_pTexMap, 0, m_width*m_height*sizeof(openni::RGB888Pixel));

while ( flag[DEPTH] ){

    openni::Status rc = openni::OpenNI::waitForAnyStream(stream, 1, &changedIndex);//1 for single stream in m_streams
    if (rc != openni::STATUS_OK){
        qDebug() << "Wait failed";
        return;
    }

    //memset(m_pTexMap, 0, m_width*m_height*sizeof(openni::RGB888Pixel));

    depth.readFrame(&m_depthFrame);
    if ( m_depthFrame.isValid() ){
        calculateHistogram(m_pDepthHist, MAX_DEPTH, m_depthFrame);

        const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)m_depthFrame.getData();
        openni::RGB888Pixel* pTexRow = m_pTexMap + m_depthFrame.getCropOriginY() * m_width;
        int rowSize = m_depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

        for (int y = 0; y < m_depthFrame.getHeight(); ++y){
            const openni::DepthPixel* pDepth = pDepthRow;
            openni::RGB888Pixel* pTex = pTexRow + m_depthFrame.getCropOriginX();

            for (int x = 0; x < m_depthFrame.getWidth(); ++x, ++pDepth, ++pTex){
                if (*pDepth != 0){
                    int nHistValue = m_pDepthHist[*pDepth];
                    pTex->r = nHistValue;
                    pTex->g = nHistValue;
                    pTex->b = nHistValue;//to obtain gray color
                }
            }
            pDepthRow += rowSize;
            pTexRow += m_width;
        }
    }

    image = QImage((uchar*)m_pTexMap,m_width,m_height,QImage::Format_RGB888);
    scene->addPixmap(QPixmap::fromImage(image));
    ui->view->show();
    m_depthFrame.release();
    qApp->processEvents();//stay responsive to button click
}//end while(flag)

}//end on_pbDepth_clicked()

void MainWindow::on_pb3D_clicked(){

}

void MainWindow::on_pb2D_clicked(){

}

void MainWindow::on_pbIR_clicked(){
    qDebug() << "MainWindow::on_pbIR_clicked()";

    int changedIndex;
    openni::VideoStream** stream;
    stream = new openni::VideoStream*[1];
    stream[0] = &ir;
    openni::Status rc = openni::OpenNI::waitForAnyStream(stream, 1, &changedIndex);//1 for single stream in m_streams
    if (rc != openni::STATUS_OK){
        qDebug() << "Wait failed";
        return;
    }
    qDebug() << "  salimos del wait";

    ir.readFrame(&m_irFrame);
    qDebug() << "  color.readFrame";
    memset(m_pTexMap, 0, m_width*m_height*sizeof(openni::RGB888Pixel));
    qDebug() << "  memset(m_pTexMap, 0, m_width*m_nTexMap";

    if( m_irFrame.isValid() ){
        const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*)m_irFrame.getData();
        openni::RGB888Pixel* pTexRow = m_pTexMap;
        int rowSize = m_irFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel);

        for (int y = 0; y < m_colorFrame.getHeight(); ++y){
            const openni::RGB888Pixel* pImage = pImageRow;
            openni::RGB888Pixel* pTex = pTexRow;

            for (int x = 0; x < m_irFrame.getWidth(); ++x, ++pImage, ++pTex){
                *pTex = *pImage;
            }
            pImageRow += rowSize;
            pTexRow += m_width;
        }
    }

    ///------ aquí C parece una mierda ¿habrá que copiarlo uno a uno?----> Sí, hay que apuntar byte a byte
    image = QImage((uchar*)m_pTexMap,m_width,m_height,QImage::Format_RGB888);
    scene->addPixmap(QPixmap::fromImage(image));
    ui->view->show();
    m_irFrame.release();
}

void MainWindow::on_pbSwept_clicked(){

}

void MainWindow::selectDevice(){
    qDebug() << "MainWindow::selectDevice";

    deviceInfoList = new openni::Array<openni::DeviceInfo>();

    openni::OpenNI::enumerateDevices(deviceInfoList);
    deviceNumber = deviceInfoList->getSize();//DO NOT CALL IT MORE THAN ONCE
    qDebug() << "  openni dice que hay: " << deviceNumber << " deviceNumber";
    deviceInfo = new openni::DeviceInfo[deviceNumber];

    if( deviceNumber == 0 ){
        ui->combo->addItem("No kinect detected");
        scene->addText(" No kinect detected, unable to start");
        ui->view->show();
        ui->pbStart->setEnabled(false);
    }else{
        for( int i = 0; i < deviceNumber ; i++){
            deviceInfo[i] = deviceInfoList->operator [](i);
            QString str;
            ui->combo->addItem(str.setNum(i));
            qDebug() << " " << deviceInfo[i].getUri();
        }
        qDebug() << "  rellenamos combo";
        scene->addText(" Select kinect in combo box to start");
        ui->view->show();
    }

}

void MainWindow::closeK(){
    on_pbStop_clicked();//stop loops
    color.destroy();
    depth.destroy();
    ir.destroy();
    device.close();
    openni::OpenNI::shutdown();
}

void MainWindow::calculateHistogram(float* pHistogram, int histogramSize, const openni::VideoFrameRef& frame){
    const openni::DepthPixel* pDepth = (const openni::DepthPixel*)frame.getData();
    // Calculate the accumulative histogram (the gray display...)
    memset(pHistogram, 0, histogramSize*sizeof(float));
    int restOfRow = frame.getStrideInBytes() / sizeof(openni::DepthPixel) - frame.getWidth();
    int height = frame.getHeight();
    int width = frame.getWidth();

    unsigned int nNumberOfPoints = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x, ++pDepth)
        {
            if (*pDepth != 0)
            {
                pHistogram[*pDepth]++;
                nNumberOfPoints++;
            }
        }
        pDepth += restOfRow;
    }
    for (int nIndex=1; nIndex<histogramSize; nIndex++)
    {
        pHistogram[nIndex] += pHistogram[nIndex-1];
    }
    if (nNumberOfPoints)
    {
        for (int nIndex=1; nIndex<histogramSize; nIndex++)
        {
            pHistogram[nIndex] = (256 * (1.0f - (pHistogram[nIndex] / nNumberOfPoints)));
        }
    }
}

void MainWindow::on_pbStop_clicked(){
    for(int i=0; i<NR_STR;i++)   flag[i] = 0;
}
