// Fix It In Post - the campaign, as text. Included by MJLevels.cpp as an array of narrow raw strings (one per reel,
// because MSVC limits a single literal to 16 KB). The same format can be loaded from a file at runtime for level
// design without recompiling:  mj.LoadFile <path>   mj.SolveFile <path>
// Every level here is checked by the brute-force solver (mj.SolveAll): the theatrical cut must fail, the minimum number
// of edits must equal par, and any bonus must be reachable.
//
//   level <id>                         starts a level; 'end' (or the next 'level') finishes it
//   title / slug / script / note <text>
//   reel <1..3>
//   map ... endmap                     rows of the set, see the legend below
//   actor <role> <NAME> <facing n|e|s|w> [locked] [sight=N] [look=Mesh] | <strip>
//                                      role: star ally guard villain camera crusher prop; '_' in NAME becomes a space
//   bin <actor index> | <clips>        deleted scenes that can be restored into that actor's strip
//   rules edits=<max> par=<n> tools=cut,move,insert [hints=full|vague|none]   (hints: default full)
//   needitem                           the star must grab the prize (*) before reaching the mark
//   invert <ch>[,<ch>]                 channels that start active (door open / laser off)
//   bonus maxbeats <n> | maxedits <n> | noko | collect | keep <LABEL>   then  | <text shown to the player>
//
// Map legend:  '#' wall  '.' floor  ' ' void  '~' pit  'X' the star's mark  '*' the prize  '$' golden reel (bonus)
//   '1'..'9' start tile of the n-th actor      'D' 'E' 'F' 'G' doors on channel 0..3     'd' 'e' 'f' 'g' plates
//   'p' 'q' 'r' 't' levers on channel 0..3     'L' 'M' 'N' 'O' laser tiles on channel 0..3
//
// Strip tokens:  n e s w = move (e3 = three-beat run)   z = wait (z2 = two beats)   ln le ls lw = look
//   u = use (lever / grab)   a = attack   c0+ / c0- = prop switches channel 0 on / off
//   suffix =LABEL names the clip ('_' becomes a space), suffix ! locks the clip.

// ---------------------------------------------------------------------------------------------- REEL 1: THE HEIST
R"MJ(
level r1s1
title Product Placement
reel 1
slug INT. MUSEUM CORRIDOR - NIGHT
script ACE slips past the night guard and reaches his mark.
note The studio added a soda advert. Click a clip, then press X to cut it.
map
#########
#1.....X#
#####2###
#########
endmap
actor star ACE e | e e z2=SODA_AD e e e e
actor guard GUARD s locked | z5=NAP ln=WAKE z3=GLARE
rules edits=1 par=1 tools=cut
end

level r1s2
title The Laser Hall
reel 1
slug INT. LASER HALL - NIGHT
script ACE crosses the hall while the laser grid is down.
note The grid has a strip too. ACE can only cross while it says OFF.
map
###########
#1...L...X#
###########
endmap
actor star ACE e | e z2=SELFIE e e z=COMB_HAIR e e e e e
actor prop LASER_GRID s locked | z3=ON c0+=OFF z=OFF c0-=ON z5=ON
rules edits=2 par=1 tools=cut
bonus maxbeats 8 | Tight edit: the scene runs 8 beats or fewer
end

level r1s3
title The Night Watchman
reel 1
slug INT. GALLERY - NIGHT
script ACE walks straight past the watchman, who is looking the other way.
note ACE's agent forbids cutting his scenes. The watchman has no agent.
map
#########
###2#####
###.#####
#1.....X#
###.#####
#########
endmap
actor star ACE e locked | e e e e e e
actor guard WATCHMAN s | z2=STARE ln=CHECK_CLOCK z=YAWN ls=STARE z3=STARE
rules edits=1 par=1 tools=cut
end

level r1s4
title The Scenic Route
reel 1
slug INT. ROTUNDA - NIGHT
script ACE walks to his mark. That is the whole scene.
note Cutting a move changes where everything after it happens.
map
#~~~~####
#1....X.#
#..L....#
#########
endmap
actor star ACE e | e e s=DIVE e n=LEAP e e e
rules edits=2 par=2 tools=cut
end

