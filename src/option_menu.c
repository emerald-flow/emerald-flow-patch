#include "global.h"
#include "option_menu.h"
#include "sound.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"
#include "event_data.h"

#define Y(option_name) gText_##option_name
#define M(name) MENUITEM_##name

#define MENUITEM_MAX 7

enum
{
    WindowFrameType,
    COUNT,
    MAX
};

#define M_ENUMS(name, ...) M(name), 
enum
{
    OPTIONS(M_ENUMS)
    M(WindowFrameType),
    M(COUNT),
};
#undef M_ENUMS

#define tWindowFrameType data[MENUITEM_MAX]
#define tMenuSelection data[NUM_TASKS+1]
#define tMenuOffset data[MENUITEM_MAX+2]

enum
{
    WIN_HEADER,
    WIN_OPTIONS
};

static void Task_OptionMenuFadeIn(u8 taskId);
static void Task_OptionMenuProcessInput(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
static void HighlightOptionMenuItem(u8 selection);
static bool8 checkInWindow(u8 pos, u8 offset);
static void DrawChoices3(u8 menuitem, u8 taskId, s16 block, const u8 *a, const u8 *b, const u8 *c);
static void DrawChoices2(u8 menuitem, u8 taskId, s16 block, const u8 *a, const u8 *b);
static u8 ProcessInput3(u8 selection);
static u8 ProcessInput2(u8 selection);
static u8 WindowFrameType_ProcessInput(u8 selection);
static void WindowFrameType_DrawChoices(u8 taskId, s16 block);
static void DrawHeaderText(void);
static void DrawOptionMenuTexts(void);
static void DrawBgWindowFrames(void);
static void ReDrawMenu(u8 taskId);

EWRAM_DATA static bool8 sArrowPressed = FALSE;

static const u16 sOptionMenuText_Pal[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCGFX_U8("graphics/interface/option_menu_equals_sign.png", ".4bpp");

#define OPTIONS_MENU_ITEM_NAMES(name, ...) [M(name)] = Y(name),
static const u8 *const sOptionMenuItemsNames[M(COUNT)] =
{
    OPTIONS(OPTIONS_MENU_ITEM_NAMES)
    [M(WindowFrameType)]   = Y(WindowFrameType),
};
#undef OPTIONS_MENU_ITEM_NAMES

static const struct WindowTemplate sOptionMenuWinTemplates[] =
{
    [WIN_HEADER] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [WIN_OPTIONS] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 0x36
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOptionMenuBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_InitOptionMenu(void)
{
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        ChangeBgX(3, 0, BG_COORD_SET);
        ChangeBgY(3, 0, BG_COORD_SET);
        InitWindows(sOptionMenuWinTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        gMain.state++;
        break;
    case 4:
        LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(0), sizeof(sOptionMenuBg_Pal));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 5:
        LoadPalette(sOptionMenuText_Pal, BG_PLTT_ID(1), sizeof(sOptionMenuText_Pal));
        gMain.state++;
        break;
    case 6:
        PutWindowTilemap(WIN_HEADER);
        DrawHeaderText();
        gMain.state++;
        break;
    case 7:
        gMain.state++;
        break;
    case 8:
        PutWindowTilemap(WIN_OPTIONS);
        DrawOptionMenuTexts();
        gMain.state++;
    // FALLTHROUGH
    case 9:
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 10:
    {
        u8 taskId = CreateTask(Task_OptionMenuFadeIn, 0);

        gTasks[taskId].tMenuSelection = 0;
        gTasks[taskId].tMenuOffset = 0;
        gTasks[taskId].tWindowFrameType = gSaveBlock2Ptr->optionsWindowFrameType;

        ReDrawMenu(taskId);

        gMain.state++;
        break;
    }
    case 11:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        return;
    }
}

static void Task_OptionMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_OptionMenuProcessInput;
}

static void Task_OptionMenuProcessInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_OptionMenuFadeOut;
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gTasks[taskId].tMenuSelection > 0)
            gTasks[taskId].tMenuSelection--;
        else if((gTasks[taskId].tMenuSelection == 0) && (gTasks[taskId].tMenuOffset == 0))
        {
            gTasks[taskId].tMenuSelection = M(MAX) - 1;
            gTasks[taskId].tMenuOffset = M(COUNT) - M(MAX);
            ReDrawMenu(taskId);
        }
        else 
        {
            gTasks[taskId].tMenuOffset--;
            ReDrawMenu(taskId);
        }
        HighlightOptionMenuItem(gTasks[taskId].tMenuSelection);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (gTasks[taskId].tMenuSelection < M(MAX) - 1)
            gTasks[taskId].tMenuSelection++;
        else if ((gTasks[taskId].tMenuSelection + gTasks[taskId].tMenuOffset) == M(COUNT) - 1)
        {
            gTasks[taskId].tMenuSelection = 0;
            gTasks[taskId].tMenuOffset = 0;
            ReDrawMenu(taskId);
        }
        else
        {
            gTasks[taskId].tMenuOffset++;
            ReDrawMenu(taskId);
        }
        HighlightOptionMenuItem(gTasks[taskId].tMenuSelection);
    }
    else
    {
        u8 previousOption;
        u8 selection = gTasks[taskId].tMenuSelection + gTasks[taskId].tMenuOffset;
        u8 pos;
        #define Z(func, ...)  func(__VA_ARGS__ __VA_OPT__(,) gTasks[taskId].data[pos])
        switch (selection)
        {
        #define CASES(name, count, callback, ...) \
        case M(name): \
        { \
            pos = M(name) - gTasks[taskId].tMenuOffset; \
            previousOption = gTasks[taskId].data[pos]; \
            gTasks[taskId].data[pos] = ProcessInput##count(gTasks[taskId].data[pos]); \
            gSaveBlock2Ptr->options##name = gTasks[taskId].data[pos]; \
            callback; \
            DrawChoices##count(M(name), taskId, gSaveBlock2Ptr->options##name, __VA_ARGS__); \
            break; \
        } 
        OPTIONS(CASES)
        case M(WindowFrameType):

            previousOption = gTasks[taskId].tWindowFrameType;
            gTasks[taskId].tWindowFrameType = WindowFrameType_ProcessInput(gTasks[taskId].tWindowFrameType);
            gSaveBlock2Ptr->optionsWindowFrameType = gTasks[taskId].tWindowFrameType;

            WindowFrameType_DrawChoices(taskId, gSaveBlock2Ptr->optionsWindowFrameType);
            break;
        default:
            return;
        #undef CASES
        }
        #undef Z
        if (sArrowPressed)
        {
            sArrowPressed = FALSE;
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        }
    }
}

