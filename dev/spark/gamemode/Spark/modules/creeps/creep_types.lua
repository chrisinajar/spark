-- "creep name", Health, Mana, Damage, Armor, Gold Bounty, Exp Bounty
CreepTypes = {
  -- 1 "easy camp"
  {
    {                                           --HP   MANA  DMG   ARM   GOLD  EXP
      {"npc_spark_creep_1.0",                    280,    0,  10,   0.5,   29,  32}, --expected gold is 56 and XP is 61
      {"npc_spark_creep_1.01",          		 480,    0,  12,    1,    36,  35}
    },
    {
      {"npc_spark_creep_1.1",        			 560,    0,  16,    1,    40,  35},
      {"npc_spark_creep_1.11",                   280,    0,  10,   0.5,   26,  30}
    },
    {
      {"npc_spark_creep_1.2",        			 560,    0,  16,    1,    40,  35},
      {"npc_spark_creep_1.21",                   280,    0,  10,   0.5,   26,  30}
    },
    {
      {"npc_spark_creep_1.3",                     480,    0,  12,    1,    33,  40},
      {"npc_spark_creep_1.31",                    480,    0,  12,    1,    33,  31}
    }
  },
    -- 2 "medium camp"
  {
    {                                          --HP   MANA  DMG   ARM   GOLD  EXP
      {"npc_spark_creep_1.4",              		560,  320,  24,   1.2,   45,  82}, --expected gold is 75 and XP is 272
      {"npc_spark_creep_1.41",              	560,  320,  24,   1.2,   45,  82},
    },
    {
      {"npc_spark_creep_1.5",              		560,  320,  24,   1.2,   45,  82},
      {"npc_spark_creep_1.51",              	440,    0,  40,   0.7,   40,  61},
    },
    {
      {"npc_spark_creep_1.6",   				480,    0,  28,   1.3,   50,  75},
      {"npc_spark_creep_1.61",                 	800,    0,  28,   1.3,   50,  75}
    }
  },
    -- 3 "hard camp"
  {
    {                                          --HP   MANA  DMG   ARM   GOLD  EXP
      {"npc_spark_creep_1.7",         			800,  400,  30,   1.5,   76,  61}, --expected gold is 113 and XP 121
      {"npc_spark_creep_1.71",       			600,  240,  20,   0.8,   37,  60},
    },
    {
      {"npc_spark_creep_1.8",         			800,  400,  30,   1.5,   76,  61},
      {"npc_spark_creep_1.81",       			600,  240,  20,   0.8,   37,  60},
    },
    {
      {"npc_spark_creep_1.9",      				400,  160,  15,    1,    31,  30},
      {"npc_spark_creep_1.91",     				400,  160,  15,    1,    31,  30},
      {"npc_spark_creep_1.92",        			600,  480,  35,    2,    85,  71}
    },
    {
      {"npc_spark_creep_2.0",      				400,  160,  15,    1,    31,  30},
      {"npc_spark_creep_2.01",        			600,  480,  35,    2,    85,  91}
    },
    {
      {"npc_spark_creep_2.1",      				400,  160,  15,    1,    31,  30},
      {"npc_spark_creep_2.11",        			600,  480,  35,    2,    85,  91}
    },
    {
      {"npc_spark_creep_2.2",           		350,  160,  10,    1,    28,  27},
      {"npc_spark_creep_2.21",        			450,  480,  20,    1,    38,  40},
      {"npc_spark_creep_2.22",         			550,  480,  30,   1.5,   51,  53}
    }
  },
   -- 4 "ancient camp"
  {
    {                                           --HP  MANA  DMG   ARM   GOLD  EXP
      {"npc_spark_creep_2.3",           		1400,    0,  50,    2,    99,  75}, --expected gold is 244 and XP is 151
      {"npc_spark_creep_2.31",              	1000,    0,  40,    1,    50,  38},
      {"npc_spark_creep_2.32",              	1000,    0,  40,    1,    50,  38}
    },
    {
      {"npc_spark_creep_2.4",          			900,    0,  30,    1,    79,  51},
      {"npc_spark_creep_2.41",          		1200,    0,  60,    2,   120, 100}
    },
    {
      {"npc_spark_creep_2.5",     				1700,    0,  80,    3,   199, 151}
    }
  },
   -- 5 "solo camp" radiant
  {
    {
       {"npc_spark_creep_2.6",            		1500,    0,  70,    3,   152, 156}
    }
  }
}