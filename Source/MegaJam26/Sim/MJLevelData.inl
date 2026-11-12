// Fix It In Post - the campaign, as text. Included by MJLevels.cpp as an array of narrow raw strings (one per reel,
// because MSVC limits a single literal to 16 KB). The same format can be loaded from a file at runtime for level
// design without recompiling:  mj.LoadFile <path>   mj.SolveFile <path>
//
//   level <id>                         starts a level; 'end' (or the next 'level') finishes it
//   title / slug / script / note <text>
//   reel <1..3>
//   map ... endmap                     rows of the set, see the legend below
//   actor <role> <NAME> <facing n|e|s|w> [locked] [sight=N] [look=Mesh] | <strip>
//                                      role: star ally guard camera crusher prop; '_' in NAME becomes a space
//   bin <actor index> | <clips>        deleted scenes that can be restored into that actor's strip
//   rules edits=<max> par=<n> tools=cut,move,insert
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
actor guard GUARD s locked | z5=NAP ln z3
rules edits=1 par=1 tools=cut
end

level r1s2
title The Laser Hall
reel 1
slug INT. LASER HALL - NIGHT
script ACE crosses the hall while the grid is down.
note Cutting a clip pulls everything after it one beat earlier.
map
#########
#1..L..X#
#########
endmap
actor star ACE e | z2=SELFIE e e e e e e
actor prop LASER_GRID s locked | z2=ON c0+=OFF z1=OFF c0-=ON z3=ON
rules edits=1 par=1 tools=cut
end
)MJ",

R"MJ(
level r2s1
title Hold The Door
reel 2
slug INT. SERVICE TUNNEL - NIGHT
script PIP holds the door. ACE walks through it.
note Drag a clip along its strip to splice it somewhere else.
map
#########
#1..D..X#
#########
#2.d....#
#########
endmap
actor star ACE e | z=YAWN e e e e e e
actor ally PIP e | e e e e z2
rules edits=2 par=1 tools=cut,move
end
)MJ",
