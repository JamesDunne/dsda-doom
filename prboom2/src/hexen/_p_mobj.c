//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

//----------------------------------------------------------------------------
//
// PROC P_XYMovement
//
//----------------------------------------------------------------------------

void P_XYMovement(mobj_t * mo)
{
    fixed_t ptryx, ptryy;
    player_t *player;
    fixed_t xmove, ymove;
    int special;
    angle_t angle;
    static int windTab[3] = { 2048 * 5, 2048 * 10, 2048 * 25 };

    if (!mo->momx && !mo->momy)
    {
        if (mo->flags & MF_SKULLFLY)
        {                       // A flying mobj slammed into something
            mo->flags &= ~MF_SKULLFLY;
            mo->momx = mo->momy = mo->momz = 0;
            P_SetMobjState(mo, mo->info->seestate);
        }
        return;
    }
    special = mo->subsector->sector->special;
    if (mo->flags2 & MF2_WINDTHRUST)
    {
        switch (special)
        {
            case 40:
            case 41:
            case 42:           // Wind_East
                P_ThrustMobj(mo, 0, windTab[special - 40]);
                break;
            case 43:
            case 44:
            case 45:           // Wind_North
                P_ThrustMobj(mo, ANG90, windTab[special - 43]);
                break;
            case 46:
            case 47:
            case 48:           // Wind_South
                P_ThrustMobj(mo, ANG270, windTab[special - 46]);
                break;
            case 49:
            case 50:
            case 51:           // Wind_West
                P_ThrustMobj(mo, ANG180, windTab[special - 49]);
                break;
        }
    }
    player = mo->player;
    if (mo->momx > MAXMOVE)
    {
        mo->momx = MAXMOVE;
    }
    else if (mo->momx < -MAXMOVE)
    {
        mo->momx = -MAXMOVE;
    }
    if (mo->momy > MAXMOVE)
    {
        mo->momy = MAXMOVE;
    }
    else if (mo->momy < -MAXMOVE)
    {
        mo->momy = -MAXMOVE;
    }
    xmove = mo->momx;
    ymove = mo->momy;
    do
    {
        if (xmove > MAXMOVE / 2 || ymove > MAXMOVE / 2)
        {
            ptryx = mo->x + xmove / 2;
            ptryy = mo->y + ymove / 2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }
        if (!P_TryMove(mo, ptryx, ptryy))
        {                       // Blocked move
            if (mo->flags2 & MF2_SLIDE)
            {                   // Try to slide along it
                if (BlockingMobj == NULL)
                {               // Slide against wall
                    P_SlideMove(mo);
                }
                else
                {               // Slide against mobj
                    //if(P_TryMove(mo, mo->x, mo->y+mo->momy))
                    if (P_TryMove(mo, mo->x, ptryy))
                    {
                        mo->momx = 0;
                    }
                    //else if(P_TryMove(mo, mo->x+mo->momx, mo->y))
                    else if (P_TryMove(mo, ptryx, mo->y))
                    {
                        mo->momy = 0;
                    }
                    else
                    {
                        mo->momx = mo->momy = 0;
                    }
                }
            }
            else if (mo->flags & MF_MISSILE)
            {
                if (mo->flags2 & MF2_FLOORBOUNCE)
                {
                    if (BlockingMobj)
                    {
                        if ((BlockingMobj->flags2 & MF2_REFLECTIVE) ||
                            ((!BlockingMobj->player) &&
                             (!(BlockingMobj->flags & MF_COUNTKILL))))
                        {
                            fixed_t speed;

                            angle = R_PointToAngle2(BlockingMobj->x,
                                                    BlockingMobj->y, mo->x,
                                                    mo->y) +
                                ANG1 * ((P_Random(pr_hexen) % 16) - 8);
                            speed = P_AproxDistance(mo->momx, mo->momy);
                            speed = FixedMul(speed, 0.75 * FRACUNIT);
                            mo->angle = angle;
                            angle >>= ANGLETOFINESHIFT;
                            mo->momx = FixedMul(speed, finecosine[angle]);
                            mo->momy = FixedMul(speed, finesine[angle]);
                            if (mo->info->seesound)
                            {
                                S_StartSound(mo, mo->info->seesound);
                            }
                            return;
                        }
                        else
                        {       // Struck a player/creature
                            P_ExplodeMissile(mo);
                        }
                    }
                    else
                    {           // Struck a wall
                        P_BounceWall(mo);
                        switch (mo->type)
                        {
                            case HEXEN_MT_SORCBALL1:
                            case HEXEN_MT_SORCBALL2:
                            case HEXEN_MT_SORCBALL3:
                            case HEXEN_MT_SORCFX1:
                                break;
                            default:
                                if (mo->info->seesound)
                                {
                                    S_StartSound(mo, mo->info->seesound);
                                }
                                break;
                        }
                        return;
                    }
                }
                if (BlockingMobj && (BlockingMobj->flags2 & MF2_REFLECTIVE))
                {
                    angle = R_PointToAngle2(BlockingMobj->x,
                                            BlockingMobj->y, mo->x, mo->y);

                    // Change angle for delflection/reflection
                    switch (BlockingMobj->type)
                    {
                        case HEXEN_MT_CENTAUR:
                        case HEXEN_MT_CENTAURLEADER:
                            if (abs((int) angle - (int) BlockingMobj->angle) >> 24 > 45)
                                goto explode;
                            if (mo->type == HEXEN_MT_HOLY_FX)
                                goto explode;
                            // Drop through to sorcerer full reflection
                        case HEXEN_MT_SORCBOSS:
                            // Deflection
                            if (P_Random(pr_hexen) < 128)
                                angle += ANG45;
                            else
                                angle -= ANG45;
                            break;
                        default:
                            // Reflection
                            angle += ANG1 * ((P_Random(pr_hexen) % 16) - 8);
                            break;
                    }

                    // Reflect the missile along angle
                    mo->angle = angle;
                    angle >>= ANGLETOFINESHIFT;
                    mo->momx =
                        FixedMul(mo->info->speed >> 1, finecosine[angle]);
                    mo->momy =
                        FixedMul(mo->info->speed >> 1, finesine[angle]);
//                                      mo->momz = -mo->momz;
                    if (mo->flags2 & MF2_SEEKERMISSILE)
                    {
                        mo->special1.m = mo->target;
                    }
                    mo->target = BlockingMobj;
                    return;
                }
              explode:
                // Explode a missile
                if (ceilingline && ceilingline->backsector
                    && ceilingline->backsector->ceilingpic == skyflatnum)
                {               // Hack to prevent missiles exploding against the sky
                    if (mo->type == HEXEN_MT_BLOODYSKULL)
                    {
                        mo->momx = mo->momy = 0;
                        mo->momz = -FRACUNIT;
                    }
                    else if (mo->type == HEXEN_MT_HOLY_FX)
                    {
                        P_ExplodeMissile(mo);
                    }
                    else
                    {
                        P_RemoveMobj(mo);
                    }
                    return;
                }
                P_ExplodeMissile(mo);
            }
            //else if(mo->info->crashstate)
            //{
            //      mo->momx = mo->momy = 0;
            //      P_SetMobjState(mo, mo->info->crashstate);
            //      return;
            //}
            else
            {
                mo->momx = mo->momy = 0;
            }
        }
    }
    while (xmove || ymove);

    // Friction

    if (player && player->cheats & CF_NOMOMENTUM)
    {                           // Debug option for no sliding at all
        mo->momx = mo->momy = 0;
        return;
    }
    if (mo->flags & (MF_MISSILE | MF_SKULLFLY))
    {                           // No friction for missiles
        return;
    }
    if (mo->z > mo->floorz && !(mo->flags2 & MF2_FLY)
        && !(mo->flags2 & MF2_ONMOBJ))
    {                           // No friction when falling
        if (mo->type != HEXEN_MT_BLASTEFFECT)
            return;
    }
    if (mo->flags & MF_CORPSE)
    {                           // Don't stop sliding if halfway off a step with some momentum
        if (mo->momx > FRACUNIT / 4 || mo->momx < -FRACUNIT / 4
            || mo->momy > FRACUNIT / 4 || mo->momy < -FRACUNIT / 4)
        {
            if (mo->floorz != mo->subsector->sector->floorheight)
            {
                return;
            }
        }
    }
    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED && mo->momy < STOPSPEED
        && (!player || (player->cmd.forwardmove == 0
                        && player->cmd.sidemove == 0)))
    {                           // If in a walking frame, stop moving
        if (player)
        {
            if ((unsigned) ((player->mo->state - states)
                            - PStateRun[player->pclass]) < 4)
            {
                P_SetMobjState(player->mo, PStateNormal[player->pclass]);
            }
        }
        mo->momx = 0;
        mo->momy = 0;
    }
    else
    {
        if (mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz)
            && !(mo->flags2 & MF2_ONMOBJ))
        {
            mo->momx = FixedMul(mo->momx, FRICTION_FLY);
            mo->momy = FixedMul(mo->momy, FRICTION_FLY);
        }
        else if (P_GetThingFloorType(mo) == FLOOR_ICE)
        {
            mo->momx = FixedMul(mo->momx, FRICTION_LOW);
            mo->momy = FixedMul(mo->momy, FRICTION_LOW);
        }
        else
        {
            mo->momx = FixedMul(mo->momx, ORIG_FRICTION);
            mo->momy = FixedMul(mo->momy, ORIG_FRICTION);
        }
    }
}


