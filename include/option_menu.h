#ifndef GUARD_OPTION_MENU_H
#define GUARD_OPTION_MENU_H

void CB2_InitOptionMenu(void);

// Name, Type, Bitfield, Options_count, Callback(Func, ...FuncArgs, [Selection]), ...Options
#ifndef OPTIONS
#define OPTIONS(X) \
    X(PermaRepel, 2, Z(VarSet, VAR_REPEL_STEP_COUNT), \
      Y(Off), Y(On)) \
    X(Running, 3, NULL, \
      Y(Off), Y(Indoor), Y(Perma)) \
    X(LevelCap, 2, NULL, \
      Y(Off), Y(On)) \
    X(NoHMSlave, 2, NULL, \
      Y(Off), Y(On)) \
    X(BetterSummary, 2, NULL, \
      Y(Off), Y(On)) \
    X(PSS, 2, NULL, \
      Y(Off), Y(On)) \
    X(PocketTutor, 2, NULL, \
      Y(Off), Y(On)) \
    X(BetterEvos, 3, NULL, \
      Y(Off), Y(Lvl30), Y(Item)) \
    X(AdoptEggs, 2, NULL, \
      Y(Off), Y(On)) \
    X(PocketPC, 2, NULL, \
      Y(Off), Y(On)) \
    X(PocketHeal, 2, NULL, \
      Y(Off), Y(On)) \
    X(ForgetHm, 2, NULL, \
      Y(Off), Y(On)) \
    X(InfiniteTm, 2, NULL, \
      Y(Off), Y(On)) \
    X(PocketMart, 2, NULL, \
      Y(Off), Y(On)) \
    X(OneDollarItems, 2, NULL, \
      Y(Off), Y(On)) \
    X(PocketBikes, 2, NULL, \
      Y(Off), Y(On)) \
    X(AutoBlend, 2, NULL, \
      Y(Off), Y(On)) \
    X(CustomBlend, 2, NULL, \
      Y(Off), Y(On)) \
    X(BadgeBoost, 2, NULL, \
      Y(Off), Y(On)) \
    X(EvTraining, 3, NULL, \
      Y(Off), Y(On), Y(Vitamin)) \
    X(CatchRate, 2, NULL, \
     Y(Off), Y(On)) \
    X(FastEggs, 3, NULL, \
      Y(OneX), Y(FourX), Y(TenX)) \
    X(InstaFish, 2, NULL, \
      Y(Off), Y(On)) \
    X(AlwaysFeebas, 2, NULL, \
      Y(Off), Y(On)) \
    X(BetterSafari, 2, NULL, \
      Y(Off), Y(On)) \
    X(NoFleeingMon, 2, NULL, \
      Y(Off), Y(On)) \
    X(InstaText, 2, NULL, \
      Y(Off), Y(On)) \
    X(Music, 2, Z(SetMapMusic), \
      Y(Off), Y(On)) \
    X(TextSpeed, 3, NULL, \
      Y(Slow), Y(Mid), Y(Fast)) \
    X(BattleScene, 2, NULL, \
      Y(Off), Y(On)) \
    X(BattleStyle, 2, NULL, \
      Y(Set), Y(Shift)) \
    X(Sound, 2, Z(SetPokemonCryStereo), \
      Y(Stereo), Y(Mono)) \
    X(ButtonMode, 3, NULL, \
      Y(Normal), Y(LR), Y(LEqualsA))
#endif

#endif // GUARD_OPTION_MENU_H