#include <QApplication>
#include "controller/AppController.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    s21::mlp::AppController controller;
    controller.show();
    
    return app.exec();
}