// Move this to p_inter ***
void P_MonsterFallingDamage(mobj_t * mo)
{
    int damage;
    int mom;

    mom = abs(mo->momz);
    if (mom > 35 * FRACUNIT)
    {                           // automatic death
        damage = 10000;
    }
    else
    {
        damage = ((mom - (23 * FRACUNIT)) * 6) >> FRACBITS;
    }
    damage = 10000;             // always kill 'em
    P_DamageMobj(mo, NULL, NULL, damage);
}



/*
===============
=
= P_ZMovement
=
===============
*/

void P_ZMovement(mobj_t * mo)
{
    int dist;
    int delta;
//
// check for smooth step up
//
    if (mo->player && mo->z < mo->floorz)
    {
        mo->player->viewheight -= mo->floorz - mo->z;
        mo->player->deltaviewheight =
            (VIEWHEIGHT - mo->player->viewheight) >> 3;
    }
//
// adjust height
//
    mo->z += mo->momz;
    if (mo->flags & MF_FLOAT && mo->target)
    {                           // float down towards target if too close
        if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
        {
            dist =
                P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y);
            delta = (mo->target->z + (mo->height >> 1)) - mo->z;
            if (delta < 0 && dist < -(delta * 3))
                mo->z -= FLOATSPEED;
            else if (delta > 0 && dist < (delta * 3))
                mo->z += FLOATSPEED;
        }
    }
    if (mo->player && mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz)
        && leveltime & 2)
    {
        mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
    }

//
// clip movement
//
    if (mo->z <= mo->floorz)
    {                           // Hit the floor
        if (mo->flags & MF_MISSILE)
        {
            mo->z = mo->floorz;
            if (mo->flags2 & MF2_FLOORBOUNCE)
            {
                P_FloorBounceMissile(mo);
                return;
            }
            else if (mo->type == HEXEN_MT_HOLY_FX)
            {                   // The spirit struck the ground
                mo->momz = 0;
                P_HitFloor(mo);
                return;
            }
            else if (mo->type == HEXEN_MT_MNTRFX2 || mo->type == HEXEN_MT_LIGHTNING_FLOOR)
            {                   // Minotaur floor fire can go up steps
                return;
            }
            else
            {
                P_HitFloor(mo);
                P_ExplodeMissile(mo);
                return;
            }
        }
        if (mo->flags & MF_COUNTKILL)   // Blasted mobj falling
        {
            if (mo->momz < -(23 * FRACUNIT))
            {
                P_MonsterFallingDamage(mo);
            }
        }
        if (mo->z - mo->momz > mo->floorz)
        {                       // Spawn splashes, etc.
            P_HitFloor(mo);
        }
        mo->z = mo->floorz;
        if (mo->momz < 0)
        {
            if (mo->flags2 & MF2_ICEDAMAGE && mo->momz < -GRAVITY * 8)
            {
                mo->tics = 1;
                mo->momx = 0;
                mo->momy = 0;
                mo->momz = 0;
                return;
            }
            if (mo->player)
            {
                mo->player->jumpTics = 7;       // delay any jumping for a short time
                if (mo->momz < -GRAVITY * 8 && !(mo->flags2 & MF2_FLY))
                {               // squat down
                    mo->player->deltaviewheight = mo->momz >> 3;
                    if (mo->momz < -23 * FRACUNIT)
                    {
                        P_FallingDamage(mo->player);
                        P_NoiseAlert(mo, mo);
                    }
                    else if (mo->momz < -GRAVITY * 12
                             && !mo->player->morphTics)
                    {
                        S_StartSound(mo, hexen_sfx_player_land);
                        switch (mo->player->pclass)
                        {
                            case PCLASS_FIGHTER:
                                S_StartSound(mo, hexen_sfx_player_fighter_grunt);
                                break;
                            case PCLASS_CLERIC:
                                S_StartSound(mo, hexen_sfx_player_cleric_grunt);
                                break;
                            case PCLASS_MAGE:
                                S_StartSound(mo, hexen_sfx_player_mage_grunt);
                                break;
                            default:
                                break;
                        }
                    }
                    else if ((P_GetThingFloorType(mo) < FLOOR_LIQUID) &&
                             (!mo->player->morphTics))
                    {
                        S_StartSound(mo, hexen_sfx_player_land);
                    }
                    // haleyjd: removed externdriver crap
                    mo->player->centering = true;
                }
            }
            else if (mo->type >= HEXEN_MT_POTTERY1 && mo->type <= HEXEN_MT_POTTERY3)
            {
                P_DamageMobj(mo, NULL, NULL, 25);
            }
            else if (mo->flags & MF_COUNTKILL)
            {
                if (mo->momz < -23 * FRACUNIT)
                {
                    // Doesn't get here
                }
            }
            mo->momz = 0;
        }
        if (mo->flags & MF_SKULLFLY)
        {                       // The skull slammed into something
            mo->momz = -mo->momz;
        }
        if (mo->info->crashstate &&
            (mo->flags & MF_CORPSE) && !(mo->flags2 & MF2_ICEDAMAGE))
        {
            P_SetMobjState(mo, mo->info->crashstate);
            return;
        }
    }
    else if (mo->flags2 & MF2_LOGRAV)
    {
        if (mo->momz == 0)
            mo->momz = -(GRAVITY >> 3) * 2;
        else
            mo->momz -= GRAVITY >> 3;
    }
    else if (!(mo->flags & MF_NOGRAVITY))
    {
        if (mo->momz == 0)
            mo->momz = -GRAVITY * 2;
        else
            mo->momz -= GRAVITY;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {                           // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;
        mo->z = mo->ceilingz - mo->height;
        if (mo->flags2 & MF2_FLOORBOUNCE)
        {
            // Maybe reverse momentum here for ceiling bounce
            // Currently won't happen

            if (mo->info->seesound)
            {
                S_StartSound(mo, mo->info->seesound);
            }
            return;
        }
        if (mo->flags & MF_SKULLFLY)
        {                       // the skull slammed into something
            mo->momz = -mo->momz;
        }
        if (mo->flags & MF_MISSILE)
        {
            if (mo->type == HEXEN_MT_LIGHTNING_CEILING)
            {
                return;
            }
            if (mo->subsector->sector->ceilingpic == skyflatnum)
            {
                if (mo->type == HEXEN_MT_BLOODYSKULL)
                {
                    mo->momx = mo->momy = 0;
                    mo->momz = -FRACUNIT;
                }
                else if (mo->type == HEXEN_MT_HOLY_FX)
                {
                    P_ExplodeMissile(mo);
                }
                else
                {
                    P_RemoveMobj(mo);
                }
                return;
            }
            P_ExplodeMissile(mo);
            return;
        }
    }
}

