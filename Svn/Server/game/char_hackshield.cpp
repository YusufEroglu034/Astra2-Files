
#include "stdafx.h"

#include "char.h"

#include "config.h"
#include "event.h"
#include "HackShield.h"
#include "log.h"
#include "desc.h"
#include "packet.h"

EVENTINFO(hackshield_event_info)
{
	DynamicCharacterPtr CharPtr;
};

EVENTFUNC(hackshield_event)
{
	hackshield_event_info* info = dynamic_cast<hackshield_event_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "hackshield_event> <Factor> Null pointer" );
		return 0;
	}

	LPCHARACTER ch = info->CharPtr;

	if (nullptr == ch)
	{
		sys_err("HShield: character pointer is null");
		return 0;
	}

	if (nullptr == ch->GetDesc())
	{
		sys_err("HShield: character has no descriptor");
		return 0;
	}

	if (false == ch->GetHackShieldCheckMode())
	{
		if (false == CHackShieldManager::instance().SendCheckPacket(ch))
		{
			return 0;
		}
		else
		{
			ch->SetHackShieldCheckMode(true);

			return HackShield_CheckCycleTime;
		}
	}

	sys_log(0, "HShield: no response from Player(%u)", ch->GetPlayerID());

	LogManager::instance().HackShieldLog(0, ch);

	ch->m_HackShieldCheckEvent = nullptr;

	ch->GetDesc()->SetPhase(PHASE_CLOSE);

	return 0;
}

void CHARACTER::StartHackShieldCheckCycle(int32_t seconds)
{
	StopHackShieldCheckCycle();

	if (false == isHackShieldEnable)
		return;

	hackshield_event_info* info = AllocEventInfo<hackshield_event_info>();

	info->CharPtr = this;

	m_HackShieldCheckEvent = event_create(hackshield_event, info, seconds);

	sys_log(0, "HShield: StartHackShieldCheckCycle %d", seconds);
}

void CHARACTER::StopHackShieldCheckCycle()
{
	if (nullptr != m_HackShieldCheckEvent)
	{
		event_cancel(&m_HackShieldCheckEvent);
		m_HackShieldCheckEvent = nullptr;

		sys_log(0, "HShield: StopHackShieldCheckCycle");
	}
}

