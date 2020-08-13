#pragma once

#define BIT(x) (1 << (x))

namespace spark
{
    enum  : unsigned int {
        DEAD            = BIT(1),

        ATTACK_IMMUNITY = BIT(2),
        DISARM          = BIT(3),
        ETHEREAL        = BIT(4),
        HEX             = BIT(5),
        INVULNERABILITY = BIT(6),
        SILENCE         = BIT(7),
        MUTED           = BIT(8),
		ROOTED          = BIT(9),
        GROUNDED        = BIT(10),
        STUN            = BIT(11),
        BROKEN          = BIT(12),
        FEAR            = BIT(13),
        FLYING_MOVEMENT = BIT(14),
        FORCED_MOVEMENT = BIT(15),
        STEALTH         = BIT(16), // invisible
        PHASED          = BIT(17), // can walk through other units
        STATUS_IMMUNITY = BIT(18),
        TAUNT           = BIT(19), 
		BLIND			= BIT(20), 
		LEASH			= BIT(21), 
		HIDDEN          = BIT(22), // in another dimension, untargettable
		BLUR			= BIT(23), // not visible on the minimap
		SPELL_IMMUNITY  = BIT(24),

        CANT_MOVE              = DEAD | ROOTED | STUN | FORCED_MOVEMENT,
		CANT_ROTATE            = DEAD | STUN,
        CANT_ATTACK            = DEAD | HEX | DISARM,
        CANT_CAST_ABILITIES    = DEAD | HEX | SILENCE | STUN,
        CANT_CAST_ITEMS        = DEAD | HEX | MUTED | STUN,
        CANT_BE_ATTACKED       = DEAD | INVULNERABILITY | STEALTH | ATTACK_IMMUNITY,
        CANT_BE_SPELL_TARGETED = DEAD | INVULNERABILITY | STEALTH | SPELL_IMMUNITY,
		CANT_TELEPORT_OR_LEAP  = DEAD | ROOTED | GROUNDED,

        CUSTOM_1 = BIT(26),
        CUSTOM_2 = BIT(27),
        CUSTOM_3 = BIT(28),
        CUSTOM_4 = BIT(29),
		MAX_STATUS = BIT(30),
    };

	using Status = unsigned int;

}
