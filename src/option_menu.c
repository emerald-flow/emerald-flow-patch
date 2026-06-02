#include "global.h"
#include "option_menu.h"
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

// #ifndef OPTIONS
// #define OPTIONS(X) \
//     X(TextSpeed, 3, \
//       Y(Slow), Y(Medium), Y(Fast)) \
//     X(BattleScene, 2, \
//       Y(On), Y(Off)) \
//     X(BattleStyle, 2, \
//       Y(Shift), Y(Set))  \
//     X(Sound, 2, \
//       Y(Mono), Y(Stereo))  \
//     X(ButtonMode, 2, \
//       Y(On), Y(Off))  
// #endif

#define MENUITEM_MAX_CLASSIC_SLIDINGOPTIONSMENU 7

enum
{
    MENUITEM_TEXTSPEED,
    MENUITEM_BATTLESCENE,
    MENUITEM_BATTLESTYLE,
    MENUITEM_SOUND,
    MENUITEM_BUTTONMODE,
    MENUITEM_LEVELCAP_CLASSIC_LEVELCAP,
    MENUITEM_FORGETHMS_CLASSIC_FORGETHMS,
    MENUITEM_INFINITETMS_CLASSIC_INFINITETMS,
    MENUITEM_PERMAREPEL_CLASSIC_PERMAREPEL,
    MENUITEM_1COSTITEMS_CLASSIC_1COSTITEMS,
    MENUITEM_EVTRAINING_CLASSIC_EVTRAINING,
    MENUITEM_FRAMETYPE,
    MENUITEM_COUNT,
};

#define tTextSpeed data[0]
#define tBattleSceneOff data[1]
#define tBattleStyle data[2]
#define tSound data[3]
#define tButtonMode data[4]
#define tWindowFrameType data[5]
#define tLevelCap_CLASSIC_LEVELCAP data[6]
#define tForgetHMs_CLASSIC_FORGETHMS data[7]
#define tInfiniteTMs_CLASSIC_INFINITETMS data[8]
#define tPermaRepel_CLASSIC_PERMAREPEL data[9]
#define t1CostItems_CLASSIC_1COSTITEMS data[10]
#define tEVTraining_CLASSIC_EVTRAINING data[11]
#define tMenuSelection data[12]
#define tMenuOffset data[13]

enum
{
    WIN_HEADER,
    WIN_OPTIONS
};

