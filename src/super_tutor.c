#define MOVESET_WORDS ((MOVES_COUNT + 31) / 32)

static void SetMove(u16 move, u32 *moves)
{
    moves[move / 32] |= 1U << (move % 32);
}

static void DelMove(u16 move, u32 *moves)
{
    moves[move / 32] &= ~(1U << (move % 32));
}

static u8 GetMoves(u32 *moves, u16 *safeMoves)
{
    int i, j;
    u8 count = 0;
    for(i = 0; i < MOVESET_WORDS; i++)
        for(j = 0; j < 32; j++)
            if(moves[i] & (1U << j))
                safeMoves[count++] = (i * 32 + j);
    return count;
}

static void GetAllTMHMMovesBySpecies(u16 species, u32 *moves)
{
    u16 i;
    for (i = 0; i < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; i++)
    {
        if (CanSpeciesLearnTMHM(species, i))
            SetMove(ItemIdToBattleMoveId(ITEM_TM01 + i), moves);
    }
}

static void GetAllTutorMoveBySpecies(u16 species, u32 *moves)
{
    u16 i;
    for(i = 0; i < 30; i++) // TUTOR_MOVE_COUNT
        if (sTutorLearnsets[species] & (1u << i))
            SetMove(gTutorMoves[i], moves);
}

static void GetAllEggMovesBySpecies(u16 species, u32 *moves)
{
    u16 i, eggMoveIdx = 0;

    for (i = 0; i < ARRAY_COUNT(gEggMoves) - 1; i++)
    {
        if (gEggMoves[i] == species + EGG_MOVES_SPECIES_OFFSET)
        {
            eggMoveIdx = i + 1;
            break;
        }
    }

    for (i = 0; i < 10; i++) // EGG_MOVES_ARRAY_COUNT
    {
        if (gEggMoves[eggMoveIdx + i] > EGG_MOVES_SPECIES_OFFSET)
            break;

        SetMove(gEggMoves[eggMoveIdx + i], moves);
    }

}

static void GetCappedLevelUpMovesBySpecies(u16 species, u32 *moves, u8 level)
{
    int i;
    u16 monLevel = level << 9;

    for (i = 0; i < MAX_LEVEL_UP_MOVES; i++)
    {
        u16 moveId, moveLevel;
        u16 levelUpMove = gLevelUpLearnsets[species][i];

        if(levelUpMove == LEVEL_UP_END)
            break;

        moveLevel = levelUpMove & LEVEL_UP_MOVE_LV;

        if(moveLevel > monLevel)
            break;

        moveId = levelUpMove & LEVEL_UP_MOVE_ID;

        SetMove(moveId, moves);
    }
}

static void GetAllLevelUpMovesBySpecies(u16 species, u32 *moves)
{
    GetCappedLevelUpMovesBySpecies(species, moves, MAX_LEVEL);
}

static u8 GetTopDownEvolutionLine(u16 species, u16 *stages)
{  
    int i, j;
    bool8 hit = TRUE;
    u8 count = 0;
    u16 evolution;
    
    stages[count++] = species;

    while(hit && count < 3)
    {
        for(i = SPECIES_NONE + 1, hit = FALSE; i < NUM_SPECIES && !hit; i++)
        {
            for(j = 0; j < EVOS_PER_MON; j++)
            {
                evolution = gEvolutionTable[i][j].targetSpecies;

                if(evolution == SPECIES_NONE)
                    break;
                
                if(evolution == stages[count - 1])
                {
                    stages[count++] = i;
                    hit = TRUE;
                    break;
                }
            }
        }
    }

    return count;
}
/**
I had deduced this algorithm. 

// All moves numbered from 0 to 354 in sMoves 

Func: SetMove(move, moves)
moves[move/32] |= 1U << (move%32)

Func: DelMove(move, moves)
moves[move/32] &= ~(1U << (move%32))

Func: GetMoves(moves, safeMoves)
 Loop 0 to 11 as i
  Loop 0 to 31 as j
   If(moves[i] & 1U << j)
    safeMoves[k++] = sMoves[i*32+j]  

Func: GetSuperMoveTutorMoves(Pokemon){
Obtain a backwards list of the evolution lines species, starting from the args Pokemon and downwards Evo[3]
// If a base form is passed as args, then Evo is guaranteed to host only one mon, ie itself, due to the top down nature. 
// Top down helps ignore moves learnt at higher stages. 
Create an array: u32 moves[12] = {0}. 
// 12 u32 out of which only 355 bits represent each moves. 
Loop through the evolution line, top down starting from args Pokemon: {
Loop through all legal TMs, HMs and Tutor Moves for this iter mon and set each move. 
If hatched and ( base form of the evolution line or Pokemon level is 1 ):{
Obtain all egg moves of iter/base form mon and set moves. 
// Games or Mons don’t track chains nor parents as a property
Obtain all level up moves of the iter/base form upto level 100 as a base form only and set each move.
// Assumes the parent never evolved learned all the base form moves and legally is capable every single level up move it knows to the child. 
// Level 1 to catch special base forms with exclusive babies like Pikachu, Electabuzz, etc, Defaults to assume babies if level 2 i.e Pichu and Elekid. 
} 
Else: {
 Obtain all legal level up moves of iter mon upto current level. 
// Base form can’t learn level up moves out side its own pool. 
}
} 
Get currently know moves of the mon and delete moves
Return using GetMoves.
}

My pseudo code skills are crude, so please forgive this part, lol. Typed this out on mobile.
Basically, given the information available, what is the largest legal move set? Yes, Cooper and TARS went into the Black Hole 
to save humanity but to also check all the timelines the where this mon could've legally obtained said moves.

**/
static u8 GetSuperTutorMoves(struct Pokemon *mon, u16 *moves)
{
    int i;
    u8 count;
    u16 species, move;
    u16 stages[3] = {SPECIES_NONE};
    u8 mons = GetTopDownEvolutionLine(GetMonData(mon, MON_DATA_SPECIES, 0), stages);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, 0);
    u32 movesBitSet[MOVESET_WORDS] = {0};
    bool8 isHatched = (GetMonData(mon, MON_DATA_MET_LEVEL, 0) == 0);

    for(i = 0; i < mons; i++)
    {
        species = stages[i];
        GetAllTMHMMovesBySpecies(species, movesBitSet);
        GetAllTutorMoveBySpecies(species, movesBitSet);
        if(isHatched && ((i == mons - 1) || (level == 1)))
        {
            GetAllEggMovesBySpecies(species, movesBitSet);
            GetAllLevelUpMovesBySpecies(species, movesBitSet);
        }
        else
        {
            GetCappedLevelUpMovesBySpecies(species, movesBitSet, level);
        }
    }

    for(i = 0; i < 4; i++)
    {
        move = GetMonData(mon, MON_DATA_MOVE1 + i, 0);
        if(move == MOVE_NONE)
            break;
        DelMove(move, movesBitSet);
    }

    count = GetMoves(movesBitSet, moves);

    return count;

}