//----------------------------------------------------------------------------
//
// PROC P_BlasterMobjThinker
//
//
//----------------------------------------------------------------------------

void P_BlasterMobjThinker(mobj_t * mobj)
{
    int i;
    fixed_t xfrac;
    fixed_t yfrac;
    fixed_t zfrac;
    fixed_t z;
    dboolean changexy;
    mobj_t *mo;

    // Handle movement
    if (mobj->momx || mobj->momy || (mobj->z != mobj->floorz) || mobj->momz)
    {
        xfrac = mobj->momx >> 3;
        yfrac = mobj->momy >> 3;
        zfrac = mobj->momz >> 3;
        changexy = xfrac || yfrac;
        for (i = 0; i < 8; i++)
        {
            if (changexy)
            {
                if (!P_TryMove(mobj, mobj->x + xfrac, mobj->y + yfrac))
                {               // Blocked move
                    P_ExplodeMissile(mobj);
                    return;
                }
            }
            mobj->z += zfrac;
            if (mobj->z <= mobj->floorz)
            {                   // Hit the floor
                mobj->z = mobj->floorz;
                P_HitFloor(mobj);
                P_ExplodeMissile(mobj);
                return;
            }
            if (mobj->z + mobj->height > mobj->ceilingz)
            {                   // Hit the ceiling
                mobj->z = mobj->ceilingz - mobj->height;
                P_ExplodeMissile(mobj);
                return;
            }
            if (changexy)
            {
                if (mobj->type == HEXEN_MT_MWAND_MISSILE && (P_Random(pr_hexen) < 128))
                {
                    z = mobj->z - 8 * FRACUNIT;
                    if (z < mobj->floorz)
                    {
                        z = mobj->floorz;
                    }
                    P_SpawnMobj(mobj->x, mobj->y, z, HEXEN_MT_MWANDSMOKE);
                }
                else if (!--mobj->special1.i)
                {
                    mobj->special1.i = 4;
                    z = mobj->z - 12 * FRACUNIT;
                    if (z < mobj->floorz)
                    {
                        z = mobj->floorz;
                    }
                    mo = P_SpawnMobj(mobj->x, mobj->y, z, HEXEN_MT_CFLAMEFLOOR);
                    if (mo)
                    {
                        mo->angle = mobj->angle;
                    }
                }
            }
        }
    }
    // Advance the state
    if (mobj->tics != -1)
    {
        mobj->tics--;
        while (!mobj->tics)
        {
            if (!P_SetMobjState(mobj, mobj->state->nextstate))
            {                   // mobj was removed
                return;
            }
        }
    }
}

//===========================================================================
//
// PlayerLandedOnThing
//
//===========================================================================

static void PlayerLandedOnThing(mobj_t * mo, mobj_t * onmobj)
{
    mo->player->deltaviewheight = mo->momz >> 3;
    if (mo->momz < -23 * FRACUNIT)
    {
        P_FallingDamage(mo->player);
        P_NoiseAlert(mo, mo);
    }
    else if (mo->momz < -GRAVITY * 12 && !mo->player->morphTics)
    {
        S_StartSound(mo, hexen_sfx_player_land);
        switch (mo->player->pclass)
        {
            case PCLASS_FIGHTER:
                S_StartSound(mo, hexen_sfx_player_fighter_grunt);
                break;
            case PCLASS_CLERIC:
                S_StartSound(mo, hexen_sfx_player_cleric_grunt);
                break;
            case PCLASS_MAGE:
                S_StartSound(mo, hexen_sfx_player_mage_grunt);
                break;
            default:
                break;
        }
    }
    else if (!mo->player->morphTics)
    {
        S_StartSound(mo, hexen_sfx_player_land);
    }
    // haleyjd: removed externdriver crap
    mo->player->centering = true;
}

//----------------------------------------------------------------------------
//
// PROC P_MobjThinker
//
//----------------------------------------------------------------------------

