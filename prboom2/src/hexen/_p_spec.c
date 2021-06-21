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

/*
==============================================================================

							EVENTS

Events are operations triggered by using, crossing, or shooting special lines, or by timed thinkers

==============================================================================
*/
//============================================================================
//
// P_ExecuteLineSpecial
//
// Invoked when crossing a linedef. The args[] array should be at least
// 5 elements in length.
//
//============================================================================

dboolean P_ExecuteLineSpecial(int special, byte * args, line_t * line,
                             int side, mobj_t * mo)
{
    dboolean buttonSuccess;

    buttonSuccess = false;
    switch (special)
    {
        case 1:                // Poly Start Line
            break;
        case 2:                // Poly Rotate Left
            buttonSuccess = EV_RotatePoly(line, args, 1, false);
            break;
        case 3:                // Poly Rotate Right
            buttonSuccess = EV_RotatePoly(line, args, -1, false);
            break;
        case 4:                // Poly Move
            buttonSuccess = EV_MovePoly(line, args, false, false);
            break;
        case 5:                // Poly Explicit Line:  Only used in initialization
            break;
        case 6:                // Poly Move Times 8
            buttonSuccess = EV_MovePoly(line, args, true, false);
            break;
        case 7:                // Poly Door Swing
            buttonSuccess = EV_OpenPolyDoor(line, args, PODOOR_SWING);
            break;
        case 8:                // Poly Door Slide
            buttonSuccess = EV_OpenPolyDoor(line, args, PODOOR_SLIDE);
            break;
        case 10:               // Door Close
            buttonSuccess = EV_DoDoor(line, args, DREV_CLOSE);
            break;
        case 11:               // Door Open
            if (!args[0])
            {
                buttonSuccess = EV_VerticalDoor(line, mo);
            }
            else
            {
                buttonSuccess = EV_DoDoor(line, args, DREV_OPEN);
            }
            break;
        case 12:               // Door Raise
            if (!args[0])
            {
                buttonSuccess = EV_VerticalDoor(line, mo);
            }
            else
            {
                buttonSuccess = EV_DoDoor(line, args, DREV_NORMAL);
            }
            break;
        case 13:               // Door Locked_Raise
            if (CheckedLockedDoor(mo, args[3]))
            {
                if (!args[0])
                {
                    buttonSuccess = EV_VerticalDoor(line, mo);
                }
                else
                {
                    buttonSuccess = EV_DoDoor(line, args, DREV_NORMAL);
                }
            }
            break;
        case 20:               // Floor Lower by Value
            buttonSuccess = EV_DoFloor(line, args, FLEV_LOWERFLOORBYVALUE);
            break;
        case 21:               // Floor Lower to Lowest
            buttonSuccess = EV_DoFloor(line, args, FLEV_LOWERFLOORTOLOWEST);
            break;
        case 22:               // Floor Lower to Nearest
            buttonSuccess = EV_DoFloor(line, args, FLEV_LOWERFLOOR);
            break;
        case 23:               // Floor Raise by Value
            buttonSuccess = EV_DoFloor(line, args, FLEV_RAISEFLOORBYVALUE);
            break;
        case 24:               // Floor Raise to Highest
            buttonSuccess = EV_DoFloor(line, args, FLEV_RAISEFLOOR);
            break;
        case 25:               // Floor Raise to Nearest
            buttonSuccess = EV_DoFloor(line, args, FLEV_RAISEFLOORTONEAREST);
            break;
        case 26:               // Stairs Build Down Normal
            buttonSuccess = EV_BuildStairs(line, args, -1, STAIRS_NORMAL);
            break;
        case 27:               // Build Stairs Up Normal
            buttonSuccess = EV_BuildStairs(line, args, 1, STAIRS_NORMAL);
            break;
        case 28:               // Floor Raise and Crush
            buttonSuccess = EV_DoFloor(line, args, FLEV_RAISEFLOORCRUSH);
            break;
        case 29:               // Build Pillar (no crushing)
            buttonSuccess = EV_BuildPillar(line, args, false);
            break;
        case 30:               // Open Pillar
            buttonSuccess = EV_OpenPillar(line, args);
            break;
        case 31:               // Stairs Build Down Sync
            buttonSuccess = EV_BuildStairs(line, args, -1, STAIRS_SYNC);
            break;
        case 32:               // Build Stairs Up Sync
            buttonSuccess = EV_BuildStairs(line, args, 1, STAIRS_SYNC);
            break;
        case 35:               // Raise Floor by Value Times 8
            buttonSuccess = EV_DoFloor(line, args, FLEV_RAISEBYVALUETIMES8);
            break;
        case 36:               // Lower Floor by Value Times 8
            buttonSuccess = EV_DoFloor(line, args, FLEV_LOWERBYVALUETIMES8);
            break;
        case 40:               // Ceiling Lower by Value
            buttonSuccess = EV_DoCeiling(line, args, CLEV_LOWERBYVALUE);
            break;
        case 41:               // Ceiling Raise by Value
            buttonSuccess = EV_DoCeiling(line, args, CLEV_RAISEBYVALUE);
            break;
        case 42:               // Ceiling Crush and Raise
            buttonSuccess = EV_DoCeiling(line, args, CLEV_CRUSHANDRAISE);
            break;
        case 43:               // Ceiling Lower and Crush
            buttonSuccess = EV_DoCeiling(line, args, CLEV_LOWERANDCRUSH);
            break;
        case 44:               // Ceiling Crush Stop
            buttonSuccess = EV_CeilingCrushStop(line, args);
            break;
        case 45:               // Ceiling Crush Raise and Stay
            buttonSuccess = EV_DoCeiling(line, args, CLEV_CRUSHRAISEANDSTAY);
            break;
        case 46:               // Floor Crush Stop
            buttonSuccess = EV_FloorCrushStop(line, args);
            break;
        case 60:               // Plat Perpetual Raise
            buttonSuccess = EV_DoPlat(line, args, PLAT_PERPETUALRAISE, 0);
            break;
        case 61:               // Plat Stop
            EV_StopPlat(line, args);
            break;
        case 62:               // Plat Down-Wait-Up-Stay
            buttonSuccess = EV_DoPlat(line, args, PLAT_DOWNWAITUPSTAY, 0);
            break;
        case 63:               // Plat Down-by-Value*8-Wait-Up-Stay
            buttonSuccess = EV_DoPlat(line, args, PLAT_DOWNBYVALUEWAITUPSTAY,
                                      0);
            break;
        case 64:               // Plat Up-Wait-Down-Stay
            buttonSuccess = EV_DoPlat(line, args, PLAT_UPWAITDOWNSTAY, 0);
            break;
        case 65:               // Plat Up-by-Value*8-Wait-Down-Stay
            buttonSuccess = EV_DoPlat(line, args, PLAT_UPBYVALUEWAITDOWNSTAY,
                                      0);
            break;
        case 66:               // Floor Lower Instant * 8
            buttonSuccess = EV_DoFloor(line, args, FLEV_LOWERTIMES8INSTANT);
            break;
        case 67:               // Floor Raise Instant * 8
            buttonSuccess = EV_DoFloor(line, args, FLEV_RAISETIMES8INSTANT);
            break;
        case 68:               // Floor Move to Value * 8
            buttonSuccess = EV_DoFloor(line, args, FLEV_MOVETOVALUETIMES8);
            break;
        case 69:               // Ceiling Move to Value * 8
            buttonSuccess = EV_DoCeiling(line, args, CLEV_MOVETOVALUETIMES8);
            break;
        case 70:               // Teleport
            if (side == 0)
            {                   // Only teleport when crossing the front side of a line
                buttonSuccess = EV_Teleport(args[0], mo, true);
            }
            break;
        case 71:               // Teleport, no fog
            if (side == 0)
            {                   // Only teleport when crossing the front side of a line
                buttonSuccess = EV_Teleport(args[0], mo, false);
            }
            break;
        case 72:               // Thrust Mobj
            if (!side)          // Only thrust on side 0
            {
                P_ThrustMobj(mo, args[0] * (ANG90 / 64),
                             args[1] << FRACBITS);
                buttonSuccess = 1;
            }
            break;
        case 73:               // Damage Mobj
            if (args[0])
            {
                P_DamageMobj(mo, NULL, NULL, args[0]);
            }
            else
            {                   // If arg1 is zero, then guarantee a kill
                P_DamageMobj(mo, NULL, NULL, 10000);
            }
            buttonSuccess = 1;
            break;
        case 74:               // Teleport_NewMap
            if (side == 0)
            {                   // Only teleport when crossing the front side of a line
                if (!(mo && mo->player && mo->player->playerstate == PST_DEAD)) // Players must be alive to teleport
                {
                    G_Completed(args[0], args[1]);
                    buttonSuccess = true;
                }
            }
            break;
        case 75:               // Teleport_EndGame
            if (side == 0)
            {                   // Only teleport when crossing the front side of a line
                if (!(mo && mo->player && mo->player->playerstate == PST_DEAD)) // Players must be alive to teleport
                {
                    buttonSuccess = true;
                    if (deathmatch)
                    {           // Winning in deathmatch just goes back to map 1
                        G_Completed(1, 0);
                    }
                    else
                    {           // Passing -1, -1 to G_Completed() starts the Finale
                        G_Completed(-1, -1);
                    }
                }
            }
            break;
        case 80:               // ACS_Execute
            buttonSuccess =
                P_StartACS(args[0], args[1], &args[2], mo, line, side);
            break;
        case 81:               // ACS_Suspend
            buttonSuccess = P_SuspendACS(args[0], args[1]);
            break;
        case 82:               // ACS_Terminate
            buttonSuccess = P_TerminateACS(args[0], args[1]);
            break;
        case 83:               // ACS_LockedExecute
            buttonSuccess = P_StartLockedACS(line, args, mo, side);
            break;
        case 90:               // Poly Rotate Left Override
            buttonSuccess = EV_RotatePoly(line, args, 1, true);
            break;
        case 91:               // Poly Rotate Right Override
            buttonSuccess = EV_RotatePoly(line, args, -1, true);
            break;
        case 92:               // Poly Move Override
            buttonSuccess = EV_MovePoly(line, args, false, true);
            break;
        case 93:               // Poly Move Times 8 Override
            buttonSuccess = EV_MovePoly(line, args, true, true);
            break;
        case 94:               // Build Pillar Crush
            buttonSuccess = EV_BuildPillar(line, args, true);
            break;
        case 95:               // Lower Floor and Ceiling
            buttonSuccess = EV_DoFloorAndCeiling(line, args, false);
            break;
        case 96:               // Raise Floor and Ceiling
            buttonSuccess = EV_DoFloorAndCeiling(line, args, true);
            break;
        case 109:              // Force Lightning
            buttonSuccess = true;
            P_ForceLightning();
            break;
        case 110:              // Light Raise by Value
            buttonSuccess = EV_SpawnLight(line, args, LITE_RAISEBYVALUE);
            break;
        case 111:              // Light Lower by Value
            buttonSuccess = EV_SpawnLight(line, args, LITE_LOWERBYVALUE);
            break;
        case 112:              // Light Change to Value
            buttonSuccess = EV_SpawnLight(line, args, LITE_CHANGETOVALUE);
            break;
        case 113:              // Light Fade
            buttonSuccess = EV_SpawnLight(line, args, LITE_FADE);
            break;
        case 114:              // Light Glow
            buttonSuccess = EV_SpawnLight(line, args, LITE_GLOW);
            break;
        case 115:              // Light Flicker
            buttonSuccess = EV_SpawnLight(line, args, LITE_FLICKER);
            break;
        case 116:              // Light Strobe
            buttonSuccess = EV_SpawnLight(line, args, LITE_STROBE);
            break;
        case 120:              // Quake Tremor
            buttonSuccess = A_LocalQuake(args, mo);
            break;
        case 129:              // UsePuzzleItem
            buttonSuccess = EV_LineSearchForPuzzleItem(line, args, mo);
            break;
        case 130:              // Thing_Activate
            buttonSuccess = EV_ThingActivate(args[0]);
            break;
        case 131:              // Thing_Deactivate
            buttonSuccess = EV_ThingDeactivate(args[0]);
            break;
        case 132:              // Thing_Remove
            buttonSuccess = EV_ThingRemove(args[0]);
            break;
        case 133:              // Thing_Destroy
            buttonSuccess = EV_ThingDestroy(args[0]);
            break;
        case 134:              // Thing_Projectile
            buttonSuccess = EV_ThingProjectile(args, 0);
            break;
        case 135:              // Thing_Spawn
            buttonSuccess = EV_ThingSpawn(args, 1);
            break;
        case 136:              // Thing_ProjectileGravity
            buttonSuccess = EV_ThingProjectile(args, 1);
            break;
        case 137:              // Thing_SpawnNoFog
            buttonSuccess = EV_ThingSpawn(args, 0);
            break;
        case 138:              // Floor_Waggle
            buttonSuccess = EV_StartFloorWaggle(args[0], args[1],
                                                args[2], args[3], args[4]);
            break;
        case 140:              // Sector_SoundChange
            buttonSuccess = EV_SectorSoundChange(args);
            break;

            // Line specials only processed during level initialization
            // 100: Scroll_Texture_Left
            // 101: Scroll_Texture_Right
            // 102: Scroll_Texture_Up
            // 103: Scroll_Texture_Down
            // 121: Line_SetIdentification

            // Inert Line specials
        default:
            break;
    }
    return buttonSuccess;
}

