:: Set Environment
@echo off
clear

:: compile
g++ main.c -o main -Wall

:: compile success: Run program
if %errorlevel% equ 0 (
    main
    pause
    exit
)

:: compile success: ask for run last program
choice /c yn /t 3 /d n /m "コンパイル失敗しました、前回のプログラムを実行しますか?"
if %errorlevel% equ 1 (
    main
    pause
)