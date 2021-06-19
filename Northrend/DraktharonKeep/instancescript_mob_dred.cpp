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

enum Spells
{
    SPELL_BELLOWING_ROAR = 22686, // fears the group, can be resisted/dispelled
    SPELL_GRIEVOUS_BITE = 48920,
    SPELL_MANGLING_SLASH = 48873, // cast on the current tank, adds debuf
    SPELL_FEARSOME_ROAR = 48849,
    SPELL_PIERCING_SLASH = 48878, // debuff --> Armor reduced by 75%
    SPELL_RAPTOR_CALL = 59416, // dummy
    SPELL_GUT_RIP = 49710,
    SPELL_REND = 13738
};

enum Info
{
    NPC_DRAKKARI_GUTRIPPER = 26641,
    NPC_DRAKKARI_SCYTHECLAW = 26628,
    NPC_KING_DRED = 27483
};

enum Misc
{
    ACTION_RAPTOR_KILLED = 1,
    DATA_RAPTORS_KILLED = 2
};

enum Events
{
    EVENT_BELLOWING_ROAR = 1,
    EVENT_GRIEVOUS_BITE,
    EVENT_MANGLING_SLASH,
    EVENT_FEARSOME_ROAR,
    EVENT_PIERCING_SLASH,
    EVENT_RAPTOR_CALL
};

class instancescript_mob_dred : public AICreatureScript
{
public:
    AI_CREATURE_SCRIPT_FUNCTION(instancescript_mob_dred, AICreatureScript);
    instancescript_mob_dred(Creature* pCreature) : AICreatureScript(pCreature)
    {
    }

    void Initialize()
    {
        raptorsKilled = 0;
    }

    void EnterCombat(Unit* mTarget)
    {
        events.ScheduleEvent(EVENT_BELLOWING_ROAR, 33000);
        events.ScheduleEvent(EVENT_GRIEVOUS_BITE, 20000);
        events.ScheduleEvent(EVENT_MANGLING_SLASH, 18500);
        events.ScheduleEvent(EVENT_FEARSOME_ROAR, urand(10000, 20000));
        events.ScheduleEvent(EVENT_PIERCING_SLASH, 17000);
        events.ScheduleEvent(EVENT_RAPTOR_CALL, urand(20000, 25000));

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
    }

    void OnCombatStop(Unit* mAttacker)
    {
        Initialize();

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);
        ParentClass::OnCombatStop(mAttacker);
    }

    void DoAction(int32 action)
    {
        if (action == ACTION_RAPTOR_KILLED)
            ++raptorsKilled;
    }

    void JustDied(Unit*  mKiller)
    {
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
            case EVENT_BELLOWING_ROAR:
                _unit->DoCastAOE(SPELL_BELLOWING_ROAR);
                events.ScheduleEvent(EVENT_BELLOWING_ROAR, 33000);
                break;
            case EVENT_GRIEVOUS_BITE:
                _unit->DoCastVictim(SPELL_GRIEVOUS_BITE);
                events.ScheduleEvent(EVENT_GRIEVOUS_BITE, 20000);
                break;
            case EVENT_MANGLING_SLASH:
                _unit->DoCastVictim(SPELL_MANGLING_SLASH);
                events.ScheduleEvent(EVENT_MANGLING_SLASH, 18500);
                break;
            case EVENT_FEARSOME_ROAR:
                _unit->DoCastAOE(SPELL_FEARSOME_ROAR);
                events.ScheduleEvent(EVENT_FEARSOME_ROAR, urand(10000, 20000));
                break;
            case EVENT_PIERCING_SLASH:
                _unit->DoCastVictim(SPELL_PIERCING_SLASH);
                events.ScheduleEvent(EVENT_PIERCING_SLASH, 17000);
                break;
            case EVENT_RAPTOR_CALL:
                _unit->DoCastVictim(SPELL_RAPTOR_CALL);
                if (RandomUInt(1) == 1)
                {
                    Unit* Gutripper = _unit->GetMapMgr()->GetInterface()->SpawnCreature(NPC_DRAKKARI_GUTRIPPER, -522.02f, -718.89f, 30.26f, 2.41f, true, false, 0, 0);
                    if (Gutripper)
                    {
                        if (GetRandomPlayerTarget())
                        {
                            Gutripper->GetAIInterface()->AttackReaction(GetRandomPlayerTarget(), 1, 0);
                        }
                    }
                }
                else
                {
                    Unit* Scytheclaw = _unit->GetMapMgr()->GetInterface()->SpawnCreature(NPC_DRAKKARI_SCYTHECLAW, -522.02f, -718.89f, 30.26f, 2.41f, true, false, 0, 0);
                    if (Scytheclaw)
                    {
                        if (GetRandomPlayerTarget())
                        {
                            Scytheclaw->GetAIInterface()->AttackReaction(GetRandomPlayerTarget(), 1, 0);
                        }
                    }
                }
                events.ScheduleEvent(EVENT_RAPTOR_CALL, urand(20000, 25000));
                break;
            default:
                break;
            }
        }
        ParentClass::UpdateAI();
    }
