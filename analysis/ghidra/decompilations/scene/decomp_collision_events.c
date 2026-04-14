// GameObject_HandleCollision (0x40C5D0) — "CreateNoDizzy"
// BASE collision handler — dispatches ALL game events
// Called by Level_HandleCollision and Arena_HandleCollision as final step
// Parameters: this=Level, param_1=Ball, param_2=CollisionObject

void __thiscall GameObject_HandleCollision(void *this, int *ball, int *collObj) {
    char *eventName = *(char **)(collObj[1] + 0x864); // object type string
    int *app = *(int **)((int)this + 0x878);
    
    // N:SECRET — Mark rotator as triggered (secret found)
    if (__strnicmp(eventName, "N:SECRET", 8) == 0) {
        Rotator_MarkTriggered(*(int *)(*collObj + 0x47C));
    }
    
    // N:UNLOCKSECRET — Check if arena should be unlocked
    if (__strnicmp(eventName, "N:UNLOCKSECRET", 0xE) == 0) {
        CheckArenaUnlock((int)this);
    }
    
    // E:NODIZZY<TIME>N</TIME> — Anti-dizzy zone with duration
    // Parses XML-style tags: <TIME>value</TIME>
    if (__strnicmp(eventName, "E:NODIZZY", 9) == 0) {
        // Parse TIME parameter from tags
        AthenaString_Set(temp, eventName);
        tag = MWParser_ReadTag(temp);
        while (tag != NULL) {
            if (__stricmp(tag.name, "TIME") == 0) {
                duration = atol(tag.value);
                Ball_RecordBest(ball, duration); // record anti-dizzy time
            }
            tag = MWParser_ReadTag(temp);
        }
    }
    
    // E:SAFESWITCH — Copy parenthesized data to ball+0xC2C
    // Example: "E:SAFESWITCH(data)" → copies "(data)" to ball state
    if (__strnicmp(eventName, "E:SAFESWITCH", 0xC) == 0) {
        char *paren = strchr(eventName, '(');
        if (paren == NULL) {
            ball[0x30B] = 0; // clear switch
        } else {
            // copy parenthesized content to ball+0xC2C
            strcpy(ball + 0xC2C, paren);  
        }
    }
    
    // E:LIMIT — Arena boundary / completion tracking
    // Tracks arena completions per player (0-3)
    if (__stricmp(eventName, "E:LIMIT") == 0) {
        ball[0x1DA] = 0;  // clear some flag
        ball[0x2E9] = 1;  // set limit hit
        // Per-player arena completion counting at board+0x47B4/47B8/47BC/47C0
        // Only counts if this player hasn't already finished
        for (player = 0; player < 4; player++) {
            if (player != ball.playerIndex && player_is_active) {
                board->completions[player]++;
            }
        }
    }
    
    // E:BREAK — Call ball bounce callback (vtable[0x20])
    if (__stricmp(eventName, "E:BREAK") == 0) {
        ball->vtable[0x20](); // bounce/deflect
    }
    
    // E:JUMP — Bounce the ball upward with sound and score
    if (__stricmp(eventName, "E:JUMP") == 0 && ball.impactCounter < 1) {
        Sound_Play3D(app->sound_jump, ball.x, ball.y, ball.z);
        ball.impactCounter = 10;
        ball.forceX = 0.025; // upward force (0x3B03126F)
        ball.forceMode = 1;
        ball.freezeCounter = 10;
        Ball_RecordBest(ball, 200); // +200 score
    }
    
    // E:ACTION — Action event with ONCE flag and SCORE
    // Example: E:ACTION<ONCE>TRUE</ONCE><SCORE>500</SCORE>
    if (__strnicmp(eventName, "E:ACTION", 8) == 0) {
        AthenaString_Set(temp, eventName);
        tag = MWParser_ReadTag(temp);
        while (tag != NULL) {
            // ONCE=TRUE → only trigger once per ball
            if (__stricmp(tag.name, "ONCE") == 0) {
                if (AthenaList_ContainsValue(ball+0xCB, collObj))
                    break; // already triggered, skip
                AthenaList_Append(ball+0xCB, collObj); // mark as triggered
            }
            // SCORE → award points with difficulty modifier
            if (__stricmp(tag.name, "SCORE") == 0) {
                points = atol(tag.value);
                modified = Difficulty_GetTimeModifier(app, points);
                app->playerScore[ball.playerIndex] += modified;
            }
            tag = MWParser_ReadTag(temp);
        }
    }
    
    // E:TRAJECTORY(X,Y,Z) — Set ball trajectory direction
    if (__strnicmp(eventName, "E:TRAJECTORY", 0xC) == 0) {
        float x = 0, y = 0, z = 0;
        // Parse X, Y, Z from tags
        tag = MWParser_ReadTag(temp);
        while (tag != NULL) {
            if (__stricmp(tag.name, "X") == 0) x = atof(tag.value);
            if (__stricmp(tag.name, "Y") == 0) y = atof(tag.value);
            if (__stricmp(tag.name, "Z") == 0) z = atof(tag.value);
            tag = MWParser_ReadTag(temp);
        }
        ball->collisionMesh->direction = {x, y, z}; // +0xCA4/CA8/CAC
    }
    
    // N:NOCONTROL — Disable ball input for 10 frames
    if (__stricmp(eventName, "N:NOCONTROL") == 0) {
        ball[0x202] = 10; // freeze input
    }
    
    // N:WATER — Set water flag, start 10-frame counter
    if (__stricmp(eventName, "N:WATER") == 0) {
        ball[0x2D5] = 1;  // in water flag
        ball[0xB6] = 10;  // water timer
    }
    
    // N:TARPIT — Slow down ball, play tar sound
    if (__stricmp(eventName, "N:TARPIT") == 0) {
        if (ball[0xB3] == 0) { // first time in tar
            Sound_Play3D(app->sound_tar, ball.x, ball.y, ball.z);
        }
        ball[0xB3] = 1;  // in tar flag
        ball[0x1DA] = 0; // clear velocity?
    }
    
    // DROPIN — Drop-in event (pipe entrance)
    if (__stricmp(eventName+2, "DROPIN") == 0) {
        float dist = length(ball->collisionMesh->direction);
        if (dist > threshold && ball.dropinCounter < 1) {
            Sound_PlayChannel(app->sound_dropin);
            ball.dropinCounter = 50;
            Ball_RecordBest(ball, 200);
        }
    }
    
    // PIPEBONK — Pipe collision, random sound from 3
    if (__stricmp(eventName+2, "PIPEBONK") == 0 && ball.pipebonkCounter < 1) {
        int snd = RNG_Rand(3); // random 0-2
        Sound_Play3D(app->pipe_bonk_sounds[snd], ball.x, ball.y, ball.z);
        ball.pipebonkCounter = 10;
        Ball_RecordBest(ball, 100);
    }
    
    // POPOUT — Pop out of pipe
    if (__stricmp(eventName+2, "POPOUT") == 0 && ball.popoutCounter < 1) {
        Sound_PlayChannel(app->sound_popout);
        ball.popoutCounter = 50;
        Ball_RecordBest(ball, 100);
    }
    
    // N:GOAL — Finish the race!
    if (strncmp(eventName, "N:GOAL", 7) == 0 && !ball.finished && ball.active) {
        if (!this->goalReached) { // first ball to reach goal
            this->goalReached = 1;
            Audio_PlayMusic(app->music, "Goal!");
            // Store finish time for tournament scoring
        }
        // Mark this player as finished
        app->playerFinished[ball.playerIndex] = 1;
        // Copy race time to finish record
        app->finishTime[ball.playerIndex] = app->raceTime;
    }
    
    // N:MOUSETRAP — Deflect ball and track rotator collision
    if (strncmp(eventName, "N:MOUSETRAP", 12) == 0) {
        RNG_Rand(50, true); // randomize
        RNG_Rand(50, true);
        // Deflect ball: normalize direction × trap speed (0x4CF370)
        Vec3_NormalizeAndScale(&ball->direction, 1.0);
        ball->direction = ball->direction * _DAT_004CF370;
        // Search rotator list for matching object
        // Play collision sound and track interaction
    }
}

