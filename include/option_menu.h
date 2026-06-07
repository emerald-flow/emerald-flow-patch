#ifndef GUARD_OPTION_MENU_H
#define GUARD_OPTION_MENU_H

void CB2_InitOptionMenu(void);

#endif // GUARD_OPTION_MENU_H


// Name, Type, Bitfield, Options_count, Callback(Func, ...FuncArgs, [Selection]), ...Options
#ifndef OPTIONS
#define OPTIONS(X) \
    X(TextSpeed, 3, NULL, \
      Y(Slow), Y(Mid), Y(Fast)) \
    X(BattleScene, 2, NULL, \
      Y(On), Y(Off)) \
    X(BattleStyle, 2, NULL, \
      Y(Shift), Y(Set)) \
    X(Sound, 2, Z(SetPokemonCryStereo), \
      Y(Mono), Y(Stereo)) \
    X(ButtonMode, 3, NULL, \
      Y(Normal), Y(LR), Y(LEqualsA)) \
    X(LevelCap, 2, NULL, \
      Y(On), Y(Off)) \
    X(ForgetHm, 2, NULL, \
      Y(On), Y(Off)) \
    X(InfiniteTm, 2, NULL, \
      Y(On), Y(Off)) \
    X(PermaRepel, 2, Z(VarSet, VAR_REPEL_STEP_COUNT), \
      Y(On), Y(Off)) \
    X(OneCostItem, 2, NULL, \
      Y(On), Y(Off)) \
    X(EvTraining, 3, NULL, \
      Y(Off), Y(Normal), Y(Easy)) \
    X(BetterSafari, 2, NULL, \
      Y(On), Y(Off)) \
    X(NoFleeingMon, 2, NULL, \
      Y(On), Y(Off)) \
    X(AlwaysFeebas, 2, NULL, \
      Y(On), Y(Off)) \
    X(AutoBlend, 2, NULL, \
      Y(On), Y(Off)) \
    X(CustomBlend, 2, NULL, \
      Y(On), Y(Off)) \
    X(BetterEvos, 3, NULL, \
      Y(Off), Y(Lvl30), Y(Item)) \
    X(Running, 3, NULL, \
      Y(Off), Y(Indoor), Y(Perma)) \
    X(InstaFish, 2, NULL, \
      Y(Off), Y(On)) \
    X(RemoteTutor, 2, NULL, \
      Y(On), Y(Off)) \
    X(RemoteMart, 2, NULL, \
      Y(On), Y(Off)) \
    X(RemoteHeal, 2, NULL, \
      Y(On), Y(Off)) \
    X(RemoteBikeSwap, 2, NULL, \
      Y(On), Y(Off)) 
#endif