static void Task_OptionMenuFadeIn(u8 taskId);
static void Task_OptionMenuProcessInput(u8 taskId);
static void Task_OptionMenuSave(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
static void HighlightOptionMenuItem(u8 selection);
static bool8 checkInWindow_CLASSIC_SLIDINGOPTIONSMENU(u8 pos, u8 offset);
static void Draw3Choices(u8 selection, u8 offset, u8 menuitem, const u8 *a, const u8 *b, const u8 *c);
static void Draw2Choices(u8 selection, u8 offset, u8 menuitem, const u8 *a, const u8 *b);
static u8 Process3Input(u8 selection);
static u8 Process2Input(u8 selection);
static u8 FrameType_ProcessInput(u8 selection);
static void FrameType_DrawChoices(u8 selection, u8 offset);
static void DrawHeaderText(void);
static void DrawOptionMenuTexts(void);
static void DrawBgWindowFrames(void);
static void ReDrawMenu_CLASSIC_SLIDINGOPTIONSMENU(u8 taskId);

EWRAM_DATA static bool8 sArrowPressed = FALSE;

static const u16 sOptionMenuText_Pal[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCGFX_U8("graphics/interface/option_menu_equals_sign.png", ".4bpp");

static const u8 *const sOptionMenuItemsNames[MENUITEM_COUNT] =
{
    [MENUITEM_TEXTSPEED]   = gText_TextSpeed,
    [MENUITEM_BATTLESCENE] = gText_BattleScene,
    [MENUITEM_BATTLESTYLE] = gText_BattleStyle,
    [MENUITEM_SOUND]       = gText_Sound,
    [MENUITEM_BUTTONMODE]  = gText_ButtonMode,
    [MENUITEM_FRAMETYPE]   = gText_Frame,
    [MENUITEM_LEVELCAP_CLASSIC_LEVELCAP] = gText_LevelCap_CLASSIC_LEVELCAP,
    [MENUITEM_FORGETHMS_CLASSIC_FORGETHMS] = gText_ForgetHMs_CLASSIC_FORGETHMS,
    [MENUITEM_INFINITETMS_CLASSIC_INFINITETMS] = gText_InfiniteTMs_CLASSIC_INFINITETMS,
    [MENUITEM_PERMAREPEL_CLASSIC_PERMAREPEL] = gText_PermaRepel_CLASSIC_PERMAREPEL,
    [MENUITEM_1COSTITEMS_CLASSIC_1COSTITEMS] = gText_1CostItems_CLASSIC_1COSTITEMS,
    [MENUITEM_EVTRAINING_CLASSIC_EVTRAINING] = gText_EVTraining_CLASSIC_EVTRAINING
};

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
    case 9:
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 10:
    {
        u8 taskId = CreateTask(Task_OptionMenuFadeIn, 0);

        gTasks[taskId].tMenuSelection = 0;
        gTasks[taskId].tMenuOffset = 0;
        gTasks[taskId].tTextSpeed = gSaveBlock2Ptr->optionsTextSpeed;
        gTasks[taskId].tBattleSceneOff = gSaveBlock2Ptr->optionsBattleSceneOff;
        gTasks[taskId].tLevelCap_CLASSIC_LEVELCAP = gSaveBlock2Ptr->optionsLevelCap_CLASSIC_LEVELCAP;
        gTasks[taskId].tForgetHMs_CLASSIC_FORGETHMS = gSaveBlock2Ptr->optionsForgetHMs_CLASSIC_FORGETHMS;
        gTasks[taskId].tInfiniteTMs_CLASSIC_INFINITETMS = gSaveBlock2Ptr->optionsInfiniteTMs_CLASSIC_INFINITETMS;
        gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL = gSaveBlock2Ptr->optionsPermaRepel_CLASSIC_PERMAREPEL;
        gTasks[taskId].t1CostItems_CLASSIC_1COSTITEMS = gSaveBlock2Ptr->options1CostItems_CLASSIC_1COSTITEMS;
        gTasks[taskId].tEVTraining_CLASSIC_EVTRAINING = gSaveBlock2Ptr->optionsEVTraining_CLASSIC_EVTRAINING;
        gTasks[taskId].tBattleStyle = gSaveBlock2Ptr->optionsBattleStyle;
        gTasks[taskId].tSound = gSaveBlock2Ptr->optionsSound;
        gTasks[taskId].tButtonMode = gSaveBlock2Ptr->optionsButtonMode;
        gTasks[taskId].tWindowFrameType = gSaveBlock2Ptr->optionsWindowFrameType;

        ReDrawMenu_CLASSIC_SLIDINGOPTIONSMENU(taskId);

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
         gTasks[taskId].func = Task_OptionMenuSave;
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gTasks[taskId].tMenuSelection > 0)
            gTasks[taskId].tMenuSelection--;
        else if((gTasks[taskId].tMenuSelection == 0) && (gTasks[taskId].tMenuOffset == 0))
        {
            gTasks[taskId].tMenuSelection = MENUITEM_MAX_CLASSIC_SLIDINGOPTIONSMENU - 1;
            gTasks[taskId].tMenuOffset = MENUITEM_COUNT - MENUITEM_MAX_CLASSIC_SLIDINGOPTIONSMENU;
            ReDrawMenu_CLASSIC_SLIDINGOPTIONSMENU(taskId);
        }
        else 
        {
            gTasks[taskId].tMenuOffset--;
            ReDrawMenu_CLASSIC_SLIDINGOPTIONSMENU(taskId);
        }
        HighlightOptionMenuItem(gTasks[taskId].tMenuSelection);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (gTasks[taskId].tMenuSelection < MENUITEM_MAX_CLASSIC_SLIDINGOPTIONSMENU - 1)
            gTasks[taskId].tMenuSelection++;
        else if ((gTasks[taskId].tMenuSelection + gTasks[taskId].tMenuOffset) == MENUITEM_COUNT - 1)
        {
            gTasks[taskId].tMenuSelection = 0;
            gTasks[taskId].tMenuOffset = 0;
            ReDrawMenu_CLASSIC_SLIDINGOPTIONSMENU(taskId);
        }
        else
        {
            gTasks[taskId].tMenuOffset++;
            ReDrawMenu_CLASSIC_SLIDINGOPTIONSMENU(taskId);
        }
        HighlightOptionMenuItem(gTasks[taskId].tMenuSelection);
    }
    else
    {
        u8 previousOption;

        switch (gTasks[taskId].tMenuSelection + gTasks[taskId].tMenuOffset)
        {
        case MENUITEM_TEXTSPEED:
            previousOption = gTasks[taskId].tTextSpeed;
            gTasks[taskId].tTextSpeed = Process3Input(gTasks[taskId].tTextSpeed);

            if (previousOption != gTasks[taskId].tTextSpeed)
                Draw3Choices(gTasks[taskId].tTextSpeed, gTasks[taskId].tMenuOffset, MENUITEM_TEXTSPEED, gText_TextSpeedSlow, gText_TextSpeedMid, gText_TextSpeedFast);
            break;
        case MENUITEM_BATTLESCENE:
            previousOption = gTasks[taskId].tBattleSceneOff;
            gTasks[taskId].tBattleSceneOff = Process2Input(gTasks[taskId].tBattleSceneOff);

            if (previousOption != gTasks[taskId].tBattleSceneOff)
                Draw2Choices(gTasks[taskId].tBattleSceneOff, gTasks[taskId].tMenuOffset, MENUITEM_BATTLESCENE, gText_BattleSceneOn, gText_BattleSceneOff);
            break;
        case MENUITEM_BATTLESTYLE:
            previousOption = gTasks[taskId].tBattleStyle;
            gTasks[taskId].tBattleStyle = Process2Input(gTasks[taskId].tBattleStyle);

            if (previousOption != gTasks[taskId].tBattleStyle)
                Draw2Choices(gTasks[taskId].tBattleStyle, gTasks[taskId].tMenuOffset, MENUITEM_BATTLESTYLE, gText_BattleStyleShift, gText_BattleStyleSet);
            break;
        case MENUITEM_SOUND:
            previousOption = gTasks[taskId].tSound;
            gTasks[taskId].tSound = Process2Input(gTasks[taskId].tSound);
            SetPokemonCryStereo(gTasks[taskId].tSound);
            if (previousOption != gTasks[taskId].tSound)
                Draw2Choices(gTasks[taskId].tSound, gTasks[taskId].tMenuOffset, MENUITEM_SOUND, gText_SoundMono, gText_SoundStereo);
            break;
        case MENUITEM_BUTTONMODE:
            previousOption = gTasks[taskId].tButtonMode;
            gTasks[taskId].tButtonMode = Process3Input(gTasks[taskId].tButtonMode);

            if (previousOption != gTasks[taskId].tButtonMode)
                Draw3Choices(gTasks[taskId].tButtonMode, gTasks[taskId].tMenuOffset, MENUITEM_BUTTONMODE, gText_ButtonTypeNormal, gText_ButtonTypeLR, gText_ButtonTypeLEqualsA);
            break;
        case MENUITEM_FRAMETYPE:
            previousOption = gTasks[taskId].tWindowFrameType;
            gTasks[taskId].tWindowFrameType = FrameType_ProcessInput(gTasks[taskId].tWindowFrameType);

            if (previousOption != gTasks[taskId].tWindowFrameType)
                FrameType_DrawChoices(gTasks[taskId].tWindowFrameType, gTasks[taskId].tMenuOffset);
            break;
        case MENUITEM_LEVELCAP_CLASSIC_LEVELCAP:
            previousOption = gTasks[taskId].tLevelCap_CLASSIC_LEVELCAP;
            gTasks[taskId].tLevelCap_CLASSIC_LEVELCAP = Process2Input(gTasks[taskId].tLevelCap_CLASSIC_LEVELCAP);

            if (previousOption != gTasks[taskId].tLevelCap_CLASSIC_LEVELCAP)
                Draw2Choices(gTasks[taskId].tLevelCap_CLASSIC_LEVELCAP, gTasks[taskId].tMenuOffset, MENUITEM_LEVELCAP_CLASSIC_LEVELCAP, gText_LevelCapOn_CLASSIC_LEVELCAP, gText_LevelCapOff_CLASSIC_LEVELCAP);
            break;
        case MENUITEM_FORGETHMS_CLASSIC_FORGETHMS:
            previousOption = gTasks[taskId].tForgetHMs_CLASSIC_FORGETHMS;
            gTasks[taskId].tForgetHMs_CLASSIC_FORGETHMS = Process2Input(gTasks[taskId].tForgetHMs_CLASSIC_FORGETHMS);

            if (previousOption != gTasks[taskId].tForgetHMs_CLASSIC_FORGETHMS)
                Draw2Choices(gTasks[taskId].tForgetHMs_CLASSIC_FORGETHMS, gTasks[taskId].tMenuOffset, MENUITEM_FORGETHMS_CLASSIC_FORGETHMS, gText_ForgetHMsOn_CLASSIC_FORGETHMS, gText_ForgetHMsOff_CLASSIC_FORGETHMS);
            break;
        case MENUITEM_INFINITETMS_CLASSIC_INFINITETMS:
            previousOption = gTasks[taskId].tInfiniteTMs_CLASSIC_INFINITETMS;
            gTasks[taskId].tInfiniteTMs_CLASSIC_INFINITETMS = Process2Input(gTasks[taskId].tInfiniteTMs_CLASSIC_INFINITETMS);

            if (previousOption != gTasks[taskId].tInfiniteTMs_CLASSIC_INFINITETMS)
                Draw2Choices(gTasks[taskId].tInfiniteTMs_CLASSIC_INFINITETMS, gTasks[taskId].tMenuOffset, MENUITEM_INFINITETMS_CLASSIC_INFINITETMS, gText_InfiniteTMsOn_CLASSIC_INFINITETMS, gText_InfiniteTMsOff_CLASSIC_INFINITETMS);
            break;
        case MENUITEM_PERMAREPEL_CLASSIC_PERMAREPEL:
            previousOption = gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL;
            gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL = Process2Input(gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL);
            VarSet(VAR_REPEL_STEP_COUNT, gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL);
            if (previousOption != gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL)
                Draw2Choices(gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL, gTasks[taskId].tMenuOffset, MENUITEM_PERMAREPEL_CLASSIC_PERMAREPEL, gText_PermaRepelOn_CLASSIC_PERMAREPEL, gText_PermaRepelOff_CLASSIC_PERMAREPEL);
            break;
        case MENUITEM_1COSTITEMS_CLASSIC_1COSTITEMS:
            previousOption = gTasks[taskId].t1CostItems_CLASSIC_1COSTITEMS;
            gTasks[taskId].t1CostItems_CLASSIC_1COSTITEMS = Process2Input(gTasks[taskId].t1CostItems_CLASSIC_1COSTITEMS);

            if (previousOption != gTasks[taskId].t1CostItems_CLASSIC_1COSTITEMS)
                Draw2Choices(gTasks[taskId].t1CostItems_CLASSIC_1COSTITEMS, gTasks[taskId].tMenuOffset, MENUITEM_1COSTITEMS_CLASSIC_1COSTITEMS, gText_1CostItemsOn_CLASSIC_1COSTITEMS, gText_1CostItemsOff_CLASSIC_1COSTITEMS);
            break;
        case MENUITEM_EVTRAINING_CLASSIC_EVTRAINING:
            previousOption = gTasks[taskId].tEVTraining_CLASSIC_EVTRAINING;
            gTasks[taskId].tEVTraining_CLASSIC_EVTRAINING = Process3Input(gTasks[taskId].tEVTraining_CLASSIC_EVTRAINING);

            if (previousOption != gTasks[taskId].tEVTraining_CLASSIC_EVTRAINING)
                Draw3Choices(gTasks[taskId].tEVTraining_CLASSIC_EVTRAINING, gTasks[taskId].tMenuOffset, MENUITEM_EVTRAINING_CLASSIC_EVTRAINING, gText_EVTrainingOff_CLASSIC_EVTRAINING, gText_EVTrainingVanilla_CLASSIC_EVTRAINING, gText_EVTrainingEasy_CLASSIC_EVTRAINING);
            break;
        default:
            return;
        }

        if (sArrowPressed)
        {
            sArrowPressed = FALSE;
            CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
        }
    }
}