//----------------------------------------------------------------------------
//
// PROC P_UpdateSpecials
//
//----------------------------------------------------------------------------

void P_UpdateSpecials(void)
{
    int i;

    // Handle buttons
    for (i = 0; i < MAXBUTTONS; i++)
    {
        if (buttonlist[i].btimer)
        {
            buttonlist[i].btimer--;
            if (!buttonlist[i].btimer)
            {
                switch (buttonlist[i].where)
                {
                    case SWTCH_TOP:
                        sides[buttonlist[i].line->sidenum[0]].toptexture =
                            buttonlist[i].btexture;
                        break;
                    case SWTCH_MIDDLE:
                        sides[buttonlist[i].line->sidenum[0]].midtexture =
                            buttonlist[i].btexture;
                        break;
                    case SWTCH_BOTTOM:
                        sides[buttonlist[i].line->sidenum[0]].bottomtexture =
                            buttonlist[i].btexture;
                        break;
                }
                //S_StartSound((mobj_t *)&buttonlist[i].soundorg, sfx_switch);
                memset(&buttonlist[i], 0, sizeof(button_t));
            }
        }
    }
}

/*
==============================================================================

							SPECIAL SPAWNING

==============================================================================
*/
/*
================================================================================
= P_SpawnSpecials
=
= After the map has been loaded, scan for specials that
= spawn thinkers
=
===============================================================================
*/