level r1s5
title The Diamond
reel 1
slug INT. JEWEL ROOM - NIGHT
script ACE lifts the diamond while the guard dozes, and walks out with it.
note Some pauses matter. The guard looks in on a schedule.
map
###########
#1...*...X#
#####2#####
###########
endmap
actor star ACE e | e e z=YAWN z2=TIE_SHOE e e u=GRAB z2=ADMIRE e e e e
actor guard GUARD s locked | z2=DOZE ln=PEER z=PEER ls=DOZE z3=DOZE ln=PEER z=PEER ls=DOZE z4=DOZE
rules edits=2 par=1 tools=cut
needitem
bonus keep ADMIRE | Keep the money shot: ADMIRE stays in the cut
end

level r1s6
title The Vault
reel 1
slug INT. VAULT - NIGHT
script ACE beats the lasers, takes the diamond and slips out past the last guard.
note Three strips. Work out who needs to be where, beat by beat.
map
#############
#1..L..*..X##
#########2###
#############
endmap
actor star ACE e | e z=SODA_AD e e z2=SELFIE e e e u=GRAB z=WINK e e e
actor guard GUARD s | z5=NAP ln=WAKE z7=GLARE ls=DOZE z3=NAP
actor prop LASER_GRID s locked | z3=ON c0+=OFF c0-=ON z9=ON
rules edits=3 par=2 tools=cut
needitem
bonus maxbeats 11 | Tight edit: the scene runs 11 beats or fewer
end
)MJ",

// -------------------------------------------------------------------------------------------- REEL 2: THE GETAWAY
R"MJ(
level r2s1
title Hold The Door
reel 2
slug INT. SERVICE TUNNEL - NIGHT
script PIP stands on the switch. ACE walks through both doors.
note New tool: SPLICE. Drag a clip along its own strip to move it.
map
#########
#1..DD.X#
#########
#2..d...#
#########
endmap
actor star ACE e | e e e e e e e
actor ally PIP e | z2=TEXTING e e e e
rules edits=2 par=1 tools=cut,move
end

level r2s2
title Pull
reel 2
slug INT. CONTROL ROOM - NIGHT
script ACE pulls the lever, the door opens, ACE leaves.
note USE acts on the tile ACE is standing on.
map
#########
#1.p.D.X#
#########
endmap
actor star ACE e | e e e u=PULL e e e
rules edits=2 par=1 tools=cut,move hints=vague
end

level r2s3
title Sucker Punch
reel 2
slug INT. LONG CORRIDOR - NIGHT
script ACE creeps up while the goon checks his phone, and puts him to sleep.
note PUNCH knocks out whoever is right in front of ACE. Guards see three tiles ahead.
map
##########
#1....2.X#
##########
endmap
actor star ACE e | a=WARM_UP e e e e z=BREATHE e e e
actor guard GOON w locked | z2=GLARE ln=PHONE z3=PHONE lw=GLARE z6=GLARE
rules edits=2 par=2 tools=cut,move hints=vague
end

level r2s4
title Dead Weight
reel 2
slug INT. LOADING BAY - NIGHT
script ACE drops the goon on the pressure switch, and the goon keeps the door open.
note A sleeping goon is still heavy.
map
#########
#..d2D.X#
#.#######
#1#######
#########
endmap
actor star ACE n | z2=STRETCH z3=PEP_TALK n n e e a=PUNCH e e e e e
actor guard GOON w locked | z2=IDLE w=PATROL ln=SNIFF z3=LISTEN e=PATROL le=STARE z6=STARE
rules edits=2 par=1 tools=cut,move hints=vague
bonus maxbeats 12 | Tight edit: the scene runs 12 beats or fewer
end

level r2s5
title Look Both Ways
reel 2
slug EXT. BACK ALLEY - NIGHT
script The goon never makes it across the road. ACE strolls out.
note The getaway car is right on time. Somebody is being very careful crossing the road.
map
######3####
######.####
#1......2.X
######.####
######.####
endmap
actor star ACE e locked | z3=HIDES e e e e e e e e e
actor guard GOON w | w z=LOOK_LEFT z=LOOK_RIGHT z=LOOK_LEFT z=WAIT_FOR_CAR w w z9=STAND_GUARD
actor crusher CAR s locked | z3=REVS s s s s
rules edits=2 par=1 tools=cut,move hints=none
end

