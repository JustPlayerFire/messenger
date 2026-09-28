#-------------------------------------------------
#
# Project created by QtCreator 2026-01-14T15:03:19
#
#-------------------------------------------------

QT       += core gui network sql

TARGET = server_messenger
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    server.cpp \
    database.cpp \
    userrepository.cpp

HEADERS += \
    server.h \
    database.h \
    userrepository.h