short numlinespecials;
line_t *linespeciallist[MAXLINEANIMS];

void P_SpawnSpecials(void)
{
    sector_t *sector;
    int i;

    //
    //      Init special SECTORs
    //
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
        if (!sector->special)
            continue;
        switch (sector->special)
        {
            case 1:            // Phased light
                // Hardcoded base, use sector->lightlevel as the index
                P_SpawnPhasedLight(sector, 80, -1);
                break;
            case 2:            // Phased light sequence start
                P_SpawnLightSequence(sector, 1);
                break;
                // Specials 3 & 4 are used by the phased light sequences

                /*
                   case 1:         // FLICKERING LIGHTS
                   P_SpawnLightFlash (sector);
                   break;
                   case 2:         // STROBE FAST
                   P_SpawnStrobeFlash(sector,FASTDARK,0);
                   break;
                   case 3:         // STROBE SLOW
                   P_SpawnStrobeFlash(sector,SLOWDARK,0);
                   break;
                   case 4:         // STROBE FAST/DEATH SLIME
                   P_SpawnStrobeFlash(sector,FASTDARK,0);
                   sector->special = 4;
                   break;
                   case 8:         // GLOWING LIGHT
                   P_SpawnGlowingLight(sector);
                   break;
                   case 9:         // SECRET SECTOR
                   totalsecret++;
                   break;
                   case 10:        // DOOR CLOSE IN 30 SECONDS
                   P_SpawnDoorCloseIn30 (sector);
                   break;
                   case 12:        // SYNC STROBE SLOW
                   P_SpawnStrobeFlash (sector, SLOWDARK, 1);
                   break;
                   case 13:        // SYNC STROBE FAST
                   P_SpawnStrobeFlash (sector, FASTDARK, 1);
                   break;
                   case 14:        // DOOR RAISE IN 5 MINUTES
                   P_SpawnDoorRaiseIn5Mins (sector, i);
                   break;
                 */
        }
    }


    //
    //      Init line EFFECTs
    //
    numlinespecials = 0;
    TaggedLineCount = 0;
    for (i = 0; i < numlines; i++)
    {
        switch (lines[i].special)
        {
            case 100:          // Scroll_Texture_Left
            case 101:          // Scroll_Texture_Right
            case 102:          // Scroll_Texture_Up
            case 103:          // Scroll_Texture_Down
                linespeciallist[numlinespecials] = &lines[i];
                numlinespecials++;
                break;
            case 121:          // Line_SetIdentification
                if (lines[i].arg1)
                {
                    if (TaggedLineCount == MAX_TAGGED_LINES)
                    {
                        I_Error("P_SpawnSpecials: MAX_TAGGED_LINES "
                                "(%d) exceeded.", MAX_TAGGED_LINES);
                    }
                    TaggedLines[TaggedLineCount].line = &lines[i];
                    TaggedLines[TaggedLineCount++].lineTag = lines[i].arg1;
                }
                lines[i].special = 0;
                break;
        }
    }

    //
    //      Init other misc stuff
    //
    for (i = 0; i < MAXCEILINGS; i++)
        activeceilings[i] = NULL;
    for (i = 0; i < MAXPLATS; i++)
        activeplats[i] = NULL;
    for (i = 0; i < MAXBUTTONS; i++)
        memset(&buttonlist[i], 0, sizeof(button_t));

    // Initialize flat and texture animations
    P_InitFTAnims();
}