private:
    uint8 raptorsKilled;
};

class npc_drakkari_gutripper : public AICreatureScript
{
public:
    AI_CREATURE_SCRIPT_FUNCTION(npc_drakkari_gutripper, AICreatureScript);
    npc_drakkari_gutripper(Creature* pCreature) : AICreatureScript(pCreature)
    {
    }

    void EnterCombat(Unit* mTarget)
    {
        events.ScheduleEvent(1, urand(10000, 15000));
        ParentClass::EnterCombat(mTarget);
    }
    void UpdateAI() override
    {
        events.Update(1000);

        if (_unit->IsCasting())
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case 1:
                _unit->DoCastVictim(SPELL_GUT_RIP, false);
                break;
            }
        }
        ParentClass::UpdateAI();
    }

    void JustDied(Unit* killer) override
    {
        if (Creature* Dred = _unit->GetMapMgr()->GetCreature(instance->GetInstance()->m_BossGUID3))
            Dred->GetAIInterface()->DoAction(ACTION_RAPTOR_KILLED);
        ParentClass::JustDied(killer);
    }
private:
    InstanceScript* instance;
};

class npc_drakkari_scytheclaw : public AICreatureScript
{
public:
    AI_CREATURE_SCRIPT_FUNCTION(npc_drakkari_scytheclaw, AICreatureScript);
    npc_drakkari_scytheclaw(Creature* pCreature) : AICreatureScript(pCreature)
    {
    }

    void EnterCombat(Unit* mTarget)
    {
        events.ScheduleEvent(1, urand(10000, 15000));
        ParentClass::EnterCombat(mTarget);
    }
    void UpdateAI() override
    {
        events.Update(1000);

        if (_unit->IsCasting())
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
            case 1:
                _unit->DoCastVictim(SPELL_REND, false);
                break;
            }
        }
        ParentClass::UpdateAI();
    }

    void JustDied(Unit* killer) override
    {
        if (Creature* Dred = _unit->GetMapMgr()->GetCreature(instance->GetInstance()->m_BossGUID3))
            Dred->GetAIInterface()->DoAction(ACTION_RAPTOR_KILLED);
        ParentClass::JustDied(killer);
    }
private:
    InstanceScript* instance;
};

void SetupDrakTharonKeep(ScriptMgr* mgr)
{
    mgr->register_creature_script(NPC_KING_DRED, &instancescript_mob_dred::Create);
    mgr->register_creature_script(NPC_DRAKKARI_GUTRIPPER, &npc_drakkari_gutripper::Create);
    mgr->register_creature_script(NPC_DRAKKARI_SCYTHECLAW, &npc_drakkari_scytheclaw::Create);
}