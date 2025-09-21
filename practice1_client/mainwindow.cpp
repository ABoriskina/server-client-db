#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "serverHandler.hpp"
#include <QMessageBox>
#include <thread>

MainWindow::MainWindow(QWidget *parent) : 
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton_quit, &QPushButton::clicked, this, &QApplication::quit);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_pushButton_register_clicked() {
    QString login = ui->lineEdit_login->text();
    QString password = ui->lineEdit_password->text();
    
    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }

    std::string data = "register:" + login.toStdString() + ":" + password.toStdString();

    std::thread([this, data]() {
        ServerHandler serverHandler;
        int result = serverHandler.startCommunication(data);
        QMetaObject::invokeMethod(this, [this, result]() {
            if (result == DATABASE_USER_FOUND) {
                QMessageBox::information(this, "Успех", "Регистрация прошла успешно!");
            } else {
                QMessageBox::critical(this, "Ошибка", "Ошибка регистрации!");
            }
        });
    }).detach();
}

void MainWindow::on_pushButton_enter_clicked() {
    QString login = ui->lineEdit_login->text();
    QString password = ui->lineEdit_password->text();
    
    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }

    std::string data = "login:" + login.toStdString() + ":" + password.toStdString();

    std::thread([this, data]() {
        ServerHandler serverHandler;
        int result = serverHandler.startCommunication(data);
        QMetaObject::invokeMethod(this, [this, result]() {
            if (result == DATABASE_USER_FOUND) {
                QMessageBox::information(this, "Успех", "Авторизация прошла успешно!");
            } else {
                QMessageBox::critical(this, "Ошибка", "Ошибка авторизации!");
            }
        });
    }).detach();
}