static void Task_OptionMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

static void HighlightOptionMenuItem(u8 index)
{
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 40, index * 16 + 56));
}

static void DrawOptionMenuChoice(const u8 *text, u8 x, u8 y, u8 style)
{
    u8 dst[16];
    u16 i;

    for (i = 0; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);

    if (style != 0)
    {
        dst[2] = TEXT_COLOR_RED;
        dst[5] = TEXT_COLOR_LIGHT_RED;
    }

    dst[i] = EOS;
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, dst, x, y + 1, TEXT_SKIP_DRAW, NULL);
}

static u8 getPosition(u8 menuitem, u8 offset)
{
    return menuitem - offset;
}

static bool8 checkInWindow(u8 pos, u8 offset)
{
    if((pos < offset)||((offset + M(MAX) - 1) < pos))
        return FALSE;
    return TRUE;
}

static void DrawChoices3(u8 menuitem, u8 taskId, s16 block, const u8 *a, const u8 *b, const u8 *c)
{
    u8 styles[3];
    u8 offset = gTasks[taskId].tMenuOffset;
    u8 pos = menuitem - offset;
    u8 ypos = pos * 16;
    s32 widthA, widthB, widthC, x2;

    if(!checkInWindow(menuitem, offset))
        return;

    gTasks[taskId].data[pos] = block;

    styles[0] = 0;
    styles[1] = 0;
    styles[2] = 0;
    styles[gTasks[taskId].data[pos]] = 1;

    DrawOptionMenuChoice(a, 104, ypos, styles[0]);

    widthA = GetStringWidth(FONT_NORMAL, a, 0);
    widthB = GetStringWidth(FONT_NORMAL, b, 0);
    widthC = GetStringWidth(FONT_NORMAL, c, 0);

    widthB -= 94;
    x2 = (widthA - widthB - widthC) / 2 + 104;
    DrawOptionMenuChoice(b, x2, ypos, styles[1]);

    DrawOptionMenuChoice(c, GetStringRightAlignXOffset(FONT_NORMAL, c, 198), ypos, styles[2]);
}