level r2s6
title Two Man Job
reel 2
slug INT. TWIN CORRIDORS - NIGHT
script ACE holds the switch for PIP. PIP kills the lasers for ACE. ACE takes the diamond.
note Two strips to fix, and each depends on the other's timing.
map
###########
#1.d..M.*X#
###########
#2.D..q...#
###########
endmap
actor star ACE e | e e e e e z2=HUMS e e u=GRAB e
actor ally PIP e | e e e e e u=PULL z=WAVE
rules edits=3 par=2 tools=cut,move hints=none
needitem
end
)MJ",

// ------------------------------------------------------------------------------------------- REEL 3: THE SHOWDOWN
R"MJ(
level r3s1
title The Deleted Scene
reel 3
slug INT. TROPHY ROOM - NIGHT
script ACE pockets the Baron's diamond on his way out.
note New tool: RESTORE. Drag a clip from DELETED SCENES back into its strip.
map
#########
#1..*..X#
#########
endmap
actor star ACE e | e e e e e e
bin 0 | u=GRAB
rules edits=1 par=1 tools=cut,move,insert
needitem
end

level r3s2
title The Monologue
reel 3
slug INT. THE BARON'S LAIR - NIGHT
script The BARON talks too long. ACE is gone before the trap springs.
note The Baron's contract says his lines can't be cut or moved. The studio deleted one anyway.
map
###########
#1....L..X#
###########
#....p...2#
###########
endmap
actor star ACE e locked | e e e e e e e e
actor villain BARON w sight=0 | w! w! w! w! u=SPRING_TRAP! z2=CACKLE!
bin 1 | z3=MONOLOGUE
invert 0
rules edits=1 par=1 tools=cut,move,insert hints=vague
end

level r3s3
title Hit And Run
reel 3
slug EXT. THE BARON'S DRIVEWAY - NIGHT
script The getaway car clears the road. ACE walks out through what is left of the goon.
note Props have strips too. This one stalls.
map
###3###
#1....#
###.###
###2.X#
###.###
###.###
endmap
actor star ACE e | z4=WAITS e e s s e e
actor guard GOON n locked | z9=STANDS_GUARD
actor crusher CAR s | z2=IDLES s s=CRAWLS z3=STALLS
bin 2 | s3=FLOOR_IT
rules edits=2 par=1 tools=cut,move,insert hints=vague
end

level r3s5
title Pip's Big Scene
reel 3
slug INT. THE BARON'S STUDY - NIGHT
script PIP gets the drop on the BARON. ACE strolls out over him.
note The studio deleted PIP's one heroic moment. A punch only hits the tile in front.
map
###########
#1.....3.X#
#######.###
#2........#
###########
endmap
actor star ACE e locked | z6=WAITS e e e e e e e e
actor ally PIP e | e e e e e e z2=HESITATES n
actor villain BARON w locked | z9=GLOATS z9=GLOATS
bin 1 | a=PUNCH
rules edits=2 par=2 tools=cut,move,insert hints=vague
end

level r3s6
title Smile For The Camera
reel 3
slug INT. THE BARON'S GALLERY - NIGHT
script ACE walks in under the camera, takes the diamond and walks out the way he came.
note The camera's sweep is locked. Two of its deleted clips survive. Line the strips up.
map
#########
#X1...*.#
####2####
#########
endmap
actor star ACE e locked | e e e e u=GRAB w w w w w
actor camera CAMERA s | z=IDLE! ln=PAN! z2=WATCH! ls=PAN! z2=IDLE! ln=PAN! z2=WATCH! ls=PAN! z2=IDLE!
bin 1 | z=GLITCH z4=LOOP_FOOTAGE
rules edits=2 par=1 tools=cut,move,insert hints=none
needitem
end

level r3s4
title The Showdown
reel 3
slug INT. THE BARON'S VAULT - NIGHT
script ACE beats the trap, PIP holds the door, and ACE walks out with the diamond.
note The final scene. All three tools are yours.
map
#############
#1..L..E..*X#
#############
#2..e.......#
#############
#....p....3.#
#############
endmap
actor star ACE e | e e z3=SODA_AD e e e e e e e e
actor ally PIP e | e e e e z2=TEXTING
actor villain BARON w sight=0 | w! w! w! w! w! u=SPRING_TRAP! z2=CACKLE!
bin 0 | u=GRAB
bin 2 | z3=MONOLOGUE
invert 0
rules edits=3 par=3 tools=cut,move,insert hints=none
needitem
bonus keep SODA_AD | The sponsor is at the premiere: SODA AD stays in the cut
end
)MJ",