void P_MobjThinker(mobj_t * mobj)
{
    mobj_t *onmo;
/*
	// Reset to not blasted when momentums are gone
	if((mobj->flags2&MF2_BLASTED) && (!(mobj->momx)) && (!(mobj->momy)))
		ResetBlasted(mobj);
*/
    // Handle X and Y momentums
    BlockingMobj = NULL;
    if (mobj->momx || mobj->momy || (mobj->flags & MF_SKULLFLY))
    {
        P_XYMovement(mobj);
        if (mobj->thinker.function == (think_t) - 1)
        {                       // mobj was removed
            return;
        }
    }
    else if (mobj->flags2 & MF2_BLASTED)
    {                           // Reset to not blasted when momentums are gone
        ResetBlasted(mobj);
    }
    if (mobj->flags2 & MF2_FLOATBOB)
    {                           // Floating item bobbing motion (special1 is height)
        mobj->z = mobj->floorz +
            mobj->special1.i + FloatBobOffsets[(mobj->health++) & 63];
    }
    else if ((mobj->z != mobj->floorz) || mobj->momz || BlockingMobj)
    {                           // Handle Z momentum and gravity
        if (mobj->flags2 & MF2_PASSMOBJ)
        {
            if (!(onmo = P_CheckOnmobj(mobj)))
            {
                P_ZMovement(mobj);
                if (mobj->player && mobj->flags & MF2_ONMOBJ)
                {
                    mobj->flags2 &= ~MF2_ONMOBJ;
                }
            }
            else
            {
                if (mobj->player)
                {
                    if (mobj->momz < -GRAVITY * 8
                        && !(mobj->flags2 & MF2_FLY))
                    {
                        PlayerLandedOnThing(mobj, onmo);
                    }
                    if (onmo->z + onmo->height - mobj->z <= 24 * FRACUNIT)
                    {
                        mobj->player->viewheight -= onmo->z + onmo->height
                            - mobj->z;
                        mobj->player->deltaviewheight =
                            (VIEWHEIGHT - mobj->player->viewheight) >> 3;
                        mobj->z = onmo->z + onmo->height;
                        mobj->flags2 |= MF2_ONMOBJ;
                        mobj->momz = 0;
                    }
                    else
                    {           // hit the bottom of the blocking mobj
                        mobj->momz = 0;
                    }
                }
/* Landing on another player, and mimicking his movements
				if(mobj->player && onmo->player)
				{
					mobj->momx = onmo->momx;
					mobj->momy = onmo->momy;
					if(onmo->z < onmo->floorz)
					{
						mobj->z += onmo->floorz-onmo->z;
						if(onmo->player)
						{
							onmo->player->viewheight -= onmo->floorz-onmo->z;
							onmo->player->deltaviewheight = (VIEWHEIGHT-
								onmo->player->viewheight)>>3;
						}
						onmo->z = onmo->floorz;
					}
				}
*/
            }
        }
        else
        {
            P_ZMovement(mobj);
        }
        if (mobj->thinker.function == (think_t) - 1)
        {                       // mobj was removed
            return;
        }
    }

    // Cycle through states, calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;
        // you can cycle through multiple states in a tic
        while (!mobj->tics)
        {
            if (!P_SetMobjState(mobj, mobj->state->nextstate))
            {                   // mobj was removed
                return;
            }
        }
    }
}

//==========================================================================
//
// P_SpawnMobj
//
//==========================================================================

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
    mobj_t *mobj;
    state_t *st;
    mobjinfo_t *info;
    fixed_t space;

    mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
    memset(mobj, 0, sizeof(*mobj));
    info = &mobjinfo[type];
    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->flags2 = info->flags2;
    mobj->damage = info->damage;
    mobj->health = info->spawnhealth;
    if (gameskill != sk_nightmare)
    {
        mobj->reactiontime = info->reactiontime;
    }
    mobj->lastlook = P_Random(pr_hexen) % maxplayers;

    // Set the state, but do not use P_SetMobjState, because action
    // routines can't be called yet.  If the spawnstate has an action
    // routine, it will not be called.
    st = &states[info->spawnstate];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // Set subsector and/or block links.
    P_SetThingPosition(mobj);
    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;
    if (z == ONFLOORZ)
    {
        mobj->z = mobj->floorz;
    }
    else if (z == ONCEILINGZ)
    {
        mobj->z = mobj->ceilingz - mobj->info->height;
    }
    else if (z == FLOATRANDZ)
    {
        space = ((mobj->ceilingz) - (mobj->info->height)) - mobj->floorz;
        if (space > 48 * FRACUNIT)
        {
            space -= 40 * FRACUNIT;
            mobj->z =
                ((space * P_Random(pr_hexen)) >> 8) + mobj->floorz + 40 * FRACUNIT;
        }
        else
        {
            mobj->z = mobj->floorz;
        }
    }
    else if (mobj->flags2 & MF2_FLOATBOB)
    {
        mobj->z = mobj->floorz + z;     // artifact z passed in as height
    }
    else
    {
        mobj->z = z;
    }
    if (mobj->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(mobj) >= FLOOR_LIQUID
        && mobj->z == mobj->subsector->sector->floorheight)
    {
        mobj->floorclip = 10 * FRACUNIT;
    }
    else
    {
        mobj->floorclip = 0;
    }

    mobj->thinker.function = P_MobjThinker;
    P_AddThinker(&mobj->thinker);
    return (mobj);
}

//==========================================================================
//
// P_RemoveMobj
//
//==========================================================================

void P_RemoveMobj(mobj_t * mobj)
{
    // Remove from creature queue
    if (mobj->flags & MF_COUNTKILL && mobj->flags & MF_CORPSE)
    {
        A_DeQueueCorpse(mobj);
    }

    if (mobj->tid)
    {                           // Remove from TID list
        P_RemoveMobjFromTIDList(mobj);
    }

    // Unlink from sector and block lists
    P_UnsetThingPosition(mobj);

    // Stop any playing sound
    S_StopSound(mobj);

    // Free block
    P_RemoveThinker((thinker_t *) mobj);
}

//==========================================================================
//
// P_SpawnPlayer
//
// Called when a player is spawned on the level.  Most of the player
// structure stays unchanged between levels.
//
//==========================================================================

void P_SpawnPlayer(mapthing_t * mthing)
{
    player_t *p;
    fixed_t x, y, z;
    mobj_t *mobj;

    if (mthing->type - 1 >= maxplayers || !playeringame[mthing->type - 1])
    {                           // Not playing
        return;
    }

    p = &players[mthing->type - 1];
    if (p->playerstate == PST_REBORN)
    {
        G_PlayerReborn(mthing->type - 1);
    }
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ONFLOORZ;
    if (randomclass && deathmatch)
    {
        p->pclass = P_Random(pr_hexen) % 3;
        if (p->pclass == PlayerClass[mthing->type - 1])
        {
            p->pclass = (p->pclass + 1) % 3;
        }
        PlayerClass[mthing->type - 1] = p->pclass;
        SB_SetClassData();
    }
    else
    {
        p->pclass = PlayerClass[mthing->type - 1];
    }
    switch (p->pclass)
    {
        case PCLASS_FIGHTER:
            mobj = P_SpawnMobj(x, y, z, HEXEN_MT_PLAYER_FIGHTER);
            break;
        case PCLASS_CLERIC:
            mobj = P_SpawnMobj(x, y, z, HEXEN_MT_PLAYER_CLERIC);
            break;
        case PCLASS_MAGE:
            mobj = P_SpawnMobj(x, y, z, HEXEN_MT_PLAYER_MAGE);
            break;
        default:
            I_Error("P_SpawnPlayer: Unknown class type");
            return;
    }

    // Set translation table data
    if (p->pclass == PCLASS_FIGHTER
        && (mthing->type == 1 || mthing->type == 3))
    {
        // The first type should be blue, and the third should be the
        // Fighter's original gold color
        if (mthing->type == 1)
        {
            mobj->flags |= 2 << MF_TRANSSHIFT;
        }
    }
    else if (mthing->type > 1)
    {                           // Set color translation bits for player sprites
        mobj->flags |= (mthing->type - 1) << MF_TRANSSHIFT;
    }

    mobj->angle = ANG45 * (mthing->angle / 45);
    mobj->player = p;
    mobj->health = p->health;
    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    P_ClearMessage(p);
    p->damagecount = 0;
    p->bonuscount = 0;
    p->poisoncount = 0;
    p->morphTics = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = VIEWHEIGHT;
    P_SetupPsprites(p);
    if (deathmatch)
    {                           // Give all keys in death match mode
        int i;
        for (i = 0; i < NUMCARDS; ++i)
          p->cards[i] = true;
    }
}

