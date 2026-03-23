# DialogAndQuest Plugin (v2.0)

A keyword-based dialog and quest state machine system for Unreal Engine 5, inspired by classic RPG dialog (Morrowind-style topics) and Timothy Cain's quest design philosophy. Built for dedicated-server multiplayer with full replication support.

![Dialog window](./Doc/Images/DialogWindow.png?raw=true "DialogWindow")
![Quest journal](./Doc/Images/QuestJournal.png?raw=true "QuestJournal")

## Features

- **Keyword-based dialog**: Topic words embedded in NPC text become clickable hyperlinks that reveal more information. NPCs share topic bundles so common knowledge propagates naturally.
- **Six-state quest machine**: Quests follow `Unknown → Mentioned → Accepted → Achieved → Completed` with a `Botched` dead-end — matching the design principle that quests are discovered, accepted, and can fail.
- **Dialog conditions**: Topics gate on NPC relation, quest state, item possession, and skill checks (game-agnostic via interface).
- **Dialog consequences**: Clicking a topic can transition quest states, adjust faction relation, or "mention" other quests — enabling quest discovery through conversation.
- **Voiceover-ready**: Every topic, greeting, and quest step carries `TSoftObjectPtr<USoundBase>`, `FName VoiceoverEventName` (for Wwise/FMOD), and `float VoiceoverDuration` fields.
- **FText localization**: All player-facing text uses `FText` for UE localization pipeline compatibility.
- **UDataAsset authoring**: `UDialogAsset` and `UQuestAsset` replace the legacy three-DataTable chain with single versionable assets that appear in the Content Browser under Gameplay.
- **Dedicated server architecture**: Dialog master data lives on the GameMode (server-only). Per-NPC dialog replicates to clients. Quest progress replicates owner-only.
- **Backward compatible**: Legacy DataTable loading methods still work alongside the new asset pipeline.

## Quick Start

1. Add the plugin to your `Plugins/` directory
2. Add `"DialogAndQuestPlugin"` to your module's `Build.cs` dependencies
3. Implement `IDialogGameModeInterface` on your GameMode with `UDialogMainComponent` + `UQuestMainComponent`
4. Implement `IDialogInterface` on your NPCs with `UDialogComponent`
5. Implement `IQuestBearerInterface` + `IDialogDisplayInterface` on your PlayerController with `UQuestBearerComponent`
6. Create `UDialogAsset` / `UQuestAsset` in the Content Browser and load them in your GameMode's `BeginPlay`

See **[Doc/IntegrationGuide.md](./Doc/IntegrationGuide.md)** for the full step-by-step guide.

## Architecture

```
GameMode (Server)
├── UDialogMainComponent ── Global dialog registry (topics, bundles, meta-bundles)
└── UQuestMainComponent ── Global quest registry (quest metadata + steps)

NPC Actor
├── UDialogComponent ── Per-NPC dialog data (replicated to clients)
├── UQuestGiverComponent ── Which quest steps this NPC can validate (server-only)
└── IDialogInterface ── Relation, name, capabilities (trade/bank/train/give/repair)

PlayerController
├── UQuestBearerComponent ── Per-player quest progress (replicated owner-only)
├── IDialogDisplayInterface ── UI creation and display
├── IDialogConsequenceInterface ── Handle faction/quest consequences from dialog
└── IDialogSkillCheckInterface ── Evaluate skill checks for dialog conditions
```

## Quest State Machine

```
Unknown ──► Mentioned ──► Accepted ──► Achieved ──► Completed
                │              │            │
                └──────────────┴────────────┴──► Botched
```

| State | Description |
|-------|-------------|
| **Unknown** | Player has never heard of this quest |
| **Mentioned** | Player overheard a rumor, read a note, or an NPC mentioned it in passing |
| **Accepted** | Player formally took the quest from a quest giver |
| **Achieved** | All objectives completed, awaiting turn-in |
| **Completed** | Quest finished, rewards granted |
| **Botched** | Irrecoverably failed (NPC died, item destroyed, wrong dialog choice) |

## Dialog System

### Concepts

**Topics** — A keyword (e.g., "bread", "skeleton invasion") that triggers an NPC response. Topics are the atomic unit of dialog.

**Conditions** — Each topic has visibility conditions: minimum relation, quest state, carried items, skill checks, and consume-on-use flags.

**Consequences** — Clicking a topic can: transition a quest to a new state, adjust faction relation, or mention another quest (quest discovery through conversation).

**Bundles** — Collections of topics shared among similar NPCs (e.g., all villagers discuss the same "market" and "guards" topics).

**Meta-bundles** — Collections of bundles assigned to a specific NPC, plus greeting text and relation thresholds.

**Dialog Assets** (v2.0) — `UDialogAsset` replaces the three-table chain with a single asset containing greetings, relation threshold, and all topics.

### Hyperlinks

Words in `TopicText` that match another topic's keyword are automatically rendered as clickable hyperlinks using `URichInlineHyperlinkDecorator`.

## Modules

| Module | Type | Purpose |
|--------|------|---------|
| `DialogAndQuestPlugin` | Runtime | Core dialog + quest logic, components, interfaces, UI base classes |
| `DialogAndQuestPluginEditor` | Editor | Asset type actions for `UDialogAsset` and `UQuestAsset` (content browser integration) |

## Key Interfaces

| Interface | Implement On | Purpose |
|-----------|-------------|---------|
| `IDialogGameModeInterface` | GameMode | Access global dialog/quest registries |
| `IDialogInterface` | NPC Actor | NPC relation, dialog component, capabilities |
| `IQuestGiverInterface` | NPC/Trigger | Quest step validation |
| `IQuestBearerInterface` | PlayerController | Quest progress tracking and state transitions |
| `IDialogDisplayInterface` | PlayerController | Dialog UI creation and display |
| `IDialogConsequenceInterface` | PlayerController | Handle dialog-triggered world state changes |
| `IDialogSkillCheckInterface` | PlayerController | Evaluate skill/item checks for topic visibility |

## Voiceover Support

All topics, greetings, and quest steps carry:
- `TSoftObjectPtr<USoundBase> VoiceoverCue` — UE sound asset (soft-loaded)
- `FName VoiceoverEventName` — external middleware event name (Wwise/FMOD)
- `float VoiceoverDuration` — manual subtitle timing override

`UDialogComponent::OnVoiceoverRequested` delegate fires when a voiced topic is displayed. The game layer decides how to play the audio.

## Persistence

Quest progress persists as `{QuestID, StepID, EQuestState}`. Listen to `QuestUpdateDispatcher` for save triggers. On load, call `AuthoritySetupQuestData(QuestID, StepID, State)`.

## Dependencies

- `Core`, `CoreUObject`, `Engine`
- `Slate`, `SlateCore`, `UMG` (UI widgets)
- `GameplayTags` (dialog conditions, botch tags, skill checks)

## Demo

See https://github.com/Synock/UE5DialogAndQuest for a basic setup demonstrating dialog, quest progression, area validators, and relation-gated topics.

## License

See [LICENSE](./LICENSE).
