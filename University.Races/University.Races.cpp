// University.Races.cpp : Определяет точку входа для приложения.
//

#include <windows.h>
#include <tchar.h>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include "framework.h"
#include "University.Races.h"
#include <mutex>
#include <string>
#include <sstream>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

#define MAX_LOADSTRING 100

#define NUM_RACERS 5
#define FINISH_LINE 100
#define START_X 50
#define BUTTON_START 1
#define BUTTON_STOP 2
#define BUTTON_REFRESH 3

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

struct Racer {
    int position;
    int id;
    bool finished;
    int place;
    Image* image;
};

std::vector<Racer> racers(NUM_RACERS);
std::vector<std::thread> threads;
std::atomic<bool> raceOver(false);
std::mutex drawMutex;
std::atomic<bool> raceRunning(false);
std::vector<int> results;
Gdiplus::Image* racerImage;
Gdiplus::GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
HWND hWnd;
HWND hStartButton, hStopButton;
HBITMAP hRacerBitmap;

void LoadRacerImages() {
    std::vector<std::wstring> imageFiles = {
        L"big_data_racer.png",
        L"theory_programming_languages_racer.png",
        L"project_activities_racer.png",
        L"pattern_designs_racer.png",
        L"theory_computational_processes_racer.png"
    };

    for (int i = 0; i < NUM_RACERS; i++) {
        if (i < imageFiles.size()) {
            racers[i].image = new Image(imageFiles[i].c_str());
            if (racers[i].image->GetLastStatus() != Ok) {
                MessageBox(NULL, (L"Ошибка загрузки " + imageFiles[i]).c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
            }
        }
    }
}

void Race(int index) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 10);
    std::uniform_int_distribution<int> sleepDist(500, 1500);

    while (true) {
        if (!raceRunning) return;  // Если гонка остановлена — завершаем поток

        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDist(gen)));

        if (!racers[index].finished) {
            racers[index].position += dist(gen);
            if (racers[index].position >= FINISH_LINE) {
                racers[index].position = FINISH_LINE;
                racers[index].finished = true;
                racers[index].place = results.size() + 1;
                results.push_back(racers[index].id);
            }
        }

        InvalidateRect(hWnd, NULL, TRUE);

        bool allFinished = true;
        for (const auto& racer : racers) {
            if (!racer.finished) {
                allFinished = false;
                break;
            }
        }

        if (allFinished) {
            raceRunning = false;
            return;  // Завершаем поток
        }
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    hWnd = CreateWindowEx(0, _T("RaceWindow"), _T("Cockroach Race"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 300, NULL, NULL, hInstance, NULL);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_UNIVERSITYRACES, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UNIVERSITYRACES));

    for (int i = 0; i < NUM_RACERS; i++) {
        threads.emplace_back(Race, i);
    }

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    //for (auto& t : threads) {
    //    t.join();
    //}

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UNIVERSITYRACES));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_UNIVERSITYRACES);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:

        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        LoadRacerImages();

        hStartButton = CreateWindow(L"BUTTON", L"Старт", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            820, 10, 80, 30, hWnd, (HMENU)BUTTON_START, GetModuleHandle(NULL), NULL);
        hStopButton = CreateWindow(L"BUTTON", L"Стоп", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            900, 10, 80, 30, hWnd, (HMENU)BUTTON_STOP, GetModuleHandle(NULL), NULL);
        hStopButton = CreateWindow(L"BUTTON", L"Заново", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            980, 10, 80, 30, hWnd, (HMENU)BUTTON_REFRESH, GetModuleHandle(NULL), NULL);
        break;
    case WM_COMMAND:
        {

            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case BUTTON_START:
                raceRunning = true;
                break;
            case BUTTON_REFRESH:
                if (!raceRunning) {
                    raceRunning = true;

                    // Очистка списка потоков перед перезапуском
                    for (auto& t : threads) {
                        if (t.joinable()) t.join();
                    }
                    threads.clear();

                    results.clear();
                    for (int i = 0; i < NUM_RACERS; i++) {
                        racers[i].position = 0;
                        racers[i].finished = false;
                        racers[i].place = 0;
                        threads.emplace_back(Race, i);
                    }
                    InvalidateRect(hWnd, NULL, TRUE); // Принудительное обновление окна
                }
                break;
            case BUTTON_STOP:
                raceRunning = false;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            Graphics graphics(hdc);

            TextOut(hdc, START_X - 20, 20, L"СТАРТ", 5);

            MoveToEx(hdc, START_X, 40, NULL);
            LineTo(hdc, START_X, 40 + NUM_RACERS * 80);

            int finishX = START_X + (FINISH_LINE) * 5 + 210;
            TextOut(hdc, finishX - 30, 20, L"ДЕДЛАЙН", 7);

            // Рисуем линию финиша
            MoveToEx(hdc, START_X + (FINISH_LINE) * 5 + 216, 40, NULL);
            LineTo(hdc, START_X + (FINISH_LINE) * 5 + 216, 40 + NUM_RACERS * 80);

            for (int i = 0; i < NUM_RACERS; i++) {
                int x = START_X + racers[i].position * 5;
                int y = 50 + i * 80;
                graphics.DrawImage(racers[i].image, x, y, 216, 39);

                std::wstring ws;
                if (racers[i].finished) {
                    ws += std::to_wstring(racers[i].place) + L" place";
                }
                TextOut(hdc, START_X + 10, y + 10, ws.c_str(), ws.length());
            }


            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        raceRunning = false;

        // Завершаем потоки корректно
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
        for (auto& racer : racers) {
            delete racer.image;
        }
        Gdiplus::GdiplusShutdown(gdiplusToken);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
