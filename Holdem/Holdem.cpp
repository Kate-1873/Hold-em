// Holdem.cpp : Defines the entry point and UI for the application.
//

#include "framework.h"
#include "Holdem.h"

#include <array>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <cwchar>

#include "pokerEngine.h"

#define MAX_LOADSTRING 100

namespace
{
    constexpr int kCardSlotCount = 7;
    constexpr int kPlayerCardCount = 2;

    enum ControlId : int
    {
        IDC_PLAYERS_EDIT = 2000,
        IDC_ANALYZE_BUTTON,
        IDC_RESET_BUTTON,
        IDC_RANK_BASE = 2100,
        IDC_SUIT_BASE = 2200
    };

    struct AnalysisState
    {
        bool hasResult = false;
        double winChance = 0.0;
        std::wstring currentCombo = L"Waiting for input";
        std::wstring action = L"Choose your cards to begin";
        std::wstring status = L"Select 2 hole cards and any community cards.";
    };

    HINSTANCE g_instance = nullptr;
    HWND g_mainWindow = nullptr;
    HWND g_playersEdit = nullptr;
    HWND g_analyzeButton = nullptr;
    HWND g_resetButton = nullptr;
    std::array<HWND, kCardSlotCount> g_rankCombos{};
    std::array<HWND, kCardSlotCount> g_suitCombos{};

    HFONT g_titleFont = nullptr;
    HFONT g_sectionFont = nullptr;
    HFONT g_bodyFont = nullptr;
    HFONT g_buttonFont = nullptr;
    HBRUSH g_windowBrush = nullptr;
    HBRUSH g_panelBrush = nullptr;
    HBRUSH g_cardBrush = nullptr;
    HBRUSH g_editBrush = nullptr;
    HBRUSH g_buttonBrush = nullptr;

    constexpr COLORREF kBackgroundColor = RGB(8, 26, 22);
    constexpr COLORREF kPanelColor = RGB(18, 48, 41);
    constexpr COLORREF kCardColor = RGB(248, 241, 231);
    constexpr COLORREF kCardBorderColor = RGB(212, 195, 168);
    constexpr COLORREF kTextColor = RGB(241, 232, 213);
    constexpr COLORREF kMutedTextColor = RGB(178, 186, 174);
    constexpr COLORREF kAccentColor = RGB(214, 173, 96);

    AnalysisState g_analysisState;

    const std::array<const wchar_t*, 14> kRankItems = {
        L"-", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"10", L"J", L"Q", L"K", L"A"
    };

    const std::array<const wchar_t*, 5> kSuitItems = {
        L"-", L"Spades", L"Hearts", L"Diamonds", L"Clubs"
    };

    const std::array<const wchar_t*, kCardSlotCount> kSlotLabels = {
        L"Player Card 1",
        L"Player Card 2",
        L"Flop Card 1",
        L"Flop Card 2",
        L"Flop Card 3",
        L"Turn",
        L"River"
    };

    RECT MakeRect(int left, int top, int right, int bottom)
    {
        RECT rect{ left, top, right, bottom };
        return rect;
    }