// Level_HandleCollision (0x40DCD0) — Level-specific events + delegates to base
void __thiscall Level_HandleCollision(void *this, int *ball, int *collObj) {
    char *eventName = *(char **)(collObj[1] + 0x864);
    
    // E:CATAPULTBOTTOM — Launch catapult if impact counter < 1
    if (__stricmp(eventName, "E:CATAPULTBOTTOM") == 0 && ball[0x202] < 1) {
        ball[0x202] = 1000; // set impact counter
        // Find matching catapult in list at this+0x47C4
        // Set ball as catapult target, launch, play sound
        Catapult_Launch(catapult);
        Sound_PlayChannel(app->sound_catapult);
    }
    
    // E:OPENSESAME — Open all trapdoors
    if (__stricmp(eventName, "E:OPENSESAME") == 0) {
        Trapdoor_Open(list->first); // open first trapdoor
    }
    
    // N:TRAPDOOR — Activate specific trapdoor
    if (__stricmp(eventName, "N:TRAPDOOR") == 0) {
        // Find matching trapdoor in list at this+0x4BDC
        // Activate if door ID or secondary ID matches collObj
        Trapdoor_Activate(trapdoor);
    }
    
    // E:BITE — Damage: set timer values (25.0 at 0x41C80000)
    if (__stricmp(eventName, "E:BITE") == 0) {
        this[0x43A8] = 0;
        this[0x43A0] = 25.0; // damage value
    }
    
    // E:MACETRIGGER — Activate all maces
    if (__stricmp(eventName, "E:MACETRIGGER") == 0) {
        // Iterate mace list, set each to active (flag at +0x10F0)
    }
    
    // N:MACE — Mace collision, bounce ball if mace is swinging
    if (__stricmp(eventName, "N:MACE") == 0) {
        // If mace object matches and is active (not at 0x42A00000)
        // and hasn't already hit (0x10F4 == 0): bounce ball
        ball->vtable[0x20]();
    }
    
    // Delegate to base handler for all other events
    CreateNoDizzy(this, ball, collObj); // GameObject_HandleCollision
}