//==========================================================================
//
// P_SpawnMapThing
//
// The fields of the mapthing should already be in host byte order.
//
//==========================================================================

void P_SpawnMapThing(mapthing_t * mthing)
{
    int i;
    unsigned int spawnMask;
    mobj_t *mobj;
    fixed_t x, y, z;
    static unsigned int classFlags[] = {
        MTF_FIGHTER,
        MTF_CLERIC,
        MTF_MAGE
    };

    // Count deathmatch start positions
    if (mthing->type == 11)
    {
        if (deathmatch_p < &deathmatchstarts[MAXDEATHMATCHSTARTS])
        {
            memcpy(deathmatch_p, mthing, sizeof(*mthing));
            deathmatch_p++;
        }
        return;
    }
    if (mthing->type == PO_ANCHOR_TYPE)
    {                           // Polyobj Anchor Pt.
        return;
    }
    else if (mthing->type == PO_SPAWN_TYPE
             || mthing->type == PO_SPAWNCRUSH_TYPE)
    {                           // Polyobj Anchor Pt.
        po_NumPolyobjs++;
        return;
    }

    // Check for player starts 1 to 4
    if (mthing->type <= 4)
    {
        playerstarts[mthing->arg1][mthing->type - 1] = *mthing;
        if (!deathmatch && !mthing->arg1)
        {
            P_SpawnPlayer(mthing);
        }
        return;
    }
    // Check for player starts 5 to 8
    if (mthing->type >= 9100 && mthing->type <= 9103)
    {
        mapthing_t *player_start;
        int player;

        player = 4 + mthing->type - 9100;

        player_start = &playerstarts[mthing->arg1][player];
        memcpy(player_start, mthing, sizeof(mapthing_t));
        player_start->type = player + 1;

        if (!deathmatch && !player_start->arg1)
        {
            P_SpawnPlayer(player_start);
        }
        return;
    }

    if (mthing->type >= 1400 && mthing->type < 1410)
    {
        R_PointInSubsector(mthing->x << FRACBITS,
                           mthing->y << FRACBITS)->sector->seqType =
            mthing->type - 1400;
        return;
    }

    // Check current game type with spawn flags
    if (netgame == false)
    {
        spawnMask = MTF_GSINGLE;
    }
    else if (deathmatch)
    {
        spawnMask = MTF_GDEATHMATCH;
    }
    else
    {
        spawnMask = MTF_GCOOP;
    }
    if (!(mthing->options & spawnMask))
    {
        return;
    }

    // Check current skill with spawn flags
    if (gameskill == sk_baby || gameskill == sk_easy)
    {
        spawnMask = MTF_EASY;
    }
    else if (gameskill == sk_hard || gameskill == sk_nightmare)
    {
        spawnMask = MTF_HARD;
    }
    else
    {
        spawnMask = MTF_NORMAL;
    }
    if (!(mthing->options & spawnMask))
    {
        return;
    }

    // Check current character classes with spawn flags
    if (netgame == false)
    {                           // Single player
        if ((mthing->options & classFlags[PlayerClass[0]]) == 0)
        {                       // Not for current class
            return;
        }
    }
    else if (deathmatch == false)
    {                           // Cooperative
        spawnMask = 0;
        for (i = 0; i < maxplayers; i++)
        {
            if (playeringame[i])
            {
                spawnMask |= classFlags[PlayerClass[i]];
            }
        }
        if ((mthing->options & spawnMask) == 0)
        {
            return;
        }
    }

    // Find which type to spawn
    for (i = 0; i < NUMMOBJTYPES; i++)
    {
        if (mthing->type == mobjinfo[i].doomednum)
        {
            break;
        }
    }

    if (i == NUMMOBJTYPES)
    {                           // Can't find thing type
        I_Error("P_SpawnMapThing: Unknown type %i at (%i, %i)",
                mthing->type, mthing->x, mthing->y);
    }

    // Don't spawn keys and players in deathmatch
    if (deathmatch && mobjinfo[i].flags & MF_NOTDMATCH)
    {
        return;
    }

    // Don't spawn monsters if -nomonsters
    if (nomonsters && (mobjinfo[i].flags & MF_COUNTKILL))
    {
        return;
    }

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
    {
        z = ONCEILINGZ;
    }
    else if (mobjinfo[i].flags2 & MF2_SPAWNFLOAT)
    {
        z = FLOATRANDZ;
    }
    else if (mobjinfo[i].flags2 & MF2_FLOATBOB)
    {
        z = mthing->height << FRACBITS;
    }
    else
    {
        z = ONFLOORZ;
    }
    switch (i)
    {                           // Special stuff
        case HEXEN_MT_ZLYNCHED_NOHEART:
            P_SpawnMobj(x, y, ONFLOORZ, HEXEN_MT_BLOODPOOL);
            break;
        default:
            break;
    }
    mobj = P_SpawnMobj(x, y, z, i);
    if (z == ONFLOORZ)
    {
        mobj->z += mthing->height << FRACBITS;
    }
    else if (z == ONCEILINGZ)
    {
        mobj->z -= mthing->height << FRACBITS;
    }
    mobj->tid = mthing->tid;
    mobj->special = mthing->special;
    mobj->args[0] = mthing->arg1;
    mobj->args[1] = mthing->arg2;
    mobj->args[2] = mthing->arg3;
    mobj->args[3] = mthing->arg4;
    mobj->args[4] = mthing->arg5;
    if (mobj->flags2 & MF2_FLOATBOB)
    {                           // Seed random starting index for bobbing motion
        mobj->health = P_Random(pr_hexen);
        mobj->special1.i = mthing->height << FRACBITS;
    }
    if (mobj->tics > 0)
    {
        mobj->tics = 1 + (P_Random(pr_hexen) % mobj->tics);
    }
//      if(mobj->flags&MF_COUNTITEM)
//      {
//              totalitems++;
//      }
    if (mobj->flags & MF_COUNTKILL)
    {
        // Quantize angle to 45 degree increments
        mobj->angle = ANG45 * (mthing->angle / 45);
    }
    else
    {
        // Scale angle correctly (source is 0..359)
        mobj->angle = ((mthing->angle << 8) / 360) << 24;
    }
    if (mthing->options & MTF_AMBUSH)
    {
        mobj->flags |= MF_AMBUSH;
    }
    if (mthing->options & MTF_DORMANT)
    {
        mobj->flags2 |= MF2_DORMANT;
        if (mobj->type == HEXEN_MT_ICEGUY)
        {
            P_SetMobjState(mobj, HEXEN_S_ICEGUY_DORMANT);
        }
        mobj->tics = -1;
    }
}

