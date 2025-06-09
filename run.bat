:: Set Environment
@echo off
clear

:: Compile
gcc main.c -o main -Wall -O3 -s

:: Compile Success: Run program
if %errorlevel% equ 0 (
    main
    pause
    exit
)

:: Compile Failure: Ask for run last program
choice /c yn /t 3 /d n /m "コンパイル失敗しました、前回のプログラムを実行しますか?"
if %errorlevel% equ 1 (
    main
    pause
)