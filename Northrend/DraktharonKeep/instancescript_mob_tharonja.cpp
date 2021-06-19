/*
* Continued-ArcEmu MMORPG Server
* Copyright (C) 2021 Continued-ArcEmu Team <https://github.com/c-arcemu/continued-arcemu>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Setup.h"

/*
 * Known Issues: Spell 49356 and 53463 will be interrupted for an unknown reason
 */

enum TharonjaSpells
{
    // Skeletal Spells (phase 1)
    SPELL_CURSE_OF_LIFE = 49527,
    SPELL_RAIN_OF_FIRE = 49518,
    SPELL_SHADOW_VOLLEY = 49528,
    SPELL_DECAY_FLESH = 49356, // cast at end of phase 1, starts phase 2
    // Flesh Spells (phase 2)
    SPELL_GIFT_OF_THARON_JA = 52509,
    SPELL_CLEAR_GIFT_OF_THARON_JA = 53242,
    SPELL_EYE_BEAM = 49544,
    SPELL_LIGHTNING_BREATH = 49537,
    SPELL_POISON_CLOUD = 49548,
    SPELL_RETURN_FLESH = 53463, // Channeled spell ending phase two and returning to phase 1. This ability will stun the party for 6 seconds.
    SPELL_ACHIEVEMENT_CHECK = 61863,
    SPELL_FLESH_VISUAL = 52582,
    SPELL_DUMMY = 49551
};

enum TharonjaEvents
{
    EVENT_CURSE_OF_LIFE = 1,
    EVENT_RAIN_OF_FIRE,
    EVENT_SHADOW_VOLLEY,

    EVENT_EYE_BEAM,
    EVENT_LIGHTNING_BREATH,
    EVENT_POISON_CLOUD,

    EVENT_DECAY_FLESH,
    EVENT_GOING_FLESH,
    EVENT_RETURN_FLESH,
    EVENT_GOING_SKELETAL
};

enum TharonjaModels
{
    MODEL_FLESH = 27073,
    NPC_PROPHET_THARONJA = 26632
};


class instancescript_mob_tharonja : public AICreatureScript
{
public:
    AI_CREATURE_SCRIPT_FUNCTION(instancescript_mob_tharonja, AICreatureScript);
    instancescript_mob_tharonja(Creature* pCreature) : AICreatureScript(pCreature)
    {}

    void EnterCombat(Unit* who)
    {
        Emote("Tharon'ja sees all! The work of mortals shall not end the eternal dynasty!", Text_Yell, 13862);
        orginalmodelid = _unit->GetDisplayId();

        events.ScheduleEvent(EVENT_DECAY_FLESH, 20000);
        events.ScheduleEvent(EVENT_CURSE_OF_LIFE, 1000);
        events.ScheduleEvent(EVENT_RAIN_OF_FIRE, urand(14000, 18000));
        events.ScheduleEvent(EVENT_SHADOW_VOLLEY, urand(8000, 10000));

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
        ParentClass::EnterCombat(who);
    }