//==========================================================================
//
// P_CreateTIDList
//
//==========================================================================

void P_CreateTIDList(void)
{
    int i;
    mobj_t *mobj;
    thinker_t *t;

    i = 0;
    for (t = thinkercap.next; t != &thinkercap; t = t->next)
    {                           // Search all current thinkers
        if (t->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        mobj = (mobj_t *) t;
        if (mobj->tid != 0)
        {                       // Add to list
            if (i == MAX_TID_COUNT)
            {
                I_Error("P_CreateTIDList: MAX_TID_COUNT (%d) exceeded.",
                        MAX_TID_COUNT);
            }
            TIDList[i] = mobj->tid;
            TIDMobj[i++] = mobj;
        }
    }
    // Add termination marker
    TIDList[i] = 0;
}

//==========================================================================
//
// P_InsertMobjIntoTIDList
//
//==========================================================================

void P_InsertMobjIntoTIDList(mobj_t * mobj, int tid)
{
    int i;
    int index;

    index = -1;
    for (i = 0; TIDList[i] != 0; i++)
    {
        if (TIDList[i] == -1)
        {                       // Found empty slot
            index = i;
            break;
        }
    }
    if (index == -1)
    {                           // Append required
        if (i == MAX_TID_COUNT)
        {
            I_Error("P_InsertMobjIntoTIDList: MAX_TID_COUNT (%d)"
                    "exceeded.", MAX_TID_COUNT);
        }
        index = i;
        TIDList[index + 1] = 0;
    }
    mobj->tid = tid;
    TIDList[index] = tid;
    TIDMobj[index] = mobj;
}

//==========================================================================
//
// P_RemoveMobjFromTIDList
//
//==========================================================================

void P_RemoveMobjFromTIDList(mobj_t * mobj)
{
    int i;

    for (i = 0; TIDList[i] != 0; i++)
    {
        if (TIDMobj[i] == mobj)
        {
            TIDList[i] = -1;
            TIDMobj[i] = NULL;
            mobj->tid = 0;
            return;
        }
    }
    mobj->tid = 0;
}

//==========================================================================
//
// P_FindMobjFromTID
//
//==========================================================================

mobj_t *P_FindMobjFromTID(int tid, int *searchPosition)
{
    int i;

    for (i = *searchPosition + 1; TIDList[i] != 0; i++)
    {
        if (TIDList[i] == tid)
        {
            *searchPosition = i;
            return TIDMobj[i];
        }
    }
    *searchPosition = -1;
    return NULL;
}

/*
===============================================================================

						GAME SPAWN FUNCTIONS

===============================================================================
*/

//---------------------------------------------------------------------------
//
// PROC P_SpawnPuff
//
//---------------------------------------------------------------------------

extern fixed_t attackrange;
extern mobj_t *PuffSpawned;     // true if a puff was spawned

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{
    mobj_t *puff;

    z += (P_SubRandom() << 10);
    puff = P_SpawnMobj(x, y, z, PuffType);
    if (linetarget && puff->info->seesound)
    {                           // Hit thing sound
        S_StartSound(puff, puff->info->seesound);
    }
    else if (puff->info->attacksound)
    {
        S_StartSound(puff, puff->info->attacksound);
    }
    switch (PuffType)
    {
        case HEXEN_MT_PUNCHPUFF:
            puff->momz = FRACUNIT;
            break;
        case HEXEN_MT_HAMMERPUFF:
            puff->momz = .8 * FRACUNIT;
            break;
        default:
            break;
    }
    PuffSpawned = puff;
}

/*
================
=
= P_SpawnBlood
=
================
*/

/*
void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage)
{
	mobj_t	*th;

	z += (P_SubRandom()<<10);
	th = P_SpawnMobj (x,y,z, HEXEN_MT_BLOOD);
	th->momz = FRACUNIT*2;
	th->tics -= P_Random(pr_hexen)&3;

	if (damage <= 12 && damage >= 9)
		P_SetMobjState (th,HEXEN_S_BLOOD2);
	else if (damage < 9)
		P_SetMobjState (th,HEXEN_S_BLOOD3);
}
*/

//---------------------------------------------------------------------------
//
// PROC P_BloodSplatter
//
//---------------------------------------------------------------------------

void P_BloodSplatter(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator)
{
    mobj_t *mo;

    mo = P_SpawnMobj(x, y, z, HEXEN_MT_BLOODSPLATTER);
    mo->target = originator;
    mo->momx = P_SubRandom() << 10;
    mo->momy = P_SubRandom() << 10;
    mo->momz = 3 * FRACUNIT;
}

//===========================================================================
//
//  P_BloodSplatter2
//
//===========================================================================

void P_BloodSplatter2(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_Random(pr_hexen);
    r2 = P_Random(pr_hexen);
    mo = P_SpawnMobj(x + ((r2 - 128) << 11),
                     y + ((r1 - 128) << 11), z, HEXEN_MT_AXEBLOOD);
    mo->target = originator;
}

//---------------------------------------------------------------------------
//
// PROC P_RipperBlood
//
//---------------------------------------------------------------------------

void P_RipperBlood(mobj_t * mo)
{
    mobj_t *th;
    fixed_t x, y, z;

    x = mo->x + (P_SubRandom() << 12);
    y = mo->y + (P_SubRandom() << 12);
    z = mo->z + (P_SubRandom() << 12);
    th = P_SpawnMobj(x, y, z, HEXEN_MT_BLOOD);
//      th->flags |= MF_NOGRAVITY;
    th->momx = mo->momx >> 1;
    th->momy = mo->momy >> 1;
    th->tics += P_Random(pr_hexen) & 3;
}

//---------------------------------------------------------------------------
//
// FUNC P_GetThingFloorType
//
//---------------------------------------------------------------------------

int P_GetThingFloorType(mobj_t * thing)
{
    if (thing->floorpic)
    {
        return (TerrainTypes[thing->floorpic]);
    }
    else
    {
        return (TerrainTypes[thing->subsector->sector->floorpic]);
    }
/*
	if(thing->subsector->sector->floorpic
		== W_GetNumForName("FLTWAWA1")-firstflat)
	{
		return(FLOOR_WATER);
	}
	else
	{
		return(FLOOR_SOLID);
	}
*/
}

//---------------------------------------------------------------------------
//
// FUNC P_HitFloor
//
//---------------------------------------------------------------------------
#define SMALLSPLASHCLIP 12<<FRACBITS;

int P_HitFloor(mobj_t * thing)
{
    mobj_t *mo;
    int smallsplash = false;

    if (thing->floorz != thing->subsector->sector->floorheight)
    {                           // don't splash if landing on the edge above water/lava/etc....
        return (FLOOR_SOLID);
    }

    // Things that don't splash go here
    switch (thing->type)
    {
        case HEXEN_MT_LEAF1:
        case HEXEN_MT_LEAF2:
//              case HEXEN_MT_BLOOD:                  // I set these to low mass -- pm
//              case HEXEN_MT_BLOODSPLATTER:
        case HEXEN_MT_SPLASH:
        case HEXEN_MT_SLUDGECHUNK:
            return (FLOOR_SOLID);
        default:
            break;
    }

    // Small splash for small masses
    if (thing->info->mass < 10)
        smallsplash = true;

    switch (P_GetThingFloorType(thing))
    {
        case FLOOR_WATER:
            if (smallsplash)
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_SPLASHBASE);
                if (mo)
                    mo->floorclip += SMALLSPLASHCLIP;
                S_StartSound(mo, hexen_sfx_ambient10);        // small drip
            }
            else
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_SPLASH);
                mo->target = thing;
                mo->momx = P_SubRandom() << 8;
                mo->momy = P_SubRandom() << 8;
                mo->momz = 2 * FRACUNIT + (P_Random(pr_hexen) << 8);
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_SPLASHBASE);
                if (thing->player)
                    P_NoiseAlert(thing, thing);
                S_StartSound(mo, hexen_sfx_water_splash);
            }
            return (FLOOR_WATER);
        case FLOOR_LAVA:
            if (smallsplash)
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_LAVASPLASH);
                if (mo)
                    mo->floorclip += SMALLSPLASHCLIP;
            }
            else
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_LAVASMOKE);
                mo->momz = FRACUNIT + (P_Random(pr_hexen) << 7);
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ, HEXEN_MT_LAVASPLASH);
                if (thing->player)
                    P_NoiseAlert(thing, thing);
            }
            S_StartSound(mo, hexen_sfx_lava_sizzle);
            if (thing->player && leveltime & 31)
            {
                P_DamageMobj(thing, &LavaInflictor, NULL, 5);
            }
            return (FLOOR_LAVA);
        case FLOOR_SLUDGE:
            if (smallsplash)
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
                                 HEXEN_MT_SLUDGESPLASH);
                if (mo)
                    mo->floorclip += SMALLSPLASHCLIP;
            }
            else
            {
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
                                 HEXEN_MT_SLUDGECHUNK);
                mo->target = thing;
                mo->momx = P_SubRandom() << 8;
                mo->momy = P_SubRandom() << 8;
                mo->momz = FRACUNIT + (P_Random(pr_hexen) << 8);
                mo = P_SpawnMobj(thing->x, thing->y, ONFLOORZ,
                                 HEXEN_MT_SLUDGESPLASH);
                if (thing->player)
                    P_NoiseAlert(thing, thing);
            }
            S_StartSound(mo, hexen_sfx_sludge_gloop);
            return (FLOOR_SLUDGE);
    }
    return (FLOOR_SOLID);
}


