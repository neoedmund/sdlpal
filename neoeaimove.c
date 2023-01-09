# include "main.h"

// neoe ai for battle, no move
# define ITEMSIZE 0xffff
// = 65535
typedef struct aimove_ {
	BYTE stealcnt [ ITEMSIZE ] ; // 对同一种怪物不要偷3次以上。all the game.
} aimove ;

aimove * g_ai = 0 ;
aimove * getGai ( ) {
	if ( g_ai == 0 ) {
		g_ai = calloc ( 1 , sizeof ( aimove ) ) ;
	}
	return g_ai ;
}

int autoTarget = 0 ;
int stealCnt = 0 ; // max steal in one battle
int getMagicDamage ( WORD w , int * p2 , WORD wPlayerRole ) ;

SHORT
PAL_CalcMagicDamage (
	WORD wMagicStrength ,
	WORD wDefense ,
	const WORD rgwElementalResistance [ NUM_MAGIC_ELEMENTAL ] ,
	WORD wPoisonResistance ,
	WORD wMagicID
) ; // in fight.c
WORD
PAL_BattleUIPickAutoMagic ( // neoe
	WORD wPlayerRole ,
	WORD wRandomRange
)
/*++
  Purpose:

    Pick a magic for the specified player for automatic usage.

  Parameters:

    [IN]  wPlayerRole - the player role ID.

    [IN]  wRandomRange - the range of the magic power.

  Return value:

    The object ID of the selected magic. 0 for physical attack.
    
  neoe:
  	1. multi-enemy -> most powerful or random all cast magic
  	2. single-enemy -> group attack
  	3. enemy with item -> li steal (MagicID. 377)
  	4. heal/revive (if need?)

--*/ {
	WORD wMagic = 0 , w ;
	int iMaxPower = 0 , iPower ;
	//	printf ( "ai fight [%d] ...\n" , wPlayerRole ) ;
	if ( gpGlobals -> rgPlayerStatus [ wPlayerRole ] [ kStatusSilence ] != 0 ) return 0 ;

	//next player for coop, on use
	static int groupAttackPlayerid ;
	groupAttackPlayerid = ( groupAttackPlayerid + 1 ) % ( gpGlobals -> wMaxPartyMemberIndex + 1 ) ;

	//steal
	int hasItemEnemyId = -1 ;
	{ //getHasItemEnemyId
		getGai ( ) ;
		for ( int i = 0 ; i <= g_Battle . wMaxEnemyIndex ; i ++ ) {
			BATTLEENEMY * ene = & g_Battle . rgEnemy [ i ] ;
			if ( ene -> wObjectID != 0 &&
				g_ai -> stealcnt [ ene -> wObjectID ] < 3 &&
				ene -> e . wHealth > 0 &&
				ene -> e . nStealItem > 0 ) {
				hasItemEnemyId = i ;
				break ;
			}
		}
	}
	if ( wPlayerRole == 0 && hasItemEnemyId >= 0 // && gpGlobals -> wMaxPartyMemberIndex > 0 
		&& ++ stealCnt < 3 ) {
		autoTarget = hasItemEnemyId ;
		groupAttackPlayerid = 0 ; // avoid used by other player
		printf ( "ai fight steal\n" ) ;
		g_ai -> stealcnt [ g_Battle . rgEnemy [ hasItemEnemyId ] . wObjectID ] += 1 ;
		return 377 ; // fixme: add MP check?
	}
	//
	{ //coop, not work yet
		//	int enemyCnt = 0 ;
		//	{ //getEnemyCnt ( ) ;
		//		for ( int i = 0 ; i <= g_Battle . wMaxEnemyIndex ; i ++ ) {
		//			BATTLEENEMY * ene =  g_Battle . rgEnemy [ i ] ;
		//			if ( ene -> wObjectID != 0 &&
		//				ene -> e . wHealth > 0 ) {
		//				enemyCnt ++ ;
		//			}
		//		}
		//	}
		//	if ( FALSE /*coop not work in the workflow? disable first... */
		//		&& enemyCnt == 1 && groupAttackPlayerid == wPlayerRole
		//		&& PAL_BattleUIIsActionValid ( kBattleActionCoopMagic ) ) {
		//		autoTarget = PAL_BattleSelectAutoTarget ( ) ;
		//		printf ( "ai fight Coop %d\n" , wPlayerRole ) ;
		//		return -2 ; //for kBattleActionCoopMagic
		//	}
	}
	// find the most effecient magic
	int max2 = 0 , maxp2 = 0 ;
	for ( int i = 0 ; i < MAX_PLAYER_MAGICS ; i ++ ) {
		w = gpGlobals -> g . PlayerRoles . rgwMagic [ i ] [ wPlayerRole ] ;
		if ( w == 0 ) continue ;
		int p2 = 0 ;
		int v = getMagicDamage ( w , & p2 , wPlayerRole ) ;
		if ( v > max2 ) {
			max2 = v ;
			maxp2 = p2 ;
			wMagic = w ; //not: gpGlobals -> g . rgObject [ w ] . magic . wMagicNumber ;
		}
	}
	autoTarget = maxp2 ;
	printf ( "ai fight %d %d [%d]\n" , wPlayerRole , max2 , wMagic ) ;
	UTIL_WriteLog ( LOG_DEBUG , "ai fight %d %d [%s]\n" , wPlayerRole , max2 , PAL_GetWord ( wMagic ) ) ;
	return wMagic ;
}

int getMagicDamage ( WORD w , int * p2 , WORD wPlayerRole ) {
	WORD wMagicNum = gpGlobals -> g . rgObject [ w ] . magic . wMagicNumber ;
	// skip if the magic is an ultimate move or not enough MP
	if ( gpGlobals -> g . lprgMagic [ wMagicNum ] . wCostMP == 1 ||
		gpGlobals -> g . lprgMagic [ wMagicNum ] . wCostMP > gpGlobals -> g . PlayerRoles . rgwMP [ wPlayerRole ] ||
		( SHORT ) ( gpGlobals -> g . lprgMagic [ wMagicNum ] . wBaseDamage ) <= 0 ) {
		return 0 ;
	}

	BYTE toAll = gpGlobals -> g . rgObject [ w ] . magic . wFlags & kMagicFlagApplyToAll ;
	SHORT iPower = ( SHORT ) ( gpGlobals -> g . lprgMagic [ wMagicNum ] . wBaseDamage ) ;
	//	iPower += RandomLong ( 0 , iPower / 10 ) ;
	int sum = 0 , maxv = 0 , maxp = 0 ;
	WORD str = PAL_GetPlayerMagicStrength ( wPlayerRole ) ;
	for ( int j = 0 ; j <= g_Battle . wMaxEnemyIndex ; j ++ ) {
		BATTLEENEMY * ene = & g_Battle . rgEnemy [ j ] ;
		if ( ene -> wObjectID != 0 && ene -> e . wHealth > 0 ) {
			WORD def = ene -> e . wDefense ;
			def += ( ene -> e . wLevel + 6 ) * 4 ;

			SHORT sDamage = PAL_CalcMagicDamage ( str , def ,
				ene -> e . wElemResistance , ene -> e . wPoisonResistance , w ) ;

			if ( sDamage <= 0 ) sDamage = 1 ;

			if ( toAll ) {
				sum += sDamage ;
			} else { //to one
				if ( sDamage > maxv ) {
					maxv = sDamage ;
					maxp = j ;
				}
			}
		}
	}
	if ( toAll ) {
		return sum ;
	} else {
		* p2 = maxp ;
		return maxv ;
	}
}

