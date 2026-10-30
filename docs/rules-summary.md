# 2026 Epic AI MegaJam: rules summary

Source: the jam page https://itch.io/jam/2026-epic-ai-megajam (read 30 Oct 2026). The Official Rules PDF at
https://epicgames.box.com/s/a7xcgl07fqoieeb99sjtd3juhj4agc4z is authoritative; it is a JavaScript viewer, so Shane
reads it once and notes anything that differs in the *Open items* section below.

## Eligibility
- 18+, active Epic Games account (18+ verified, same email as the itch account).
- Solo or team up to 5. "No limit on agents."
- Check the PDF for restricted countries and student or employee rules.

## Engine
- Unreal Engine **or** UEFN, not both. We use Unreal Engine 5.8.4 on Windows.

## Before the jam opens
- Setting up tools, agents and MCP ahead of time is explicitly encouraged ("getting MCP working on day one costs build time").
- No building the game before the jam opens (Thu 12 Nov 05:00 GMT).
- **Declare anything made before 12 Nov** that is used in the entry: prompts, scripts, assets, agent configuration.
  Kept in `docs/asset-declarations.md`.

## AI
- Any AI tools allowed; AI is optional. Usage is logged in `docs/ai-usage-log.md`.
- AI partner credits: "details coming soon" on the jam page. Re-check 10 and 11 Nov.

## Assets
- Non-original assets (Fab, Epic samples, other marketplaces, AI output) are allowed with a valid license, and must be
  **listed in the submission**.
- "Custom gameplay beyond Epic's starter templates is encouraged."

## Submission
- A packaged **development build** for Windows, macOS, Android or iOS, uploaded to the entrant's own itch.io project
  page and then submitted to the jam. Unpackaged projects are rejected.
- Zip named **`TeamName_GameName`**, **under 1 GB** (butler for large uploads).
- Also: team name, member name + linked Epic email, game name, a **30–60 s gameplay video** link, a dev-log link
  (**required for Glass Box**).
- Deadline: 15 Nov 5:59 PM PST = **Mon 16 Nov 01:59 GMT**. Late = disqualified. We submit Sunday by 19:00 GMT.

## Judging (1–5 each)
Ambition & innovation · Theme · Fun · Gameplay · Visuals · Level design & progression · Audio.
Level design, progression and audio each carry the same weight as fun: plan time for them.

## Prize targets (solo, UE, Windows build)
| Prize | Why we target it | Extra cost |
|---|---|---|
| Best Overall UE ($7.5k / $5k / $2.5k) | the main event | none |
| Development Continuation Grants ($50k / $25k / $10k) | awarded on potential; a tight, finished loop helps | none |
| Solo Driver ($2.5k) | automatic for solo entries | none |
| Glass Box ($2.5k, best dev log) | ~20 min per day if logged from hour zero | low |
| Render Royalty ($2.5k, visuals) | only if the art direction lands | medium |
| Pocket Powerhouse (mobile) | skipped by default: Android packaging and touch controls on a first UE project | high |

## Open items (Shane)
- [ ] Read the Official Rules PDF once: restricted countries, exact declaration wording, student rules, anything on
      AI disclosure format. Note differences here.
- [ ] Team name (needed for the zip name and `tools/jam.env`).
- [ ] Partner credits announcement (check 10–11 Nov).
