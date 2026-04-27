#ifndef S21_DEBUG_H
#define S21_DEBUG_H

#ifdef DEBUG
#include <QDebug>
#define DEBUG_PRINT(...) qDebug() << __VA_ARGS__
#define DEBUG_PRINT_FUNC() DEBUG_PRINT(Q_FUNC_INFO)
#else
#define DEBUG_PRINT(...) (void)0
#define DEBUG_PRINT_FUNC() (void)0
#endif

#endif  // S21_DEBUG_H