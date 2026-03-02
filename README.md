# ⚔️ Unreal Engine 5 Action RPG Framework
﻿
![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.X-blue?logo=unrealengine)
![C++](https://img.shields.io/badge/C++-17-blue?logo=c%2B%2B)
![Status](https://img.shields.io/badge/Status-In_Development-brightgreen)

## 📖 Project Overview
**This Project Slash** is a highly polished, code-driven Action RPG framework built from scratch using Unreal Engine 5 C++. Designed with action game principles in mind, it features a combat system, intelligent AI state machines, component-based architecture, and responsive gameplay mechanics (such as Perfect Dodge, Hit-Stop, and Motion Warped Executions). 

This project demonstrates strong Object-Oriented Programming (OOP) principles, clean C++ architecture, and deep integration with UE5's core systems including Enhanced Input, Animation Blueprints, NavMesh AI, and Chaos Physics.

> 💡 **Note:** Third-party heavy environment and texture assets have been excluded via `.gitignore` to keep the repository lightweight and code-focused.
>
> ---

## ✨ Core Technical Features
### 🗡️ Advanced Combat & Game Feel
* **Precise Weapon Hitboxes:** Replaced standard overlap events with `BoxTraceSingle` attached to weapon sockets for frame-perfect, precise melee collision detection. Implemented robust `IgnoreActors` array management to prevent multi-hit bugs during single animation frames.
* **Hit-Stop (Frame Freeze):** Implemented custom Time Dilation logic to briefly freeze the attacker and victim upon hit, drastically improving weapon impact and mechanical weight.
* **Combo**: Combo functionality has been implemented through enumeration and animation notifications. When the user presses the Attack button within a specified frame, the next combo segment will play; otherwise, the first combo segment will be played again.
https://github.com/user-attachments/assets/b1c96037-0f56-4379-b7fe-19c108a6e63a

* **Perfect Dodge :** Triggering a dodge at the exact moment of enemy impact activates a  effect via Global Time Dilation, rewarding the player with stamina regeneration.
https://github.com/user-attachments/assets/24c1d239-7685-4fa4-b96a-ace3f7ef10a5

* **Executions:** Integrated UE5 **Motion Warping** to perfectly align player and enemy transforms during fatal blows, bypassing standard collision for seamless takedown animations.
https://github.com/user-attachments/assets/3a221635-dd2a-42fd-a2d7-c3c1f742f16f

* **Directional Hit Reactions:** Calculates the angle between the attacker and victim using **Dot and Cross products** to mathematically determine and play accurate directional flinch animations (Front, Back, Left, Right).
* **Accumulated Power Attack**: Players press Q to perform an accumulated power attack. If the accumulation time hasn't expired, a normal attack will be executed. When the accumulation time is up, the accumulated power attack will be launched.
https://github.com/user-attachments/assets/28dc2780-6b03-4258-b0bb-5f0e28e54c89

### 🎯 Targeting & Camera System
* **Hard Lock-on System:** Implemented a robust lock-on mechanic inside a custom `UCombatComponent`. It utilizes Sphere Traces to detect enemies, sorts them dynamically by distance, and applies `FMath::RInterpTo` for buttery-smooth camera tracking.
* **Soft Targeting Assist:** When not locked on, the system calculates the most relevant target based on the camera's forward vector and distance to assist with attack magnetism and rotation warping.
https://github.com/user-attachments/assets/f1818aa7-1083-475e-b363-3cb7712a1eb0

### 🧠 AI & State Machine Architecture
* **Enemy AI State Machine:** Fully fleshed-out AI using strict C++ Enums (`Patrolling`, `Chasing`, `Attacking`, `BeingExecuted`, `Dead`) to prevent logic overlap and race conditions.
* **Perception & Navigation:** Enemies utilize `UPawnSensingComponent` to detect players, manage NavMesh movement via `AAIController`, and dynamically switch between predefined patrol routes and chasing behaviors.
* **Death Drops**: Drops souls when enemies die.
https://github.com/user-attachments/assets/f63119dc-58cf-4bb4-b2cb-4ffad5973343

### ⚙️ Engine Systems Integration
* **Component-Based Design:** Logic is heavily decoupled using custom Actor Components (`UAttributeComponent` for stats, `UCombatComponent` for targeting) to prevent bloating the Character classes and maintain high cohesion.
* **Interface-Driven Interaction:** Utilizes UE5 C++ Interfaces (`IHitInterface`, `IPickupInterface`) to handle cross-class communication (e.g., applying damage, picking up loot) cleanly and safely, avoiding heavy casting.
* **Chaos Physics Destruction:** Integrated `GeometryCollection` for breakable props that dynamically spawn randomized loot classes (Treasure) upon destruction.
https://github.com/user-attachments/assets/4aad9c36-d3f1-49e6-8305-81e3d3755236

Perhaps it will continue to be updated in the future.

