static void Task_OptionMenuSave(u8 taskId)
{
    gSaveBlock2Ptr->optionsTextSpeed = gTasks[taskId].tTextSpeed;
    gSaveBlock2Ptr->optionsBattleSceneOff = gTasks[taskId].tBattleSceneOff;
    gSaveBlock2Ptr->optionsLevelCap_CLASSIC_LEVELCAP = gTasks[taskId].tLevelCap_CLASSIC_LEVELCAP;
    gSaveBlock2Ptr->optionsForgetHMs_CLASSIC_FORGETHMS = gTasks[taskId].tForgetHMs_CLASSIC_FORGETHMS;
    gSaveBlock2Ptr->optionsInfiniteTMs_CLASSIC_INFINITETMS = gTasks[taskId].tInfiniteTMs_CLASSIC_INFINITETMS;
    gSaveBlock2Ptr->optionsPermaRepel_CLASSIC_PERMAREPEL = gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL;
    gSaveBlock2Ptr->options1CostItems_CLASSIC_1COSTITEMS = gTasks[taskId].t1CostItems_CLASSIC_1COSTITEMS;
    gSaveBlock2Ptr->optionsEVTraining_CLASSIC_EVTRAINING = gTasks[taskId].tEVTraining_CLASSIC_EVTRAINING;
    gSaveBlock2Ptr->optionsBattleStyle = gTasks[taskId].tBattleStyle;
    gSaveBlock2Ptr->optionsSound = gTasks[taskId].tSound;
    gSaveBlock2Ptr->optionsButtonMode = gTasks[taskId].tButtonMode;
    gSaveBlock2Ptr->optionsWindowFrameType = gTasks[taskId].tWindowFrameType;

    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_OptionMenuFadeOut;
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

static bool8 checkInWindow_CLASSIC_SLIDINGOPTIONSMENU(u8 pos, u8 offset)
{
    if((pos < offset)||((offset + MENUITEM_MAX_CLASSIC_SLIDINGOPTIONSMENU - 1) < pos))
        return FALSE;
    return TRUE;
}

static void Draw3Choices(u8 selection, u8 offset, u8 menuitem, const u8 *a, const u8 *b, const u8 *c)
{
    u8 styles[3];
    u8 ypos =  (menuitem - offset) * 16;
    s32 widthA, widthB, widthC, x2;

    if(!checkInWindow_CLASSIC_SLIDINGOPTIONSMENU(menuitem, offset))
        return;

    styles[0] = 0;
    styles[1] = 0;
    styles[2] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(a, 104, ypos, styles[0]);

    widthA = GetStringWidth(FONT_NORMAL, a, 0);
    widthB = GetStringWidth(FONT_NORMAL, b, 0);
    widthC = GetStringWidth(FONT_NORMAL, c, 0);

    widthB -= 94;
    x2 = (widthA - widthB - widthC) / 2 + 104;
    DrawOptionMenuChoice(b, x2, ypos, styles[1]);

    DrawOptionMenuChoice(c, GetStringRightAlignXOffset(FONT_NORMAL, c, 198), ypos, styles[2]);
}

static void Draw2Choices(u8 selection, u8 offset, u8 menuitem, const u8 *a, const u8 *b)
{       
    u8 styles[2];
    u8 ypos =  (menuitem - offset) * 16;

    if(!checkInWindow_CLASSIC_SLIDINGOPTIONSMENU(menuitem, offset))
        return;

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(a, 104, ypos, styles[1]);
    DrawOptionMenuChoice(b, GetStringRightAlignXOffset(FONT_NORMAL, b, 198), ypos, styles[0]);
}

static u8 Process3Input(u8 selection)
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

static u8 Process2Input(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void FrameType_DrawChoices(u8 selection, u8 offset)
{
    u8 text[16];
    u8 ypos =  (MENUITEM_FRAMETYPE - offset) * 16;
    u8 n = selection + 1;
    u16 i;

    if(!checkInWindow_CLASSIC_SLIDINGOPTIONSMENU(MENUITEM_FRAMETYPE, offset))
        return;

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

static u8 FrameType_ProcessInput(u8 selection)
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
    for (i = 0; i < MENUITEM_MAX_CLASSIC_SLIDINGOPTIONSMENU; i++)
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[i], 8, (i * 16) + 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

static void ReDrawMenu_CLASSIC_SLIDINGOPTIONSMENU(u8 taskId)
{
    u8 i;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = 0; i < MENUITEM_MAX_CLASSIC_SLIDINGOPTIONSMENU; i++)
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, sOptionMenuItemsNames[gTasks[taskId].tMenuOffset + i], 8, (i * 16) + 1, TEXT_SKIP_DRAW, NULL);

    Draw3Choices(gTasks[taskId].tTextSpeed, gTasks[taskId].tMenuOffset, MENUITEM_TEXTSPEED, gText_TextSpeedSlow, gText_TextSpeedMid, gText_TextSpeedFast);
    Draw2Choices(gTasks[taskId].tBattleSceneOff, gTasks[taskId].tMenuOffset, MENUITEM_BATTLESCENE, gText_BattleSceneOn, gText_BattleSceneOff);
    Draw2Choices(gTasks[taskId].tBattleStyle, gTasks[taskId].tMenuOffset, MENUITEM_BATTLESTYLE, gText_BattleStyleShift, gText_BattleStyleSet);
    Draw2Choices(gTasks[taskId].tSound, gTasks[taskId].tMenuOffset, MENUITEM_SOUND, gText_SoundMono, gText_SoundStereo);
    Draw3Choices(gTasks[taskId].tButtonMode, gTasks[taskId].tMenuOffset, MENUITEM_BUTTONMODE, gText_ButtonTypeNormal, gText_ButtonTypeLR, gText_ButtonTypeLEqualsA);
    FrameType_DrawChoices(gTasks[taskId].tWindowFrameType, gTasks[taskId].tMenuOffset);
    Draw2Choices(gTasks[taskId].tLevelCap_CLASSIC_LEVELCAP, gTasks[taskId].tMenuOffset, MENUITEM_LEVELCAP_CLASSIC_LEVELCAP, gText_LevelCapOn_CLASSIC_LEVELCAP, gText_LevelCapOff_CLASSIC_LEVELCAP);
    Draw2Choices(gTasks[taskId].tForgetHMs_CLASSIC_FORGETHMS, gTasks[taskId].tMenuOffset, MENUITEM_FORGETHMS_CLASSIC_FORGETHMS, gText_ForgetHMsOn_CLASSIC_FORGETHMS, gText_ForgetHMsOff_CLASSIC_FORGETHMS);
    Draw2Choices(gTasks[taskId].tInfiniteTMs_CLASSIC_INFINITETMS, gTasks[taskId].tMenuOffset, MENUITEM_INFINITETMS_CLASSIC_INFINITETMS, gText_InfiniteTMsOn_CLASSIC_INFINITETMS, gText_InfiniteTMsOff_CLASSIC_INFINITETMS);
    Draw2Choices(gTasks[taskId].tPermaRepel_CLASSIC_PERMAREPEL, gTasks[taskId].tMenuOffset, MENUITEM_PERMAREPEL_CLASSIC_PERMAREPEL, gText_PermaRepelOn_CLASSIC_PERMAREPEL, gText_PermaRepelOff_CLASSIC_PERMAREPEL);
    Draw2Choices(gTasks[taskId].t1CostItems_CLASSIC_1COSTITEMS, gTasks[taskId].tMenuOffset, MENUITEM_1COSTITEMS_CLASSIC_1COSTITEMS, gText_1CostItemsOn_CLASSIC_1COSTITEMS, gText_1CostItemsOff_CLASSIC_1COSTITEMS);
    Draw3Choices(gTasks[taskId].tEVTraining_CLASSIC_EVTRAINING, gTasks[taskId].tMenuOffset, MENUITEM_EVTRAINING_CLASSIC_EVTRAINING, gText_EVTrainingOff_CLASSIC_EVTRAINING, gText_EVTrainingVanilla_CLASSIC_EVTRAINING, gText_EVTrainingEasy_CLASSIC_EVTRAINING);

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