    void OnCombatStop(Unit* mAttacker)
    {
        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);
        ParentClass::OnCombatStop(mAttacker);
    }

    void KilledUnit(Unit* who)
    {
        switch (urand(0, 1))
        {
        case 0:
            Emote("As Tharon'ja predicted!", Text_Yell, 13863);
            break;
        case 1:
            Emote("As it was written!", Text_Yell, 13864);
            break;
        }
        ParentClass::KilledUnit(who);
    }

    void JustDied(Unit* killer)
    {
        Emote("Im...impossible! Tharon'ja is eternal! Tharon'ja...is...", Text_Yell, 13869);
        _unit->DoCastAOE(SPELL_CLEAR_GIFT_OF_THARON_JA, true);
        _unit->DoCastAOE(SPELL_ACHIEVEMENT_CHECK, true);
        ParentClass::JustDied(killer);
    }

    void UpdateAI()
    {
        events.Update(1000);

        if (_unit->IsCasting())
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case EVENT_CURSE_OF_LIFE:
                if (Unit* target = GetRandomPlayerTarget())
                    _unit->DoCast(target, SPELL_CURSE_OF_LIFE);
                events.ScheduleEvent(EVENT_CURSE_OF_LIFE, urand(10000, 15000));
                return;
            case EVENT_SHADOW_VOLLEY:
                _unit->DoCastVictim(SPELL_SHADOW_VOLLEY);
                events.ScheduleEvent(EVENT_SHADOW_VOLLEY, urand(8000, 10000));
                return;
            case EVENT_RAIN_OF_FIRE:
                if (Unit* target = GetRandomPlayerTarget())
                    _unit->DoCast(target, SPELL_RAIN_OF_FIRE);
                events.ScheduleEvent(EVENT_RAIN_OF_FIRE, urand(14000, 18000));
                return;
            case EVENT_LIGHTNING_BREATH:
                if (Unit* target = GetRandomPlayerTarget())
                    _unit->DoCast(target, SPELL_LIGHTNING_BREATH);
                events.ScheduleEvent(EVENT_LIGHTNING_BREATH, urand(6000, 7000));
                return;
            case EVENT_EYE_BEAM:
                if (Unit* target = GetRandomPlayerTarget())
                    _unit->DoCast(target, SPELL_EYE_BEAM);
                events.ScheduleEvent(EVENT_EYE_BEAM, urand(4000, 6000));
                return;
            case EVENT_POISON_CLOUD:
                _unit->DoCastAOE(SPELL_POISON_CLOUD);
                events.ScheduleEvent(EVENT_POISON_CLOUD, urand(10000, 12000));
                return;
            case EVENT_DECAY_FLESH:
                _unit->DoCastAOE(SPELL_DECAY_FLESH);
                events.ScheduleEvent(EVENT_GOING_FLESH, 6000);
                return;
            case EVENT_GOING_FLESH:
                Emote("No! A taste...all too brief!", Text_Yell, 13867);
                _unit->SetDisplayId(MODEL_FLESH);
                _unit->DoCastAOE(SPELL_GIFT_OF_THARON_JA, true);
                _unit->DoCast(_unit, SPELL_FLESH_VISUAL, true);
                _unit->DoCast(_unit, SPELL_DUMMY, true);

                events.Reset();
                events.ScheduleEvent(EVENT_RETURN_FLESH, 20000);
                events.ScheduleEvent(EVENT_LIGHTNING_BREATH, urand(3000, 4000));
                events.ScheduleEvent(EVENT_EYE_BEAM, urand(4000, 8000));
                events.ScheduleEvent(EVENT_POISON_CLOUD, urand(6000, 7000));
                break;
            case EVENT_RETURN_FLESH:
                _unit->DoCastAOE(SPELL_RETURN_FLESH);
                events.ScheduleEvent(EVENT_GOING_SKELETAL, 6000);
                return;
            case EVENT_GOING_SKELETAL:
                Emote("Your flesh serves Tharon'ja now!", Text_Yell, 13865);
                _unit->SetDisplayId(orginalmodelid);
                _unit->DoCastAOE(SPELL_CLEAR_GIFT_OF_THARON_JA, true);

                events.Reset();
                events.ScheduleEvent(EVENT_DECAY_FLESH, 20000);
                events.ScheduleEvent(EVENT_CURSE_OF_LIFE, 1000);
                events.ScheduleEvent(EVENT_RAIN_OF_FIRE, urand(14000, 18000));
                events.ScheduleEvent(EVENT_SHADOW_VOLLEY, urand(8000, 10000));
                break;
            default:
                break;
            }
        }
        ParentClass::UpdateAI();
    }
protected:
    uint32 orginalmodelid = 0;
};

void SetupDrakTharonKeep(ScriptMgr* mgr)
{
    mgr->register_creature_script(NPC_PROPHET_THARONJA, &instancescript_mob_tharonja::Create);
}