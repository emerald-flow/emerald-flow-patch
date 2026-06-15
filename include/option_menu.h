#ifndef GUARD_OPTION_MENU_H
#define GUARD_OPTION_MENU_H

void CB2_InitOptionMenu(void);

// Name, Type, Bitfield, Options_count, Callback(Func, ...FuncArgs, [Selection]), ...Options
#ifndef OPTIONS
#define OPTIONS(X) \
    X(LevelCap, 2, NULL, \
      Y(On), Y(Off)) \
    X(PSS, 2, NULL, \
      Y(On), Y(Off)) \
    X(ForgetHm, 2, NULL, \
      Y(On), Y(Off)) \
    X(InfiniteTm, 2, NULL, \
      Y(On), Y(Off)) \
    X(BetterEvos, 3, NULL, \
      Y(Off), Y(Lvl30), Y(Item)) \
    X(AdoptEggs, 2, NULL, \
      Y(On), Y(Off)) \
    X(BadgeBoost, 2, NULL, \
      Y(On), Y(Off)) \
    X(EvTraining, 3, NULL, \
      Y(Off), Y(Normal), Y(Easy)) \
    X(BetterSummary, 2, NULL, \
      Y(On), Y(Off)) \
    X(SuperTutor, 2, NULL, \
      Y(On), Y(Off)) \
    X(OneDollarItems, 2, NULL, \
      Y(On), Y(Off)) \
    X(RemoteMart, 2, NULL, \
      Y(On), Y(Off)) \
    X(PermaRepel, 2, Z(VarSet, VAR_REPEL_STEP_COUNT), \
      Y(On), Y(Off)) \
    X(CatchRate, 2, NULL, \
      Y(HundredP), Y(Normal)) \
    X(Running, 3, NULL, \
      Y(Off), Y(Indoor), Y(Perma)) \
    X(InstaFish, 2, NULL, \
      Y(On), Y(Off)) \
    X(RemoteHeal, 2, NULL, \
      Y(On), Y(Off)) \
    X(RemotePC, 2, NULL, \
      Y(On), Y(Off)) \
    X(RemoteBikeSwap, 2, NULL, \
      Y(On), Y(Off)) \
    X(AlwaysFeebas, 2, NULL, \
      Y(On), Y(Off)) \
    X(BetterSafari, 2, NULL, \
      Y(On), Y(Off)) \
    X(NoFleeingMon, 2, NULL, \
      Y(On), Y(Off)) \
    X(FastEggs, 3, NULL, \
      Y(OneX), Y(FourX), Y(TenX)) \
    X(AutoBlend, 2, NULL, \
      Y(On), Y(Off)) \
    X(CustomBlend, 2, NULL, \
      Y(On), Y(Off)) \
    X(InstaText, 2, NULL, \
      Y(On), Y(Off)) \
    X(TextSpeed, 3, NULL, \
      Y(Slow), Y(Mid), Y(Fast)) \
    X(BattleScene, 2, NULL, \
      Y(On), Y(Off)) \
    X(BattleStyle, 2, NULL, \
      Y(Shift), Y(Set)) \
    X(Sound, 2, Z(SetPokemonCryStereo), \
      Y(Mono), Y(Stereo)) \
    X(ButtonMode, 3, NULL, \
      Y(Normal), Y(LR), Y(LEqualsA))
#endif

#endif // GUARD_OPTION_MENU_H