static void DrawChoices2(u8 menuitem, u8 taskId, s16 block, const u8 *a, const u8 *b)
{       
    u8 styles[2];
    u8 offset = gTasks[taskId].tMenuOffset;
    u8 pos = menuitem - offset;
    u8 ypos = (menuitem - offset) * 16;

    if(!checkInWindow(menuitem, offset))
        return;

    gTasks[taskId].data[pos] = block;

    styles[0] = 0;
    styles[1] = 0;
    styles[gTasks[taskId].data[pos]] = 1;

    DrawOptionMenuChoice(a, 104, ypos, styles[0]);
    DrawOptionMenuChoice(b, GetStringRightAlignXOffset(FONT_NORMAL, b, 198), ypos, styles[1]);
}

static u8 ProcessInput3(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection <= 1)
            selection++;
        else
            selection = 0;

        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = 2;

        sArrowPressed = TRUE;
    }
    return selection;
}

static u8 ProcessInput2(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void WindowFrameType_DrawChoices(u8 taskId, s16 block)
{
    u8 text[16];
    u8 offset = gTasks[taskId].tMenuOffset;
    u8 ypos =  (M(WindowFrameType) - offset) * 16;
    u8 n = gTasks[taskId].tWindowFrameType + 1;
    u16 i;

    if(!checkInWindow(M(WindowFrameType), offset))
        return;

    gTasks[taskId].tWindowFrameType = block;

    for (i = 0; gText_FrameTypeNumber[i] != EOS && i <= 5; i++)
        text[i] = gText_FrameTypeNumber[i];

    // Convert a number to decimal string
    if (n / 10 != 0)
    {
        text[i] = n / 10 + CHAR_0;
        i++;
        text[i] = n % 10 + CHAR_0;
        i++;
    }
    else
    {
        text[i] = n % 10 + CHAR_0;
        i++;
        text[i] = CHAR_SPACER;
        i++;
    }

    text[i] = EOS;

    DrawOptionMenuChoice(gText_FrameType, 104, ypos, 0);
    DrawOptionMenuChoice(text, 128, ypos, 1);
}

static u8 WindowFrameType_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection < WINDOW_FRAMES_COUNT - 1)
            selection++;
        else
            selection = 0;

        LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(selection)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = WINDOW_FRAMES_COUNT - 1;

        LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(selection)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        sArrowPressed = TRUE;
    }
    return selection;
}

static void DrawHeaderText(void)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, gText_Option, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawOptionMenuTexts(void)
{
    u8 i;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = 0; i < M(MAX); i++)
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[i], 8, (i * 16) + 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

static void ReDrawMenu(u8 taskId)
{
    u8 i;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = 0; i < M(MAX); i++)
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[gTasks[taskId].tMenuOffset + i], 8, (i * 16) + 1, TEXT_SKIP_DRAW, NULL);

    #define SET_DATA_ITEMS(name, count, ___, ...) DrawChoices##count(M(name), taskId, gSaveBlock2Ptr->options##name, __VA_ARGS__); 
        OPTIONS(SET_DATA_ITEMS)
    #undef SET_DATA_ITEMS

    WindowFrameType_DrawChoices(taskId, gSaveBlock2Ptr->optionsWindowFrameType);

    HighlightOptionMenuItem(gTasks[taskId].tMenuSelection);

    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}


#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA

static void DrawBgWindowFrames(void)
{
    //                     bg, tile,              x, y, width, height, palNum
    // Draw title window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  0, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1,  3,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2,  3, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28,  3,  1,  1,  7);

    // Draw options list window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  4, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 19,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 19, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 19,  1,  1,  7);

    CopyBgTilemapBufferToVram(1);
}
