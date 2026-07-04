[README.md](https://github.com/user-attachments/files/29624305/README.md)
# ZeroProtocol

A multiplayer third-person shooter built with **Unreal Engine 5** and **C++**, featuring server-authoritative networked combat, per-bone server-side rewind lag compensation, and both free-for-all and team-based game modes.

## Table of Contents
- [Overview](#overview)
- [Highlights](#highlights)
- [Gameplay Features](#gameplay-features)
- [Tech Stack](#tech-stack)
- [Architecture](#architecture)
- [Multiplayer Setup Guide](#Multiplayer-Setup-Guide)
- [Game Download Link](#Game-Download-Link)
- [License](#license)

## Overview

ZeroProtocol is a competitive multiplayer shooter where players fight it out across free-for-all and team deathmatch modes, picking up weapons and power-ups scattered around the map. Combat is server-authoritative from the ground up — hit registration, damage, ammo, and match state are all validated on the server, with client-side prediction and lag compensation layered on top to keep the game feeling responsive over the network.

## Highlights

- **Server-side rewind lag compensation** for hitscan, projectile, and shotgun weapons — the server rewinds every character's per-bone hitboxes to the shooter's perceived time before validating a hit.
- **Adaptive fallback** — clients reporting high ping automatically switch from server-side rewind to standard authoritative hit detection.
- **Round-trip clock sync** between client and server so every hit request carries an accurate timestamp.
- **Client-predicted ammo**, reconciled against the server with a sequence number so the HUD never waits on a round trip.
- **Unified weapon framework** — hitscan, projectile, and shotgun weapons share one base class and combat-state machine, each with independently replicated equip/fire/reload/swap logic.
- **Two match types, one state machine** — free-for-all and team deathmatch both run through the same warmup → match → cooldown flow, with mid-game join support.

## Gameplay Features

### Combat & Weapons
- Modular `AWeapon` base supporting three fire types: **Hitscan**, **Projectile**, and **Shotgun**
- Seven weapon types: Assault Rifle, Pistol, SMG, Shotgun, Sniper Rifle, Rocket Launcher, Grenade Launcher
- Primary + secondary weapon slots with a dedicated swap flow, and per-weapon-type reserve ammo tracked via a carried-ammo map
- Full reload pipeline, including a shell-by-shell reload loop for shotguns
- Grenade throwing with its own combat state, a visible held-grenade mesh, and a capped grenade count
- Weapon pickups use an overlap volume and a world-space prompt widget; collision, physics, and custom-depth outline all update automatically between dropped / equipped / secondary states
- Dynamic crosshair spread driven by movement, aiming, and firing state, plus per-weapon crosshair textures
- Configurable trace scatter (bloom) for hip-fire accuracy, and a dedicated multi-pellet scatter path for shotguns with separate head-shot/body-shot damage accumulation per target
- Sniper scope overlay when aiming with scoped weapons
- Physically simulated shell casings eject on fire with impact sound

### Networking & Lag Compensation
- Full client-server replication using Server / Client / NetMulticast RPCs throughout combat, buffs, and HUD state
- `ULagCompensationComponent` records a rolling per-bone hitbox history (head, spine, arms, hands, legs, and more) for every character and rewinds it to a requested timestamp to validate hits fairly under latency
- Independent rewind paths for hitscan traces, projectiles (using a cached initial velocity), and multi-pellet shotgun traces
- High-ping clients are automatically routed to standard server-authoritative damage instead of rewind-based validation
- Round-trip clock synchronization between client and server, used to timestamp every hit request

### Character & Animation
- Aim-offset system (yaw/pitch) blended with a full turn-in-place state machine for both locally controlled and simulated proxy characters
- Procedural lean based on frame-to-frame rotation delta
- FABRIK-driven hand IK plus a procedural look-at rotation for the aiming hand, gated by combat state so it doesn't fight reload/swap/grenade animations
- Third-person camera boom with automatic mesh hiding when the camera gets too close
- Elimination sequence with a dissolve-material timeline and team-colored materials

### Buffs & Pickups
- Server-authoritative heal-over-time and shield-replenish-over-time, ramped per tick and synced to the HUD
- Timed speed and jump buffs that revert automatically and broadcast the change to all clients

### Game Modes & Match Flow
- Shared warmup → in-progress → cooldown → restart state machine (`ABlockGameMode`), with countdown HUD and mid-game join catch-up
- Free-for-all scoring with a tracked leaderboard and a "current leader" crown effect (supports ties)
- `ATeamGameMode` adds automatic team balancing on join and at match start, team score tracking, and friendly-fire prevention — though self-damage from your own explosives still applies
- `ALobbyGameMode` seamlessly travels the session to the correct map once it reaches its target player count, branching on free-for-all vs. team match type

### HUD & Feedback
- Live health, shield, ammo, carried ammo, grenade count, score, and defeat tracking
- Kill feed / death messages, match announcements, and countdown timers
- High-ping warning indicator

## Tech Stack
- **Unreal Engine 5**, C++ with Blueprint-exposed properties throughout
- **Enhanced Input** for all movement and combat input actions
- **Niagara** (leader crown, projectile trails) alongside legacy **Cascade** particle systems (impacts, muzzle flashes, tracer beams)
- **MetaSounds** for weapon, impact, and pickup audio
- Timeline/curve-driven material dissolve effect for eliminations
- Custom `UProjectileMovementComponent` subclass so rockets explode on hit instead of stopping
- Session-based matchmaking via a custom `MultiplayerSessionsSubsystem`

## Architecture

| System | Key Classes |
|---|---|
| Character & Animation | `ABlockCharacter`, `UBlockAnimInstance` |
| Combat | `UCombatComponent`, `AWeapon`, `AHitScanWeapon`, `AProjectileWeapon`, `AShotGun` |
| Projectiles | `AProjectile`, `AProjectileBullet`, `AProjectileGrenade`, `AProjectileRocket`, `URocketMovementComponent` |
| Lag Compensation | `ULagCompensationComponent` |
| Buffs | `UBuffComponent` |
| Game Modes | `ABlockGameMode`, `ATeamGameMode`, `ALobbyGameMode` |
| Player Framework | `ABlockPlayerController`, `ABlockPlayerState`, `ATeamPlayerStart` |
| Shared Types | `ETeam`, `ECombatState`, `ETurningInPlace`, `EWeaponType`, `EWeaponState`, `EFireType` |

## Getting Started

### Requirements
- Unreal Engine 5.7
- Visual Studio 2022 (Windows) or Xcode (macOS) with the Unreal C++ toolchain
- An Online Subsystem plugin (e.g. Steam or EOS) wired up through the project's `MultiplayerSessionsSubsystem`

## Multiplayer Setup Guide
- Follow these steps to play multiplayer with friends:
- Open Steam and go to Settings.
- Navigate to Downloads.
- Set the Download Region to the same region for all players. Players with different download regions may not be able to discover the same lobby.
- Launch the game.
- One player should Host a multiplayer server.
- Other players should click Join and connect to the host's lobby.
  
## Game Download Link
https://drive.google.com/file/d/1_Bl1F8d6G-uNjboDUplh_ORvfZkaL4H0/view?usp=sharing

## License
All rights reserved. Shared for portfolio purposes only.