//---------------------------------------------------------------------------
//
// FUNC P_CheckMissileSpawn
//
// Returns true if the missile is at a valid spawn point, otherwise
// explodes it and returns false.
//
//---------------------------------------------------------------------------

dboolean P_CheckMissileSpawn(mobj_t * missile)
{
    //missile->tics -= P_Random(pr_hexen)&3;

    // move a little forward so an angle can be computed if it
    // immediately explodes
    missile->x += (missile->momx >> 1);
    missile->y += (missile->momy >> 1);
    missile->z += (missile->momz >> 1);
    if (!P_TryMove(missile, missile->x, missile->y))
    {
        P_ExplodeMissile(missile);
        return (false);
    }
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissile
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

mobj_t *P_SpawnMissile(mobj_t * source, mobj_t * dest, mobjtype_t type)
{
    fixed_t z;
    mobj_t *th;
    angle_t an;
    int dist;

    switch (type)
    {
        case HEXEN_MT_MNTRFX1:       // Minotaur swing attack missile
            z = source->z + 40 * FRACUNIT;
            break;
        case HEXEN_MT_MNTRFX2:       // Minotaur floor fire missile
            z = ONFLOORZ + source->floorclip;
            break;
        case HEXEN_MT_CENTAUR_FX:
            z = source->z + 45 * FRACUNIT;
            break;
        case HEXEN_MT_ICEGUY_FX:
            z = source->z + 40 * FRACUNIT;
            break;
        case HEXEN_MT_HOLY_MISSILE:
            z = source->z + 40 * FRACUNIT;
            break;
        default:
            z = source->z + 32 * FRACUNIT;
            break;
    }
    z -= source->floorclip;
    th = P_SpawnMobj(source->x, source->y, z, type);
    if (th->info->seesound)
    {
        S_StartSound(th, th->info->seesound);
    }
    th->target = source;        // Originator
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);
    if (dest->flags & MF_SHADOW)
    {                           // Invisible target
        an += P_SubRandom() << 21;
    }
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);
    dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;
    if (dist < 1)
    {
        dist = 1;
    }
    th->momz = (dest->z - source->z) / dist;
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissileXYZ
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

mobj_t *P_SpawnMissileXYZ(fixed_t x, fixed_t y, fixed_t z,
                          mobj_t * source, mobj_t * dest, mobjtype_t type)
{
    mobj_t *th;
    angle_t an;
    int dist;

    z -= source->floorclip;
    th = P_SpawnMobj(x, y, z, type);
    if (th->info->seesound)
    {
        S_StartSound(th, th->info->seesound);
    }
    th->target = source;        // Originator
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);
    if (dest->flags & MF_SHADOW)
    {                           // Invisible target
        an += P_SubRandom() << 21;
    }
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);
    dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;
    if (dist < 1)
    {
        dist = 1;
    }
    th->momz = (dest->z - source->z) / dist;
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissileAngle
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

mobj_t *P_SpawnMissileAngle(mobj_t * source, mobjtype_t type,
                            angle_t angle, fixed_t momz)
{
    fixed_t z;
    mobj_t *mo;

    switch (type)
    {
        case HEXEN_MT_MNTRFX1:       // Minotaur swing attack missile
            z = source->z + 40 * FRACUNIT;
            break;
        case HEXEN_MT_MNTRFX2:       // Minotaur floor fire missile
            z = ONFLOORZ + source->floorclip;
            break;
        case HEXEN_MT_ICEGUY_FX2:    // Secondary Projectiles of the Ice Guy
            z = source->z + 3 * FRACUNIT;
            break;
        case HEXEN_MT_MSTAFF_FX2:
            z = source->z + 40 * FRACUNIT;
            break;
        default:
            z = source->z + 32 * FRACUNIT;
            break;
    }
    z -= source->floorclip;
    mo = P_SpawnMobj(source->x, source->y, z, type);
    if (mo->info->seesound)
    {
        S_StartSound(mo, mo->info->seesound);
    }
    mo->target = source;        // Originator
    mo->angle = angle;
    angle >>= ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[angle]);
    mo->momy = FixedMul(mo->info->speed, finesine[angle]);
    mo->momz = momz;
    return (P_CheckMissileSpawn(mo) ? mo : NULL);
}