// Arena_HandleCollision (0x40E6A0) — Rumble arena events + delegates to base
void __thiscall Arena_HandleCollision(void *this, int *ball, int *collObj) {
    char *eventName = *(char **)(collObj[1] + 0x864);
    int *app = *(int **)((int)this + 0x878);
    bool isMultiplayer = *(app + 0x23C) != 0;
    
    // E:CALLHAMMER — Spawn hammer chase (only in multiplayer)
    if (__stricmp(eventName, "E:CALLHAMMER") == 0 && isMultiplayer) {
        CreateBonkPopup(this->bonkObjects);
    }
    
    // E:HAMMERCHASE — Start hammer chase sequence
    if (__stricmp(eventName, "E:HAMMERCHASE") == 0 && isMultiplayer) {
        Hammer_ChaseStart(this->bonkObjects);
    }
    
    // E:ALERTSAW1/2 — Put saw blades in alert mode
    if (__stricmp(eventName, "E:ALERTSAW1") == 0 && isMultiplayer)
        Saw_AlertActivate(this->saw1);
    if (__stricmp(eventName, "E:ALERTSAW2") == 0 && isMultiplayer)
        Saw_AlertActivate(this->saw2);
    
    // E:ACTIVATESAW1/2 — Fully activate saw blades
    if (__stricmp(eventName, "E:ACTIVATESAW1") == 0 && isMultiplayer)
        Saw_Activate(this->saw1);
    if (__stricmp(eventName, "E:ACTIVATESAW2") == 0 && isMultiplayer)
        Saw_Activate(this->saw2);
    
    // E:ALERTJUDGES — Reset all judge objects
    if (__stricmp(eventName, "E:ALERTJUDGES") == 0) {
        // Iterate judge list at this+0x4FC8, reset each
    }
    
    // E:SCORE — Set score display time (number after "E:SCORE")
    if (__strnicmp(eventName, "E:SCORE", 7) == 0) {
        int time = atol(eventName + 7);
        // Iterate score display list, set time for each
    }
    
    // E:JUMP — Same as base handler (duplicated for arena context)
    if (__stricmp(eventName, "E:JUMP") == 0 && ball.impactCounter < 1) {
        Sound_Play3D(app->sound_jump, ball.x, ball.y, ball.z);
        ball.impactCounter = 10;
        ball.freezeCounter = 10;
        Ball_RecordBest(ball, 200);
    }
    
    // E:BELL — Extra time! +5 seconds and "EXTRA TIME:" popup
    if (__strnicmp(eventName, "E:BELL", 6) == 0) {
        Bell_Activate(this->bell);
        // In single player: award 500 extra time units
        if (!app->multiplayer1 && !app->multiplayer2) {
            app->playerTimer[ball.playerIndex] += 500;
            // Create ScoreObject popup "EXTRA TIME:"
            ScoreObject_ctor(score, this, timer, "EXTRA TIME:");
            Timer_Decrement(score);
            AthenaList_Append(this->popupList, score);
        }
    }
    
    // Delegate to base handler for all remaining events
    CreateNoDizzy(this, ball, collObj); // GameObject_HandleCollision
}