    HMENU ControlHandle(int id)
    {
        return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id));
    }

    std::wstring ToWide(const std::string& value)
    {
        if (value.empty())
        {
            return L"";
        }

        int count = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
        if (count <= 0)
        {
            return std::wstring(value.begin(), value.end());
        }

        std::wstring result(static_cast<size_t>(count), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), count);
        result.resize(static_cast<size_t>(count - 1));
        return result;
    }

    std::wstring FormatPercent(double value)
    {
        wchar_t buffer[32];
        swprintf_s(buffer, L"%.1f%%", value);
        return buffer;
    }

    std::wstring GetActionLabel(double winChance)
    {
        if (winChance > 60.0)
        {
            return L"Raise";
        }
        if (winChance > 30.0)
        {
            return L"Call";
        }
        return L"Fold";
    }

    std::wstring GetStatusFromAction(double winChance)
    {
        if (winChance > 60.0)
        {
            return L"Strong position. You can lean aggressive.";
        }
        if (winChance > 30.0)
        {
            return L"Playable spot. Continue with discipline.";
        }
        return L"Risky spot. Preserve chips and wait for better equity.";
    }

    void ApplyFont(HWND control, HFONT font)
    {
        SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    }

    template <size_t N>
    void AddComboItems(HWND combo, const std::array<const wchar_t*, N>& values)
    {
        for (const auto* value : values)
        {
            SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(value));
        }
        SendMessageW(combo, CB_SETCURSEL, 0, 0);
    }

    void DrawLabel(HDC hdc, const std::wstring& text, RECT rect, HFONT font, COLORREF color, UINT format = DT_LEFT | DT_TOP | DT_NOPREFIX)
    {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, color);
        HFONT previousFont = static_cast<HFONT>(SelectObject(hdc, font));
        DrawTextW(hdc, text.c_str(), -1, &rect, format);
        SelectObject(hdc, previousFont);
    }

    void DrawRoundedPanel(HDC hdc, const RECT& rect, HBRUSH brush, COLORREF borderColor)
    {
        HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
        HPEN previousPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH previousBrush = static_cast<HBRUSH>(SelectObject(hdc, brush));
        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 26, 26);
        SelectObject(hdc, previousBrush);
        SelectObject(hdc, previousPen);
        DeleteObject(pen);
    }

    std::optional<Card> ReadSelectedCard(int index)
    {
        const int rankSelection = static_cast<int>(SendMessageW(g_rankCombos[index], CB_GETCURSEL, 0, 0));
        const int suitSelection = static_cast<int>(SendMessageW(g_suitCombos[index], CB_GETCURSEL, 0, 0));

        if (rankSelection <= 0 && suitSelection <= 0)
        {
            return std::nullopt;
        }

        if (rankSelection <= 0 || suitSelection <= 0)
        {
            return Card{ static_cast<Rank>(0), static_cast<Suit>(0) };
        }

        return Card{
            static_cast<Rank>(rankSelection + 1),
            static_cast<Suit>(suitSelection - 1)
        };
    }

    std::wstring GetCardFace(const Card& card)
    {
        const int rankValue = static_cast<int>(card.rank);
        std::wstring rankLabel;
        if (rankValue >= 2 && rankValue <= 10)
        {
            rankLabel = std::to_wstring(rankValue);
        }
        else if (rankValue == 11)
        {
            rankLabel = L"J";
        }
        else if (rankValue == 12)
        {
            rankLabel = L"Q";
        }
        else if (rankValue == 13)
        {
            rankLabel = L"K";
        }
        else
        {
            rankLabel = L"A";
        }

        switch (card.suit)
        {
        case Suit::Spades:
            return rankLabel + L" S";
        case Suit::Hearts:
            return rankLabel + L" H";
        case Suit::Diamonds:
            return rankLabel + L" D";
        case Suit::Clubs:
            return rankLabel + L" C";
        }

        return rankLabel;
    }

    void DrawBoardCard(HDC hdc, const RECT& rect, const std::wstring& face, bool isAccent)
    {
        HPEN pen = CreatePen(PS_SOLID, isAccent ? 2 : 1, isAccent ? kAccentColor : kCardBorderColor);
        HPEN previousPen = static_cast<HPEN>(SelectObject(hdc, pen));
        HBRUSH previousBrush = static_cast<HBRUSH>(SelectObject(hdc, g_cardBrush));
        RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 20, 20);
        SelectObject(hdc, previousBrush);
        SelectObject(hdc, previousPen);
        DeleteObject(pen);

        DrawLabel(
            hdc,
            face,
            MakeRect(rect.left + 12, rect.top + 12, rect.right - 12, rect.bottom - 12),
            g_sectionFont,
            RGB(28, 34, 37),
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    void ResetSelections()
    {
        SetWindowTextW(g_playersEdit, L"6");
        for (int i = 0; i < kCardSlotCount; ++i)
        {
            SendMessageW(g_rankCombos[i], CB_SETCURSEL, 0, 0);
            SendMessageW(g_suitCombos[i], CB_SETCURSEL, 0, 0);
        }

        g_analysisState = {};
        InvalidateRect(g_mainWindow, nullptr, TRUE);
    }

    bool TryAnalyze()
    {
        wchar_t playersText[16]{};
        GetWindowTextW(g_playersEdit, playersText, static_cast<int>(std::size(playersText)));
        const int players = _wtoi(playersText);

        if (players < 2 || players > 10)
        {
            g_analysisState.hasResult = false;
            g_analysisState.status = L"Players must be between 2 and 10.";
            InvalidateRect(g_mainWindow, nullptr, TRUE);
            return false;
        }

        std::vector<Card> selectedCards;
        std::vector<Card> playerCards;
        std::vector<Card> boardCards;

        for (int i = 0; i < kCardSlotCount; ++i)
        {
            auto card = ReadSelectedCard(i);
            if (!card.has_value())
            {
                continue;
            }

            if (static_cast<int>(card->rank) == 0)
            {
                g_analysisState.hasResult = false;
                g_analysisState.status = L"Each card needs both rank and suit.";
                InvalidateRect(g_mainWindow, nullptr, TRUE);
                return false;
            }

            selectedCards.push_back(*card);
            if (i < kPlayerCardCount)
            {
                playerCards.push_back(*card);
            }
            else
            {
                boardCards.push_back(*card);
            }
        }

        if (playerCards.size() != kPlayerCardCount)
        {
            g_analysisState.hasResult = false;
            g_analysisState.status = L"Select exactly 2 player cards.";
            InvalidateRect(g_mainWindow, nullptr, TRUE);
            return false;
        }

        std::set<std::pair<int, int>> seenCards;
        for (const auto& card : selectedCards)
        {
            auto key = std::make_pair(static_cast<int>(card.rank), static_cast<int>(card.suit));
            if (!seenCards.insert(key).second)
            {
                g_analysisState.hasResult = false;
                g_analysisState.status = L"Duplicate cards detected. Each card must be unique.";
                InvalidateRect(g_mainWindow, nullptr, TRUE);
                return false;
            }
        }

        PokerEngine engine(players);
        for (const auto& card : playerCards)
        {
            engine.addPlayerCard(card.rank, card.suit);
        }

        for (const auto& card : boardCards)
        {
            engine.addTableCard(card.rank, card.suit);
        }

        const auto result = engine.getFullAnalysis();
        g_analysisState.hasResult = true;
        g_analysisState.winChance = result.winChance;
        g_analysisState.currentCombo = ToWide(result.currentCombo);
        g_analysisState.action = GetActionLabel(result.winChance);
        g_analysisState.status = GetStatusFromAction(result.winChance);
        InvalidateRect(g_mainWindow, nullptr, TRUE);
        return true;
    }

    void CreateApplicationControls(HWND hwnd)
    {
        g_playersEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"6",
            WS_CHILD | WS_VISIBLE | ES_CENTER | ES_AUTOHSCROLL,
            84,
            172,
            84,
            34,
            hwnd,
            ControlHandle(IDC_PLAYERS_EDIT),
            g_instance,
            nullptr);

        g_analyzeButton = CreateWindowW(
            L"BUTTON",
            L"Analyze",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            28,
            690,
            150,
            42,
            hwnd,
            ControlHandle(IDC_ANALYZE_BUTTON),
            g_instance,
            nullptr);

        g_resetButton = CreateWindowW(
            L"BUTTON",
            L"Reset",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            28,
            742,
            150,
            42,
            hwnd,
            ControlHandle(IDC_RESET_BUTTON),
            g_instance,
            nullptr);

        for (int i = 0; i < kCardSlotCount; ++i)
        {
            const int top = 250 + i * 58;
            g_rankCombos[i] = CreateWindowW(
                L"COMBOBOX",
                L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                28,
                top,
                68,
                220,
                hwnd,
                ControlHandle(IDC_RANK_BASE + i),
                g_instance,
                nullptr);

            g_suitCombos[i] = CreateWindowW(
                L"COMBOBOX",
                L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                104,
                top,
                108,
                220,
                hwnd,
                ControlHandle(IDC_SUIT_BASE + i),
                g_instance,
                nullptr);

            AddComboItems(g_rankCombos[i], kRankItems);
            AddComboItems(g_suitCombos[i], kSuitItems);
        }

        ApplyFont(g_playersEdit, g_bodyFont);
        ApplyFont(g_analyzeButton, g_buttonFont);
        ApplyFont(g_resetButton, g_buttonFont);

        for (int i = 0; i < kCardSlotCount; ++i)
        {
            ApplyFont(g_rankCombos[i], g_bodyFont);
            ApplyFont(g_suitCombos[i], g_bodyFont);
        }
    }

    void DrawLeftPanel(HDC hdc)
    {
        RECT panel = MakeRect(16, 16, 226, 798);
        DrawRoundedPanel(hdc, panel, g_panelBrush, RGB(31, 75, 67));

        DrawLabel(hdc, L"Hold'em", MakeRect(28, 28, 196, 68), g_titleFont, kTextColor);
        DrawLabel(hdc, L"Advisor", MakeRect(28, 66, 196, 104), g_titleFont, kAccentColor);
        DrawLabel(hdc, L"Fast local poker analysis with a cleaner table view.", MakeRect(28, 116, 196, 156), g_bodyFont, kMutedTextColor, DT_WORDBREAK);
        DrawLabel(hdc, L"Players", MakeRect(28, 176, 136, 204), g_sectionFont, kTextColor);
        DrawLabel(hdc, L"Cards", MakeRect(28, 218, 136, 246), g_sectionFont, kTextColor);

        for (int i = 0; i < kCardSlotCount; ++i)
        {
            const int top = 252 + i * 58;
            DrawLabel(hdc, kSlotLabels[i], MakeRect(28, top - 22, 204, top), g_bodyFont, kMutedTextColor);
        }
    }

    void DrawTableScene(HDC hdc)
    {
        RECT boardArea = MakeRect(250, 16, 870, 798);
        DrawRoundedPanel(hdc, boardArea, g_panelBrush, RGB(35, 87, 76));

        HBRUSH feltBrush = CreateSolidBrush(RGB(22, 78, 58));
        HPEN feltPen = CreatePen(PS_SOLID, 2, RGB(183, 144, 77));
        HPEN previousPen = static_cast<HPEN>(SelectObject(hdc, feltPen));
        HBRUSH previousBrush = static_cast<HBRUSH>(SelectObject(hdc, feltBrush));
        RoundRect(hdc, 324, 174, 796, 516, 200, 200);
        SelectObject(hdc, previousBrush);
        SelectObject(hdc, previousPen);
        DeleteObject(feltBrush);
        DeleteObject(feltPen);

        DrawLabel(hdc, L"Board", MakeRect(510, 120, 620, 160), g_sectionFont, kTextColor, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DrawLabel(hdc, L"Your Hand", MakeRect(470, 550, 650, 590), g_sectionFont, kTextColor, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        for (int i = 0; i < 5; ++i)
        {
            RECT cardRect = MakeRect(375 + i * 78, 285, 441 + i * 78, 392);
            auto card = ReadSelectedCard(i + 2);
            DrawBoardCard(hdc, cardRect, card.has_value() && static_cast<int>(card->rank) != 0 ? GetCardFace(*card) : L"--", false);
        }

        for (int i = 0; i < 2; ++i)
        {
            RECT cardRect = MakeRect(470 + i * 92, 610, 548 + i * 92, 724);
            auto card = ReadSelectedCard(i);
            DrawBoardCard(hdc, cardRect, card.has_value() && static_cast<int>(card->rank) != 0 ? GetCardFace(*card) : L"--", true);
        }
    }

    void DrawRightPanel(HDC hdc)
    {
        RECT panel = MakeRect(896, 16, 1260, 798);
        DrawRoundedPanel(hdc, panel, g_panelBrush, RGB(31, 75, 67));

        DrawLabel(hdc, L"Analysis", MakeRect(920, 28, 1180, 64), g_titleFont, kTextColor);
        DrawLabel(hdc, L"Current read", MakeRect(920, 92, 1180, 120), g_sectionFont, kMutedTextColor);
        DrawLabel(hdc, g_analysisState.currentCombo, MakeRect(920, 124, 1220, 172), g_sectionFont, kTextColor, DT_WORDBREAK);
        DrawLabel(hdc, L"Win chance", MakeRect(920, 208, 1180, 236), g_sectionFont, kMutedTextColor);
        DrawLabel(hdc, g_analysisState.hasResult ? FormatPercent(g_analysisState.winChance) : L"--", MakeRect(920, 240, 1220, 324), g_titleFont, kAccentColor);
        DrawLabel(hdc, L"Recommended move", MakeRect(920, 370, 1180, 398), g_sectionFont, kMutedTextColor);
        DrawLabel(hdc, g_analysisState.action, MakeRect(920, 404, 1220, 450), g_sectionFont, kTextColor);
        DrawLabel(hdc, L"Table note", MakeRect(920, 504, 1180, 532), g_sectionFont, kMutedTextColor);
        DrawLabel(hdc, g_analysisState.status, MakeRect(920, 540, 1220, 640), g_bodyFont, kTextColor, DT_WORDBREAK);
        DrawLabel(hdc, L"Tip: leave Turn and River empty to estimate future equity.", MakeRect(920, 700, 1215, 756), g_bodyFont, kMutedTextColor, DT_WORDBREAK);
    }

    void InitializeThemeResources()
    {
        g_titleFont = CreateFontW(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        g_sectionFont = CreateFontW(22, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        g_bodyFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        g_buttonFont = CreateFontW(18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

        g_windowBrush = CreateSolidBrush(kBackgroundColor);
        g_panelBrush = CreateSolidBrush(kPanelColor);
        g_cardBrush = CreateSolidBrush(kCardColor);
        g_editBrush = CreateSolidBrush(RGB(237, 229, 214));
        g_buttonBrush = CreateSolidBrush(RGB(214, 173, 96));
    }

    void DestroyThemeResources()
    {
        DeleteObject(g_titleFont);
        DeleteObject(g_sectionFont);
        DeleteObject(g_bodyFont);
        DeleteObject(g_buttonFont);
        DeleteObject(g_windowBrush);
        DeleteObject(g_panelBrush);
        DeleteObject(g_cardBrush);
        DeleteObject(g_editBrush);
        DeleteObject(g_buttonBrush);
    }
}

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_instance = hInstance;
    InitializeThemeResources();

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HOLDEM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        DestroyThemeResources();
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOLDEM));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DestroyThemeResources();
    return static_cast<int>(msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOLDEM));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = g_windowBrush;
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_HOLDEM);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(
        szWindowClass,
        L"Hold'em Advisor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        1300,
        860,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    g_mainWindow = hWnd;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateApplicationControls(hWnd);
        ResetSelections();
        return 0;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            return 0;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            return 0;
        case IDC_ANALYZE_BUTTON:
            TryAnalyze();
            return 0;
        case IDC_RESET_BUTTON:
            ResetSelections();
            return 0;
        default:
            break;
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetTextColor(hdc, kTextColor);
        SetBkColor(hdc, kPanelColor);
        return reinterpret_cast<INT_PTR>(g_panelBrush);
    }

    case WM_CTLCOLOREDIT:
    {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetTextColor(hdc, RGB(23, 32, 31));
        SetBkColor(hdc, RGB(237, 229, 214));
        return reinterpret_cast<INT_PTR>(g_editBrush);
    }

    case WM_CTLCOLORBTN:
    {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        SetTextColor(hdc, RGB(22, 30, 29));
        SetBkColor(hdc, RGB(214, 173, 96));
        return reinterpret_cast<INT_PTR>(g_buttonBrush);
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT clientRect{};
        GetClientRect(hWnd, &clientRect);
        FillRect(hdc, &clientRect, g_windowBrush);

        DrawLeftPanel(hdc);
        DrawTableScene(hdc);
        DrawRightPanel(hdc);

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return static_cast<INT_PTR>(TRUE);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return static_cast<INT_PTR>(TRUE);
        }
        break;
    }
    return static_cast<INT_PTR>(FALSE);
}