/*
================
=
= P_SpawnPlayerMissile
=
= Tries to aim at a nearby monster
================
*/

mobj_t *P_SpawnPlayerMissile(mobj_t * source, mobjtype_t type)
{
    angle_t an;
    fixed_t x, y, z, slope;

    // Try to find a target
    an = source->angle;
    slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
    if (!linetarget)
    {
        an += 1 << 26;
        slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
        if (!linetarget)
        {
            an -= 2 << 26;
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
        }
        if (!linetarget)
        {
            an = source->angle;
            slope = ((source->player->lookdir) << FRACBITS) / 173;
        }
    }
    x = source->x;
    y = source->y;
    if (type == HEXEN_MT_LIGHTNING_FLOOR)
    {
        z = ONFLOORZ;
        slope = 0;
    }
    else if (type == HEXEN_MT_LIGHTNING_CEILING)
    {
        z = ONCEILINGZ;
        slope = 0;
    }
    else
    {
        z = source->z + 4 * 8 * FRACUNIT +
            ((source->player->lookdir) << FRACBITS) / 173;
        z -= source->floorclip;
    }
    MissileMobj = P_SpawnMobj(x, y, z, type);
    if (MissileMobj->info->seesound)
    {
        //S_StartSound(MissileMobj, MissileMobj->info->seesound);
    }
    MissileMobj->target = source;
    MissileMobj->angle = an;
    MissileMobj->momx = FixedMul(MissileMobj->info->speed,
                                 finecosine[an >> ANGLETOFINESHIFT]);
    MissileMobj->momy = FixedMul(MissileMobj->info->speed,
                                 finesine[an >> ANGLETOFINESHIFT]);
    MissileMobj->momz = FixedMul(MissileMobj->info->speed, slope);
    if (MissileMobj->type == HEXEN_MT_MWAND_MISSILE
        || MissileMobj->type == HEXEN_MT_CFLAME_MISSILE)
    {                           // Ultra-fast ripper spawning missile
        MissileMobj->x += (MissileMobj->momx >> 3);
        MissileMobj->y += (MissileMobj->momy >> 3);
        MissileMobj->z += (MissileMobj->momz >> 3);
    }
    else
    {                           // Normal missile
        MissileMobj->x += (MissileMobj->momx >> 1);
        MissileMobj->y += (MissileMobj->momy >> 1);
        MissileMobj->z += (MissileMobj->momz >> 1);
    }
    if (!P_TryMove(MissileMobj, MissileMobj->x, MissileMobj->y))
    {                           // Exploded immediately
        P_ExplodeMissile(MissileMobj);
        return (NULL);
    }
    return (MissileMobj);
}


//----------------------------------------------------------------------------
//
// P_SpawnPlayerMinotaur -
//
//      Special missile that has larger blocking than player
//----------------------------------------------------------------------------

/*
mobj_t *P_SpawnPlayerMinotaur(mobj_t *source, mobjtype_t type)
{
	angle_t an;
	fixed_t x, y, z;
	fixed_t dist=0 *FRACUNIT;

	an = source->angle;
	x = source->x + FixedMul(dist, finecosine[an>>ANGLETOFINESHIFT]);
	y = source->y + FixedMul(dist, finesine[an>>ANGLETOFINESHIFT]);
	z = source->z + 4*8*FRACUNIT+((source->player->lookdir)<<FRACBITS)/173;
	z -= source->floorclip;
	MissileMobj = P_SpawnMobj(x, y, z, type);
	if(MissileMobj->info->seesound)
	{
		//S_StartSound(MissileMobj, MissileMobj->info->seesound);
	}
	MissileMobj->target = source;
	MissileMobj->angle = an;
	MissileMobj->momx = FixedMul(MissileMobj->info->speed,
		finecosine[an>>ANGLETOFINESHIFT]);
	MissileMobj->momy = FixedMul(MissileMobj->info->speed,
		finesine[an>>ANGLETOFINESHIFT]);
	MissileMobj->momz = 0;

//	MissileMobj->x += (MissileMobj->momx>>3);
//	MissileMobj->y += (MissileMobj->momy>>3);
//	MissileMobj->z += (MissileMobj->momz>>3);

	if(!P_TryMove(MissileMobj, MissileMobj->x, MissileMobj->y))
	{ // Wouln't fit

		return(NULL);
	}
	return(MissileMobj);
}
*/

//---------------------------------------------------------------------------
//
// PROC P_SPMAngle
//
//---------------------------------------------------------------------------

mobj_t *P_SPMAngle(mobj_t * source, mobjtype_t type, angle_t angle)
{
    mobj_t *th;
    angle_t an;
    fixed_t x, y, z, slope;

//
// see which target is to be aimed at
//
    an = angle;
    slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
    if (!linetarget)
    {
        an += 1 << 26;
        slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
        if (!linetarget)
        {
            an -= 2 << 26;
            slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, 0);
        }
        if (!linetarget)
        {
            an = angle;
            slope = ((source->player->lookdir) << FRACBITS) / 173;
        }
    }
    x = source->x;
    y = source->y;
    z = source->z + 4 * 8 * FRACUNIT +
        ((source->player->lookdir) << FRACBITS) / 173;
    z -= source->floorclip;
    th = P_SpawnMobj(x, y, z, type);
//      if(th->info->seesound)
//      {
//              S_StartSound(th, th->info->seesound);
//      }
    th->target = source;
    th->angle = an;
    th->momx = FixedMul(th->info->speed, finecosine[an >> ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->info->speed, finesine[an >> ANGLETOFINESHIFT]);
    th->momz = FixedMul(th->info->speed, slope);
    return (P_CheckMissileSpawn(th) ? th : NULL);
}

mobj_t *P_SpawnKoraxMissile(fixed_t x, fixed_t y, fixed_t z,
                            mobj_t * source, mobj_t * dest, mobjtype_t type)
{
    mobj_t *th;
    angle_t an;
    int dist;

    z -= source->floorclip;
    th = P_SpawnMobj(x, y, z, type);
    if (th->info->seesound)
    {
        S_StartSound(th, th->info->seesound);
    }
    th->target = source;        // Originator
    an = R_PointToAngle2(x, y, dest->x, dest->y);
    if (dest->flags & MF_SHADOW)
    {                           // Invisible target
        an += P_SubRandom() << 21;
    }
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);
    dist = P_AproxDistance(dest->x - x, dest->y - y);
    dist = dist / th->info->speed;
    if (dist < 1)
    {
        dist = 1;
    }
    th->momz = (dest->z - z + (30 * FRACUNIT)) / dist;
    return (P_CheckMissileSpawn(th) ? th : NULL);
}
